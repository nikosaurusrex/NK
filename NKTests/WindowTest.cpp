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

        for (int i = 0; i < 10; ++i) {
            UIButton("This is not really funny! WTF is going on. HAHAHAHAHHAH", Vec2(200.f, 50.f + i * 60), Vec2(700.f, 50.f));
        }

        EndUIFrame();
    }

    DestroyUI();
    DestroyWindow(&MainWindow);
}
