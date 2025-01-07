struct Input {
    bool KeysDown[512];
    bool KeysPressed[512];
    bool ButtonsDown[2];
    bool ButtonsPressed[2];

    float MouseX;
    float MouseY;
    float MouseDeltaX;
    float MouseDeltaY;

    int Scroll;
};

static Input Input;

void UpdateInput() {
    SetMemory(Input.KeysPressed, 0, sizeof(input.KeysPressed));
    SetMemory(Input.ButtonsPressed, 0, sizeof(input.ButtonsPressed));

    Input.Scroll = 0;
}

void UpdateKeyInput(u32 Key, bool IsDown, bool WasDown) {
    Input.KeysDown[Key] = IsDown;
    Input.KeysPressed[Key] = IsDown && !WasDown;
}

void UpdateCursorPos(float X, float Y) {
    Input.MouseDeltaX = X - Input.MouseX;
    Input.MouseDeltaY = Y - Input.MouseY;

    Input.MouseX = X;
    Input.MouseY = Y;
}

void UpdateButtonState(bool LeftButtonDown, bool RightButtonDown) {
    Input.ButtonsDown[0] = LeftButtonDown;
    Input.ButtonsDown[1] = RightButtonDown;

    Input.ButtonsPressed[0] = LeftButtonDown;
    Input.ButtonsPressed[1] = RightButtonDown;
}

void UpdateScroll(int Scroll) {
    Input.Scroll = Scroll;
}

bool IsKeyDown(u32 Key) {
    return Input.KeysDown[Key];
}

bool WasKeyPressed(u32 Key) {
    return Input.KeysPressed[Key];
}

bool IsButtonDown(u32 Button) {
    return Input.ButtonsDown[Button];
}

bool WasButtonPressed(u32 Button) {
    return Input.ButtonsPressed[Button];
}

MousePos GetMousePos() {
    return (MousePos){Input.MouseX, Input.MouseY};
}

MousePos GetMouseDelta() {
    return (MousePos){Input.MouseDeltaX, Input.MouseDeltaY};
}

int GetMouseScroll() {
    return Input.Scroll;
}
