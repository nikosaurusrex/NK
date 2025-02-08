#pragma once

#include "Keys.h"

enum {
    WIN_MAX_KEYS = 256,
    WIN_MAX_TEXT = 256,
    WIN_MAX_BUTTONS = 12,
};

struct Int2 {
    int X;
    int Y;
};

typedef void (*KeyPressCallbackFunction)(u32 Codepoint);
typedef void (*CharCallbackFunction)(char Char);

struct Window {
    const char *Title;
    Int2 Position;
    Int2 Size;
    b8 Resized;

    b8 Running;
    b8 Fullscreen;

    KeyPressCallbackFunction KeyCallback;
    CharCallbackFunction CharCallback;

#if OS_WINDOWS
    HWND Handle;
    HDC DeviceContext;
    RECT Rect;
    DWORD Style;
#endif
};

struct ButtonInput {
    b8 Down;
    b8 Pressed;
    b8 Released;
};

struct InputState {
    ButtonInput Keys[WIN_MAX_KEYS];
    ButtonInput Buttons[WIN_MAX_BUTTONS];
    
    int Scroll;
    int DeltaScroll;
    Int2 MousePosition;
    Int2 MouseDeltaPosition;

    char Text[WIN_MAX_TEXT];
    u32 TextLength;
};

b8 InitWindow(Window *Win, const char *Title, int Width, int Height);
void DestroyWindow(Window *Win);

void UpdateWindow(Window *Win);
void SwapBuffers(Window *Win);

void ToggleFullscreen(Window *Win);
void MaximizeWindow(Window *Win);
b32 IsMaximized(Window *Win);

void SetWindowTitle(Window *Win, const char *Title);

void SetCursorToArrow();
void SetCursorToPointer();

char *GetTextInput(int *Length);
b8 IsKeyDown(u8 Key);
b8 WasKeyPressed(u8 Key);
b8 WasKeyReleased(u8 Key);

b8 IsButtonDown(u8 Button);
b8 WasButtonPressed(u8 Button);
b8 WasButtonReleased(u8 Button);

Int2 GetMousePosition();
Int2 GetMouseDeltaPosition();
int GetMouseScroll();
int GetMouseScrollDelta();
