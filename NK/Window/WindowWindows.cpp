#ifdef USE_OPENGL
#include "OpenGLWindows.cpp"
#endif

global HCURSOR CursorHandles[2]; // 0 - arrow, 1 - hand
global InputState Input;

static LRESULT CALLBACK WindowProc(HWND Handle, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT Result = 0;

    Window *Win = (Window *) GetWindowLongPtr(Handle, GWLP_USERDATA);

    switch (uMsg) {
        case WM_DESTROY: {
            Win->Running = 0;
            return 0;
        } break;
        case WM_SIZE: {
            Win->Resized = 1;
        } break;
        case WM_CHAR: {
            WCHAR UTF16Char = (WCHAR)wParam;
            char ASCIIChar;
            int ASCIICharLength = WideCharToMultiByte(CP_ACP, 0, &UTF16Char, 1, &ASCIIChar, 1, 0, 0);
            if (ASCIICharLength == 1 && Input.TextLength + 1 < sizeof(Input.Text) - 1) {
                Input.Text[Input.TextLength] = ASCIIChar;
                Input.Text[Input.TextLength + 1] = 0;
                Input.TextLength++;
            }
        } break;
        case WM_INPUT: {
            RAWINPUT Raw;
            UINT Size = sizeof(Raw);
            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, (LPVOID) &Raw, &Size, sizeof(RAWINPUTHEADER)) == Size) {
                if (Raw.header.dwType == RIM_TYPEMOUSE) {
                    if (!(Raw.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)) {
                        Input.MouseDeltaPosition.X += Raw.data.mouse.lLastX;
                        Input.MouseDeltaPosition.Y += Raw.data.mouse.lLastY;
                    }

                    if (Raw.data.mouse.usButtonFlags & RI_MOUSE_WHEEL) {
                        SHORT WheelDelta = (SHORT)Raw.data.mouse.usButtonData;
                        int ScrollSteps = WheelDelta / 120;
                        Input.DeltaScroll += ScrollSteps;
                        Input.Scroll += ScrollSteps;
                    }

                    if (Raw.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN) {
                        Input.Buttons[MOUSE_BUTTON_LEFT].Pressed = 1;
                        Input.Buttons[MOUSE_BUTTON_LEFT].Down = 1;
                    }
                    if (Raw.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP) {
                        Input.Buttons[MOUSE_BUTTON_LEFT].Released = 1;
                        Input.Buttons[MOUSE_BUTTON_LEFT].Down = 0;
                    }

                    if (Raw.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN) {
                        Input.Buttons[MOUSE_BUTTON_RIGHT].Pressed = 1;
                        Input.Buttons[MOUSE_BUTTON_RIGHT].Down = 1;
                    }
                    if (Raw.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP) {
                        Input.Buttons[MOUSE_BUTTON_RIGHT].Released = 1;
                        Input.Buttons[MOUSE_BUTTON_RIGHT].Down = 0;
                    }
                }
            }
            Result = DefWindowProc(Handle, uMsg, wParam, lParam);
        } break;
        default: {
            Result = DefWindowProc(Handle, uMsg, wParam, lParam);
        } break;
    }

    return Result;
}

