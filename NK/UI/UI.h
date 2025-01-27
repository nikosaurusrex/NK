#pragma once

enum {
    UI_BLACK = 0,
    UI_WHITE,
    UI_LIGHT_GRAY,

    UI_BUTTON_BG,
    UI_BUTTON_HOVERED_BG,
    UI_BUTTON_ACTIVE_BG,
    UI_BUTTON_ACTIVE_HOVERED_BG,

    UI_COLORS
};

void InitUI(Window *Win, float FontSize);
void DestroyUI();

void BeginUIFrame(Window *Win);
void EndUIFrame();

b8 UIButton(String Text, Vec2 Position, Vec2 Size);

void InitUIRendering(Window *Win, float FontSize);
void DeinitUIRendering();
void BeginUIRendering(Window *Win);
void EndUIRendering();

void DrawRectangle(Vec2 Position, Vec2 Size, u32 Color);
void DrawText(String Text, Vec2 Position, u32 Color);
void DrawTextCentered(String Text, Vec2 Position, Vec2 Size, u32 Color);
void DrawTextCenteredVertically(String Text, Vec2 Position, float Height, u32 Color);
