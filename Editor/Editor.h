#pragma once

#define MAX_KEY_COMBINATIONS (1 << (8 + 3))

enum {
    ED_INSERT = 0,
    ED_NORMAL,
    ED_VISUAL,
    ED_VISUAL_LINE,
    ED_MODE_COUNT
};

enum {
    TAB_SIZE = 4
};

struct TextEditor {
    Pane *ActivePane;
    Pane Panes[1];
    u8 Mode;
};
