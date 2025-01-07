global HCURSOR CursorHandles[2]; // 0 - arrow, 1 - hand

static LRESULT CALLBACK WindowProc(HWND Handle, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    WindowState *State = (WindowState *) GetWindowLongPtr(Handle, GWLP_USERDATA);

    switch (uMsg) {
        case WM_CLOSE: {
            Assert(State);
            State->Running = 0;
            return 0;
        } break;
        case WM_SIZE: {
            Assert(State);

            int Width = LOWORD(lParam);
            int Height = HIWORD(lParam);

            State->Width = Width;
            State->Height = Height;
        } break;
    }

    return DefWindowProc(Handle, uMsg, wParam, lParam);
}

b8 InitWindow(WindowState *State, const char *Title, int Width, int Height) {
    CursorHandles[0] = LoadCursor(0, IDC_ARROW);
    CursorHandles[1] = LoadCursor(0, IDC_HAND);

    WNDCLASSEXA WindowClass = {};
    WindowClass.cbSize = sizeof(WindowClass);
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = GetModuleHandle(0);
    WindowClass.hCursor = CursorHandles[0];
    WindowClass.lpszClassName = "PlatformWindowClass";
    WindowClass.style = CS_OWNDC;

    if (!RegisterClassExA(&WindowClass)) {
        LogError("RegisterClassExA failed.");
        return 0;
    }

    RECT Rect = {0, 0, Width, Height};
    AdjustWindowRect(&Rect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND Handle = CreateWindowExA(0,
                                Rect.lpszClassName,
                                Title,
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                Rect.right - Rect.left,
                                Rect.bottom - Rect.top,
                                0,
                                0,
                                WindowClass.hInstance,
                                0);

    if (!Handle) {
        Print("CreateWindowExA failed.\n");
        return 0;
    }

    SetWindowLongPtr(Handle, GWLP_USERDATA, (LONG_PTR) State);

    HDC DeviceContext = GetDC(Handle);

    // TODO: check vulkan double buffer, swap buffers etc
    PIXELFORMATDESCRIPTOR PixelFormat = {};
    PixelFormat.nSize = sizeof(PixelFormat);
    PixelFormat.nVersion = 1;
    PixelFormat.dwFlags = PFD_DRAW_TO_WINDOW;
    PixelFormat.iPixelType = PFD_TYPE_RGBA;
    PixelFormat.cColorBits = 32;
    PixelFormat.cAlphaBits = 8;
    PixelFormat.cDepthBits = 24;
    PixelFormat.cStencilBits = 8;
    PixelFormat.iLayerType = PFD_MAIN_PLANE;

    int Format = ChoosePixelFormat(DeviceContext, &PixelFormat);
    if (!Format) {
        Print("ChoosePixelFormat failed.\n");
        return 0;
    }

    SetPixelFormat(DeviceContext, Format, &PixelFormat);

    ShowWindow(Handle, SW_SHOW);
    UpdateWindow(Handle);

    State->Handle = Handle;
    State->DeviceContext = DeviceContext;
    State->Running = 1;
    State->Style = WS_OVERLAPPEDWINDOW;

    return 1;
}

void DestroyWindow(WindowState *State) {
    ReleaseDC(State->Handle, state->DeviceContext);
    DestroyWindow(State->Handle);
}

void UpdateWindow(WindowState *State) {
    UpdateInput();

    MSG Message;
    while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
        switch (Message.message) {
            case WM_SYSKEYUP:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_KEYDOWN: {
                u32 KeyCode = u32(Message.wParam);
                bool WasDown = (Message.lParam & (1 << 30)) != 0;
                bool IsDown = (Message.lParam & (1 << 31)) == 0;

                UpdateKeyInput(KeyCode, IsDown, WasDown);
            } break;
            case WM_MOUSEWHEEL: {
                UpdateScroll(GET_WHEEL_DELTA_WPARAM(Message.wParam));
            } break;
            default: {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            } break;
        }
    }

    POINT CursorPos;
    GetCursorPos(&CursorPos);
    ScreenToClient(State->Handle, &CursorPos);

    UpdateCursorPos(float(CursorPos.x), float(CursorPos.y));

    bool LeftButtonDown = GetKeyState(VK_LBUTTON) & (1 << 15);
    bool RightButtonDown = GetKeyState(VK_RBUTTON) & (1 << 15);

    UpdateButtonState(LeftButtonDown, RightButtonDown);
}

void ToggleFullscreen(WindowState *State) {
    State->Fullscreen = !State->Fullscreen;

    if (State->Fullscreen) {
        GetWindowRect(State->Handle, &State->Rect);
        State->Style = GetWindowLong(State->Handle, GWL_STYLE);

        SetWindowLong(
            State->Handle, GWL_STYLE, State->Style & ~(WS_CAPTION | WS_THICKFRAME));

        MONITORINFO MonitorInfo = {};
        MonitorInfo.cbSize = sizeof(MONITORINFO);

        GetMonitorInfo(MonitorFromWindow(State->Handle, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo);

        SetWindowPos(State->Handle,
                     HWND_TOP,
                     MonitorInfo.rcMonitor.left,
                     MonitorInfo.rcMonitor.top,
                     MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                     MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    } else {
        SetWindowLong(State->Handle, GWL_STYLE, State->Style);
        SetWindowPos(State->Handle,
                     NULL,
                     State->Rect.left,
                     State->Rect.top,
                     State->Rect.right - State->Rect.left,
                     State->Rect.bottom - State->Rect.top,
                     SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

void SetWindowTitle(WindowState *State, const char *Title) {
    SetWindowTextA(State->Handle, Title);
}

void SetCursorToArrow() {
    SetCursor(CursorHandles[0]);
}

void SetCursorToPointer() {
    SetCursor(CursorHandles[1]);
}
