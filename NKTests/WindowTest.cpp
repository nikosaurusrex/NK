#define NK_IMPLEMENTATION
#include "NK/PlatformLayer.h"
#include "NK/WindowLayer.h"
#include "NK/MathLayer.h"
#include "NK/DataStructuresLayer.h"
#include "NK/GraphicsLayer.h"
#include "NK/UILayer.h"

void NKMain() {
    Window MainWindow = {};
    MainWindow.Title = "Example Window";
    MainWindow.Size.X = 1280;
    MainWindow.Size.Y = 720;

    if (!InitWindow(&MainWindow)) {
        Print("Failed to initialize window!\n");
        Exit(1);
    }

    InitGraphics(&MainWindow);
    InitUI(&MainWindow);

    while (MainWindow.Running) {
        UpdateWindow(&MainWindow);
        UpdateGraphics(&MainWindow);

        BeginUIFrame(&MainWindow);

        for (int i = 0; i < 3; ++i) {
            if (UIButton("Test Button", Vec2(200.f, 50.f + i * 60), Vec2(200.f, 50.f))) {
                Print("Button %d pressed!\n", i);
            }
        }

        static b8 SomeBool = 0;
        if (UIToggleButton(&SomeBool, "ToggleButton", Vec2(200.f, 500.f), Vec2(200.f, 50.f))) {
        }

        EndUIFrame();
    }

    DestroyUI();
    ReleaseGraphics();
    DestroyWindow(&MainWindow);
}