b8 InitWindow(Window *Win) {
    if (!Win->Title) Win->Title = "Window";

    int PosX = Win->Position.X;
    if (!PosX) PosX = CW_USEDEFAULT;

    int PosY = Win->Position.Y;
    if (!PosY) PosY = CW_USEDEFAULT;

    int Width = Win->Size.X;
    if (!Width) Width = CW_USEDEFAULT;

    int Height = Win->Size.Y;
    if (!Height) Height = CW_USEDEFAULT;

    CursorHandles[0] = LoadCursor(0, IDC_ARROW);
    CursorHandles[1] = LoadCursor(0, IDC_HAND);

    WNDCLASSEXA WindowClass = {};
    WindowClass.cbSize = sizeof(WindowClass);
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = GetModuleHandle(0);
    WindowClass.hCursor = CursorHandles[0];
    WindowClass.lpszClassName = "PlatformWindowClass";

    if (Win->OpenGL) {
        WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    } else {
        WindowClass.style = CS_OWNDC;
    }

    if (!RegisterClassExA(&WindowClass)) {
        Print("RegisterClassExA failed.\n");
        return 0;
    }

    if (Width != CW_USEDEFAULT && Height != CW_USEDEFAULT) {
        RECT Rect = {0, 0, Width, Height};
        if (AdjustWindowRect(&Rect, WS_OVERLAPPEDWINDOW, FALSE)) {
            Width = Rect.right - Rect.left;
            Height = Rect.bottom - Rect.top;
        }
    }

    HWND Handle = CreateWindowExA(0,
                                WindowClass.lpszClassName,
                                Win->Title,
                                WS_OVERLAPPEDWINDOW,
                                PosX,
                                PosY,
                                Width,
                                Height,
                                0,
                                0,
                                WindowClass.hInstance,
                                0);

    if (!Handle) {
        Print("CreateWindowExA failed.\n");
        return 0;
    }

    SetWindowLongPtr(Handle, GWLP_USERDATA, (LONG_PTR) Win);

    // Setup OpenGL/Vulkan support
    HDC DeviceContext = GetDC(Handle);

    PIXELFORMATDESCRIPTOR PixelFormat = {};
    PixelFormat.nSize = sizeof(PixelFormat);
    PixelFormat.nVersion = 1;
    PixelFormat.iPixelType = PFD_TYPE_RGBA;
    PixelFormat.cColorBits = 32;
    PixelFormat.cAlphaBits = 8;
    PixelFormat.cDepthBits = 24;
    PixelFormat.cStencilBits = 8;
    PixelFormat.iLayerType = PFD_MAIN_PLANE;

    if (Win->OpenGL) {
        PixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    } else {
        PixelFormat.dwFlags = PFD_DRAW_TO_WINDOW;
    }

    int Format = ChoosePixelFormat(DeviceContext, &PixelFormat);
    if (!Format) {
        Print("ChoosePixelFormat failed.\n");
        return 0;
    }

    SetPixelFormat(DeviceContext, Format, &PixelFormat);

    if (Win->OpenGL) {
        Win->WGLContext = wglCreateContext(DeviceContext);
        wglMakeCurrent(DeviceContext, Win->WGLContext);

        LoadOpenGL();
    }

    // Setup Raw Mouse Input
    RAWINPUTDEVICE RawInputDevice = {};
    RawInputDevice.usUsagePage = 0x01;
    RawInputDevice.usUsage = 0x02;
    RawInputDevice.hwndTarget = Handle;
    
    if (!RegisterRawInputDevices(&RawInputDevice, 1, sizeof(RawInputDevice))) {
        Print("Failed to register mouse raw input\n");
        return 0;
    }

    // Show Window
    ShowWindow(Handle, SW_SHOW);
    UpdateWindow(Handle);

    Win->Handle = Handle;
    Win->DeviceContext = DeviceContext;
    Win->Running = 1;
    Win->Style = WS_OVERLAPPEDWINDOW;

    return 1;
}

void DestroyWindow(Window *Win) {
    if (Win->OpenGL) {
        wglMakeCurrent(0, 0);
        wglDeleteContext(Win->WGLContext);
    }
    ReleaseDC(Win->Handle, Win->DeviceContext);
    DestroyWindow(Win->Handle);
}

