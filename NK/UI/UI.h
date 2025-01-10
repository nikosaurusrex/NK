#pragma once

enum {
    UI_RECTANGLE = 0,
    UI_TEXT
};

struct UIRenderCommand {
    Vec2 Position;
    Vec2 Size;
    u32 Color;
    union {
        // Rectangle
        struct {
            float Rounding;
            u32 Border; // 8bit thickness, 24bit rgb
        };
        // Text
    };
    u32 Type;
};

struct UIState {
    Arena RenderCommands;
    TempArena ResetArena;
    u32 VAO;
    u32 UBO;
    u32 Shader;
};

void BeginUIFrame(Window *Win);
void EndUIFrame();
