#define NK_IMPLEMENTATION
#include "NK/PlatformLayer.h"
#include "NK/WindowLayer.h"

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

    while (MainWindow.Running) {
        UpdateWindow(&MainWindow);
    }

    DestroyWindow(&MainWindow);
}
