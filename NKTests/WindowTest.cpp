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
    }

    DestroyWindow(&MainWindow);
}
