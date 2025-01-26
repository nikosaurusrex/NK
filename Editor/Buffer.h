#pragma once

struct GapBuffer {
    u8 *Pointer;
    u64 Capacity;
    u64 Start; // gap start
    u64 End; // gap end
    u64 Length;

    u8 operator[](u64 Index) const {
        Assert(Index < Length);

        if (Index < Start) {
            return Pointer[Index];
        } else {
            return Pointer[Index + (End - Start)];
        }
    }

    u8 &operator[](u64 Index) {
        Assert(Index < Length + (End - Start));

        if (Index < Start) {
            return Pointer[Index];
        } else {
            return Pointer[Index + (End - Start)];
        }
    }
};

struct Pane {
    GapBuffer Buffer;

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

nkinline b32 IsWhitespace(u8 Char);

GapBuffer CreateGapBuffer(u32 Capacity);
void DestroyGapBuffer(GapBuffer *Buffer);
void LoadSourceFile(GapBuffer *Buffer, String path);

u64 InsertChar(GapBuffer *Buffer, u8 Char, u64 Position);
u64 InsertString(GapBuffer *Buffer, String Str, u64 Position);
u64 InsertLine(GapBuffer *Buffer, u64 Position, b32 AutoIndent);
u64 DeleteChar(GapBuffer *Buffer, u64 Position);
u64 DeleteCharBack(GapBuffer *Buffer, u64 Position);
u64 DeleteChars(GapBuffer *Buffer, u64 Position, u64 N);

String StringFromGapBuffer(GapBuffer *Buffer, Arena *a);
int GetLineFromGapBuffer(GapBuffer *Buffer, u64 Cursor, char *Output, int OutputSize);

u64 GetLineLength(GapBuffer *Buffer, u64 Cursor);

Pane CreatePane(u64 Capacity, float X, float Y, float Width, float Height);
void DestroyPane(Pane P);

// they not only move the Cursor but also reset cursor_store
nkinline void PaneCursorBack(Pane *P);
nkinline void PaneCursorNext(Pane *P);
nkinline void _PaneSetCursor(Pane *P, u64 Cursor);
nkinline void PaneSetCursor(Pane *P, u64 Cursor);
nkinline void PaneResetColStore(Pane *P);

u64 CursorBack(GapBuffer *Buffer, u64 Cursor);
u64 CursorNext(GapBuffer *Buffer, u64 Cursor);

// normal mode back and next. They don't change lines
u64 CursorBackNormal(GapBuffer *Buffer, u64 Cursor);
u64 CursorNextNormal(GapBuffer *Buffer, u64 Cursor);

u64 CursorLineBegin(GapBuffer *Buffer, u64 Cursor);
u64 CursorLineEnd(GapBuffer *Buffer, u64 Cursor);
u64 CursorNextLineBegin(GapBuffer *Buffer, u64 Cursor);
u64 CursorPrevLineBegin(GapBuffer *Buffer, u64 Cursor);
u64 CursorNextLineEnd(GapBuffer *Buffer, u64 Cursor);
u64 CursorPrevLineEnd(GapBuffer *Buffer, u64 Cursor);
u64 CursorColumn(GapBuffer *Buffer, u64 Cursor);

u64 CursorSkipWhitespace(u64 Cursor, GapBuffer *Buffer);

u64 CursorPrevWord(GapBuffer *Buffer, u64 Cursor);
u64 CursorEndOfWord(GapBuffer *Buffer, u64 Cursor);
u64 CursorNextWord(GapBuffer *Buffer, u64 Cursor);

u64 CursorParagraphUp(GapBuffer *Buffer, u64 Cursor);
u64 CursorParagraphDown(GapBuffer *Buffer, u64 Cursor);

u32 LineIndent(GapBuffer *Buffer, u64 Cursor);

u32 BraceMatchingIndentation(GapBuffer *Buffer, u64 Cursor);
