#pragma once

enum {
    UI_PRIMARY_BG = 0,
    UI_PRIMARY_FG,
    UI_SECONDARY_BG,
    UI_SECONDARY_FG,
    UI_BORDER,
    UI_SELECTION,

    UI_COLORS
};

#if OS_WINDOWS
struct IDWriteTextFormat;
struct Font {
    IDWriteTextFormat *Format;
};
#else
#error Font not implemented for OS.
#endif

void InitUI(Window *Win, float FontSize);
void DestroyUI();

Font CreateFont(NativeString Name, float Size);
void DestroyFont(Font Handle);

void BeginUIFrame(Window *Win);
void EndUIFrame();

b8 UIButton(String Text, Vec2 Position, Vec2 Size);

void InitUIRendering(Window *Win, float FontSize);
void DeinitUIRendering();
void BeginUIRendering(Window *Win);
void EndUIRendering();

void DrawRectangle(Vec2 Position, Vec2 Size, u32 Color);
void DrawBorderedRectangle(Vec2 Position, Vec2 Size, u32 Color);
void DrawText(String Text, Vec2 Position, u32 Color);
void DrawTextCentered(String Text, Vec2 Position, Vec2 Size, u32 Color);
void DrawTextCenteredVertically(String Text, Vec2 Position, float Height, u32 Color);
