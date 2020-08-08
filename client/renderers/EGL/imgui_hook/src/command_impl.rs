use crate::commands::{Frame, DrawCommand, Command};
use crate::bindings::*;

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