#pragma once

struct Pane {
    GapBuffer Buffer;
    String File;

    u64 Cursor;
    u64 VisualCursor; // visual cursor position
    s64 CursorStore; // cursor column position to restore after moving up/down

    float X;
    float Y;
    float Width;
    float Height;

    u64 Scroll;
    u64 ScrollRenderIndex;
};

Pane CreatePane(u64 Capacity, float X, float Y, float Width, float Height);
void DestroyPane(Pane P);

// they not only move the Cursor but also reset cursor_store
nkinline void PaneCursorBack(Pane *P);
nkinline void PaneCursorNext(Pane *P);
nkinline void _PaneSetCursor(Pane *P, u64 Cursor);
nkinline void PaneSetCursor(Pane *P, u64 Cursor);
nkinline void PaneResetColStore(Pane *P);

void LoadFileToPane(Pane *P, String File);
