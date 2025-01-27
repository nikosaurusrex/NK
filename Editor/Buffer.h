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

nkinline b32 IsWhitespace(u8 Char);

GapBuffer CreateGapBuffer(u32 Capacity);
void DestroyGapBuffer(GapBuffer *Buffer);

u64 InsertChar(GapBuffer *Buffer, u8 Char, u64 Position);
u64 InsertString(GapBuffer *Buffer, String Str, u64 Position);
u64 InsertLine(GapBuffer *Buffer, u64 Position, b32 AutoIndent);
u64 DeleteChar(GapBuffer *Buffer, u64 Position);
u64 DeleteCharBack(GapBuffer *Buffer, u64 Position);
u64 DeleteChars(GapBuffer *Buffer, u64 Position, u64 N);

String StringFromGapBuffer(GapBuffer *Buffer, Arena *a);
int GetLineFromGapBuffer(GapBuffer *Buffer, u64 Cursor, char *Output, int OutputSize);

u64 GetLineLength(GapBuffer *Buffer, u64 Cursor);

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
