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
        let context = bindings::SDL_GL_CreateContext(window);
        igCreateContext(null_mut());
        ImGui_ImplSDL2_InitForOpenGL(window, context);
        igStyleColorsDark(null_mut());
    }
}

lazy_static::lazy_static! {
    pub static ref CURRENT_FRAME: Mutex<commands::Frame> = Mutex::new(commands::Frame::new());
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

            // Draw the current frame updated by the pipe
            // Maybe use channels instead and keep a local frame?
            CURRENT_FRAME.lock().unwrap().draw(draw_list);

            // ImDrawList_AddRect(
            //     draw_list,
            //     ImVec2{x: 10.0, y: 10.0},
            //     ImVec2{x: 500.0, y: 500.0},
            //     0xFFFFFFFF,
            //     0.0,
            //     0,
            //     5.0
            // );

            igEnd();
        }
        igRender();
        igEndFrame();

        igGetDrawData()
    }
}