void UpdateWindow(Window *Win) {
    if (Win->OpenGL) {
        SwapBuffers(Win->DeviceContext);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    Win->Resized = 0;

    Input.Text[0] = 0;
    Input.TextLength = 0;

    Input.MouseDeltaPosition.X = 0;
    Input.MouseDeltaPosition.Y = 0;
    Input.DeltaScroll = 0;

    for (int i = 0; i < WIN_MAX_BUTTONS; ++i) {
        Input.Buttons[i].Pressed = 0;
        Input.Buttons[i].Released = 0;
    }

    MSG Message;
    while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
        switch (Message.message) {
            default: {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            } break;
        }
    }

    // Window Info
    RECT ClientRect;
    GetClientRect(Win->Handle, &ClientRect);

    Win->Size.X = ClientRect.right - ClientRect.left;
    Win->Size.Y = ClientRect.bottom - ClientRect.top;

    POINT WindowPos = {ClientRect.left, ClientRect.top};
    ClientToScreen(Win->Handle, &WindowPos);

    Win->Position.X = WindowPos.x;
    Win->Position.Y = WindowPos.y;

    // Keyboard Input
    BYTE KeyStates[256];
    GetKeyboardState(KeyStates);
    for (int i = 5; i < Min(256, WIN_MAX_KEYS); ++i) {
        ButtonInput *Button = Input.Keys + i;

        b8 Down = KeyStates[i] >> 7;
        b8 WasDown = Button->Down;

        Button->Down = Down;
        Button->Pressed = !WasDown && Down;
        Button->Released = WasDown && !Down;
    }

    // Mouse Input
    POINT MousePosition;
    GetCursorPos(&MousePosition);
    MousePosition.x -= Win->Position.X;
    MousePosition.y -= Win->Position.Y;

    Input.MousePosition.X = MousePosition.x;
    Input.MousePosition.Y = MousePosition.y;
}

void ToggleFullscreen(Window *Win) {
    Win->Fullscreen = !Win->Fullscreen;

    if (Win->Fullscreen) {
        GetWindowRect(Win->Handle, &Win->Rect);
        Win->Style = GetWindowLong(Win->Handle, GWL_STYLE);

        SetWindowLong(
            Win->Handle, GWL_STYLE, Win->Style & ~(WS_CAPTION | WS_THICKFRAME));

        MONITORINFO MonitorInfo = {};
        MonitorInfo.cbSize = sizeof(MONITORINFO);

        GetMonitorInfo(MonitorFromWindow(Win->Handle, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo);

        SetWindowPos(Win->Handle,
                     HWND_TOP,
                     MonitorInfo.rcMonitor.left,
                     MonitorInfo.rcMonitor.top,
                     MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                     MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    } else {
        SetWindowLong(Win->Handle, GWL_STYLE, Win->Style);
        SetWindowPos(Win->Handle,
                     NULL,
                     Win->Rect.left,
                     Win->Rect.top,
                     Win->Rect.right - Win->Rect.left,
                     Win->Rect.bottom - Win->Rect.top,
                     SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

void SetWindowTitle(Window *Win, const char *Title) {
    SetWindowTextA(Win->Handle, Title);
}

void SetCursorToArrow() {
    SetCursor(CursorHandles[0]);
}

void SetCursorToPointer() {
    SetCursor(CursorHandles[1]);
}

char *GetTextInput(int *Length) {
    *Length = Input.TextLength;
    return Input.Text;
}

b8 IsKeyDown(u8 Key) {
    return Input.Keys[Key].Down;
}

b8 WasKeyPressed(u8 Key) {
    return Input.Keys[Key].Pressed;
}

b8 WasKeyReleased(u8 Key) {
    return Input.Keys[Key].Released;
}

b8 IsButtonDown(u8 Button) {
    return Input.Buttons[Button].Down;
}

b8 WasButtonPressed(u8 Button) {
    return Input.Buttons[Button].Pressed;
}

b8 WasButtonReleased(u8 Button) {
    return Input.Buttons[Button].Released;
}

Int2 GetMousePosition() {
    return Input.MousePosition;
}

Int2 GetMouseDeltaPosition() {
    return Input.MouseDeltaPosition;
}

int GetMouseScroll() {
    return Input.Scroll;
}

int GetMouseScrollDelta() {
    return Input.DeltaScroll;
}
