use std::ptr::null;
use crate::bindings::*;
use std::ffi::CString;
use crate::commands::Font;

impl crate::commands::Font {
    pub unsafe fn to_font(&self) -> *mut ImFont {
        match self {
            Font::Default => {
                igGetDefaultFont()
            },
            Font::Pixel => get_font_by_name("Pixel font").unwrap(),
            Font::Verdana => get_font_by_name("Verdana").unwrap(),
            Font::Tahoma => get_font_by_name("Tahoma").unwrap(),
            _ => unimplemented!()
        }
    }
}

pub unsafe fn init_imgui_fonts(atlas: *mut ImFontAtlas) {
    ImFontAtlas_AddFontDefault(atlas, null());
    init_font(atlas, "Pixel font", include_bytes!("fonts/smallest_pixel-7.ttf"), 10.0);
    init_font(atlas, "Verdana", include_bytes!("fonts/Verdana.ttf"), 13.0);
    init_font(atlas, "Tahoma", include_bytes!("fonts/Tahoma.ttf"), 14.0);
}

unsafe fn init_font(atlas: *mut ImFontAtlas, name: &str, bytes: &[u8], size: f32) {
    /// Convert name to c array
    let mut name_buf: [i8; 40] = [0; 40];
    let name = CString::new(name).unwrap();
    libc::strncpy(name_buf.as_mut_ptr(), name.as_ptr(), 40);

    // Create default config
    let mut config: ImFontConfig = *ImFontConfig_ImFontConfig();
    config.Name = name_buf;

    // Create font
    ImFontAtlas_AddFontFromMemoryTTF(
        atlas,
        bytes.as_ptr() as _,
        1,
        size,
        &config,
        null(),
    );
}

pub unsafe fn get_fonts() -> Vec<*mut ImFont> {
    let fonts = (*(*igGetIO()).Fonts).Fonts;
    let mut font_vec = Vec::new();
    for i in 0..fonts.Size {
        let font = *(fonts.Data).offset(i as _);
        font_vec.push(font);
    }
    font_vec
}

unsafe fn get_font_by_name(font_name: &str) -> Option<*mut ImFont> {
    for font in get_fonts() {
        let name_raw = (*(*font).ConfigData).Name.clone();
        let name = {
            let mut buf: Vec<u8> = Vec::new();
            for &i in &name_raw[..] {
                if i == '\0' as _ {
                    break;
                }
                buf.push(i as _);
            }
            String::from_utf8(buf).unwrap()
        };
        if name.to_lowercase().contains(&font_name.to_lowercase()) {
            return Some(font);
        }
    }
    None
}

