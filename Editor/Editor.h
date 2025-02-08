#pragma once

#define MAX_KEY_COMBINATIONS (1 << (8 + 3))

enum {
    ED_INSERT = 0,
    ED_NORMAL,
    ED_VISUAL,
    ED_VISUAL_LINE,
    ED_MODE_COUNT
};

struct EditorConfig {
    NativeString FontName;
    float FontSize;
    int StatusBarPadding;
    float EditorPadding;
    int TabSize;

    // Not set by the user
    float SpaceWidth;
    float TabWidth;
    float LineHeight;
    float StatusBarHeight;
    Font EditorFont;
};

struct TextEditor {
    Pane *ActivePane;
    Pane Panes[1];
    String Directory;
    u8 Mode;
    b8 NeedsRedraw;
};

