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

    glClearColor(0.2, 0.3, 0.1, 1.0);

    while (MainWindow.Running) {
        UpdateWindow(&MainWindow);

        BeginUIFrame(&MainWindow);

        if (UIButton("Exit", Vec2(200.f, 200.f), Vec2(100.f, 50.f))) {
        }

        EndUIFrame();
    }

    DestroyUI();
    DestroyWindow(&MainWindow);
}
