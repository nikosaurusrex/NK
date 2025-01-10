#define NK_IMPLEMENTATION
#define USE_OPENGL
#include "NK/PlatformLayer.h"
#include "NK/WindowLayer.h"
#include "NK/MathLayer.h"
#include "NK/DataStructuresLayer.h"
#include "NK/UILayer.h"

void NKMain() {
    Window MainWindow = {};
    MainWindow.Title = "Example Window";
    MainWindow.Size.X = 1280;
    MainWindow.Size.Y = 720;
    MainWindow.OpenGL = 1;

    if (!InitWindow(&MainWindow)) {
        Print("Failed to initialize window!\n");
        Exit(1);
    }

    InitUI();

    while (MainWindow.Running) {
        UpdateWindow(&MainWindow);

        BeginUIFrame(&MainWindow);

        PushRectangle(Vec2(100.f, 100.f), Vec2(200.f, 100.f), 0xFFFF00, 0.f, 0);
        PushRectangle(Vec2(100.f, 300.f), Vec2(200.f, 100.f), 0xFF0000, 15.f, 0);
        PushRectangle(Vec2(500.f, 100.f), Vec2(200.f, 100.f), 0xFF0FF, 0.f, 0x05FF0000);
        PushRectangle(Vec2(500.f, 300.f), Vec2(200.f, 100.f), 0xFF0FF, 5.f, 0x0A00FF00);

        EndUIFrame();
    }

    DestroyUI();
    DestroyWindow(&MainWindow);
}
