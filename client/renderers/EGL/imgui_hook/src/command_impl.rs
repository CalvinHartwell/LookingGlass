use crate::commands::{Frame, DrawCommand, Command};
use crate::bindings::*;

impl Command {
    pub unsafe fn handle(&self) {
        match self {
            Command::UpdateFrame(frame) => {
                let mut current_frame = crate::CURRENT_FRAME.lock().unwrap();
                // there may be a way to not use clone here (TODO)
                *current_frame = frame.clone();
            }
        }
    }
}

impl Frame {
    pub unsafe fn draw(&self, draw_list: *mut ImDrawList) {
        for command in &self.commands {
            command.draw(draw_list);
        }
    }
}

impl DrawCommand {
    pub unsafe fn draw(&self, draw_list: *mut ImDrawList) {
        match self {
            DrawCommand::Line(data) => {
                ImDrawList_AddLine(
                    draw_list,
                    ImVec2{x: data.x1, y: data.y1},
                    ImVec2{x: data.x2, y: data.y2},
                    data.color,
                    data.width
                )
            }
            _ => unimplemented!()
        }
    }
}