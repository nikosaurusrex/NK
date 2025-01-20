#pragma once

#define MAX_KEY_COMBINATIONS (1 << (8 + 3))

struct TextEditor;
typedef void (*ShortcutFunction)(TextEditor *Ed);
struct EShortcut {
    const char *Name;
    ShortcutFunction Function;
};

struct EInputEvent {
    u32 KeyCombination;
    u8 Char;
};


struct EKeymap {
    EShortcut Shortcuts[MAX_KEY_COMBINATIONS];
};

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
    EInputEvent InputEvent;
    u8 Mode;
    EKeymap Keymaps[ED_MODE_COUNT];
};
