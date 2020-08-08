#[allow(warnings)]
mod bindings;
mod commands;
mod command_impl;
mod pipe;

use std::ffi::{CString, CStr};
use std::ptr::{null, null_mut};
use std::sync::Mutex;
use bindings::*;
use simplelog::{TermLogger, Config, TerminalMode};
use log::*;
use commands::Command;
use crossbeam::{Sender, Receiver, bounded};
use crate::commands::Frame;

// include!(concat!(env!("OUT_DIR"), "/bindings.rs"));
const PIPE_LOCATION: &str = "/tmp/overlay-pipe";

#[no_mangle]
pub extern "C" fn start_pipe_thread() {
    TermLogger::init(LevelFilter::Debug, Config::default(), TerminalMode::Mixed);
    std::thread::spawn(pipe::pipe_thread);
}

unsafe fn init_font(atlas: *mut ImFontAtlas, name: &str, bytes: &[u8], pixel_height: usize) {
    let mut config: ImFontConfig = std::mem::zeroed();
    // let mut name_buf: [i8; 40] = [0; 40];
    // for (index, &c) in name.as_bytes().iter().enumerate() {
    //     name_buf[index] = c as _;
    // }
    //
    // config.Name = name_buf;
    // config.FontNo = 1;
    // config.FontData = bytes.as_ptr() as _;
    //
    ImFontAtlas_AddFontFromMemoryTTF(
        atlas,
        bytes.as_ptr() as _,
        1,
        pixel_height as _,
        null(),
        null()
    );
}

unsafe fn init_imgui_fonts(atlas: *mut ImFontAtlas) {
    ImFontAtlas_AddFontDefault(atlas, null());
    init_font(atlas, "Pixel font", include_bytes!("fonts/smallest_pixel-7.ttf"), 10);
    init_font(atlas, "Verdana", include_bytes!("fonts/Verdana.ttf"), 13);
    init_font(atlas, "Tahoma", include_bytes!("fonts/Tahoma.ttf"), 14);
}

#[no_mangle]
pub extern "C" fn init_imgui(window: *mut SDL_Window) {
    unsafe {
        let sdl_context = bindings::SDL_GL_CreateContext(window);
        let ctx = igCreateContext(null_mut());
        init_imgui_fonts((*igGetIO()).Fonts);
        // init fonts
        ImGui_ImplSDL2_InitForOpenGL(window, sdl_context);
        igStyleColorsDark(null_mut());
    }
}

lazy_static::lazy_static! {
    pub static ref CURRENT_FRAME: Mutex<commands::Frame> = Mutex::new(commands::Frame::new());
    pub static ref COMMAND_CHAN: (Sender<Command>, Receiver<Command>) = bounded(10);
}

#[no_mangle]
pub extern "C" fn render_callback(window: *mut SDL_Window) -> *mut ImDrawData {
    unsafe {
        // ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        igNewFrame();


        // igSetMouseCursor(ImGuiMouseCursor__ImGuiMouseCursor_None);
        let overlay_title = CString::new("Overlay").unwrap();
        igSetNextWindowPos(ImVec2{x: 0.0, y: 0.0}, 0, ImVec2{x: 0.0, y: 0.0});
        igSetNextWindowSize((*igGetIO()).DisplaySize, 0);
        if igBegin(overlay_title.as_ptr(), null_mut(), (ImGuiWindowFlags__ImGuiWindowFlags_NoTitleBar
                        | ImGuiWindowFlags__ImGuiWindowFlags_NoResize
                        | ImGuiWindowFlags__ImGuiWindowFlags_NoMove
                        | ImGuiWindowFlags__ImGuiWindowFlags_NoScrollbar
                        | ImGuiWindowFlags__ImGuiWindowFlags_NoSavedSettings
                        | ImGuiWindowFlags__ImGuiWindowFlags_NoInputs
                        | ImGuiWindowFlags__ImGuiWindowFlags_NoBackground) as _) {
            let draw_list = igGetWindowDrawList();

            // Get a command from the pipe channel if there is one in the buffer
            if let Ok(command) = COMMAND_CHAN.1.try_recv() {
                match command {
                    Command::UpdateFrame(frame) => {
                        (*CURRENT_FRAME.lock().unwrap()) = frame;
                    },
                    Command::ClearScreen => {
                        (*CURRENT_FRAME.lock().unwrap()) = Frame::new();
                    }
                }
            }

            // Draw the frame
            CURRENT_FRAME.lock().unwrap().draw(draw_list);
            let title = CString::new("hello").unwrap();
            igShowFontSelector(title.as_ptr());
            igShowDemoWindow(null_mut());

            igEnd();
        }
        igRender();
        igEndFrame();

        igGetDrawData()
    }
}