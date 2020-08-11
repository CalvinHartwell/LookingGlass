use crate::commands::{Frame, DrawCommand, Command, TextStyle};
use crate::bindings::*;
use std::ffi::CString;
use std::ptr::{null, null_mut};

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
                    ImVec2 { x: data.x1, y: data.y1 },
                    ImVec2 { x: data.x2, y: data.y2 },
                    data.color,
                    data.width,
                )
            }
            DrawCommand::Text(data) => {
                let font = data.font.to_font();
                let text = CString::new(data.text.clone()).unwrap_or(CString::new("").unwrap());
                let text_pos = {
                    if !data.centered {
                        ImVec2 { x: data.x, y: data.y }
                    } else {
                        let mut text_size = ImVec2 { x: 0.0, y: 0.0 };
                        ImFont_CalcTextSizeA(
                            &mut text_size,
                            font,
                            (*(*font).ConfigData).SizePixels,
                            igGET_FLT_MAX(),
                            0.0,
                            text.as_ptr(),
                            null(),
                            null_mut(),
                        );
                        ImVec2 { x: (data.x - (text_size.x / 2.0)), y: data.y }
                    }
                };
                let font_size = {
                    if data.font_size == 0.0 {
                        (*(*font).ConfigData).SizePixels
                    } else {
                        data.font_size
                    }
                };

                let draw = |color: u32, pos_offset: (f32, f32) | {
                    let mut pos = text_pos;

                    pos.x += pos_offset.0;
                    pos.y += pos_offset.1;

                    ImDrawList_AddTextFontPtr(
                        draw_list,
                        font,
                        font_size,
                        pos,
                        color,
                        text.as_ptr(),
                        null(),
                        0.0,
                        null(),
                    );
                };

                let shadow_color = 0xDD000000;
                match data.style {
                    TextStyle::Shadow => {
                        draw(shadow_color, (1.0, 1.0));
                    },
                    TextStyle::Outlined => {
                        draw(shadow_color, (1.0, 1.0));
                        draw(shadow_color, (1.0, -1.0));
                        draw(shadow_color, (-1.0, 1.0));
                        draw(shadow_color, (-1.0, -1.0));
                        draw(shadow_color, (0.0, 1.0));
                        draw(shadow_color, (0.0, -1.0));
                        draw(shadow_color, (1.0, 0.0));
                        draw(shadow_color, (-1.0, 0.0));
                    }
                    _ => {}
                }

                draw(data.color, (0.0, 0.0));
            }
            DrawCommand::Box(data) => {
                match data.filled {
                    true => ImDrawList_AddRectFilled(
                        draw_list,
                        ImVec2 { x: data.x1, y: data.y1 },
                        ImVec2 { x: data.x2, y: data.y2 },
                        data.color,
                        data.rounding,
                        ImDrawCornerFlags__ImDrawCornerFlags_All as i32,
                    ),
                    false => ImDrawList_AddRect(
                        draw_list,
                        ImVec2 { x: data.x1, y: data.y1 },
                        ImVec2 { x: data.x2, y: data.y2 },
                        data.color,
                        data.rounding,
                        ImDrawCornerFlags__ImDrawCornerFlags_All as i32,
                        data.width,
                    )
                }
            }
            DrawCommand::Circle(data) => {
                match data.filled {
                    true => ImDrawList_AddCircleFilled(
                        draw_list,
                        ImVec2 { x: data.x, y: data.y },
                        data.radius,
                        data.color,
                        0,
                    ),
                    false => ImDrawList_AddCircle(
                        draw_list,
                        ImVec2 { x: data.x, y: data.y },
                        data.radius,
                        data.color,
                        0,
                        data.width,
                    ),
                }
            }
        }
    }
}