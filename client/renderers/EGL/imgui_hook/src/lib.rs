#[allow(warnings)]
mod bindings;
mod commands;
mod command_impl;
mod fonts;
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

#[no_mangle]
pub extern "C" fn init_imgui(window: *mut SDL_Window) {
    unsafe {
        let sdl_context = bindings::SDL_GL_CreateContext(window);
        let ctx = igCreateContext(null_mut());
        fonts::init_imgui_fonts((*igGetIO()).Fonts);
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
        igSetNextWindowPos(ImVec2 { x: 0.0, y: 0.0 }, 0, ImVec2 { x: 0.0, y: 0.0 });
        igSetNextWindowSize((*igGetIO()).DisplaySize, 0);
        igSetMouseCursor(ImGuiMouseCursor__ImGuiMouseCursor_None);
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
                    }
                    Command::ClearScreen => {
                        (*CURRENT_FRAME.lock().unwrap()) = Frame::new();
                    }
                }
            }

            // Draw the frame
            CURRENT_FRAME.lock().unwrap().draw(draw_list);
            // let title = CString::new("hello").unwrap();
            // igShowFontSelector(title.as_ptr());
            // igShowDemoWindow(null_mut());

            igEnd();
        }
        igRender();
        igEndFrame();

        igGetDrawData()
    }
}