enum {
    MAX_GAP_SIZE = 16,
};

nkinline int CharType(u8 Char) {
    b8 AlphaNum = (Char >= 'A' && Char <= 'Z')
        || (Char >= 'a' && Char <= 'z')
        || (Char >= '0' && Char <= '9');

    if (AlphaNum || Char == '_') {
        return 1;
    }

    if (Char == ' ' || Char == 0) {
        return 0;
    }

    return Char;
}

nkinline b32 IsWhitespace(u8 Char) {
    int S = Char == ' ';
    int N = Char == '\n';
    int R = Char == '\t';

    return S | N | R;
}

u32 UTF8Length(u8 Byte) {
    if_likely ((Byte & 0x80) == 0) return 1;
    if ((Byte & 0xE0) == 0xC0) return 2;
    if ((Byte & 0xF0) == 0xE0) return 3;
    if ((Byte & 0xF8) == 0xF0) return 4;

    return 1;
}

internal void MoveGap(GapBuffer *Buffer, u64 Position) {
    if (Position == Buffer->Start) {
        return;
    }

    if (Buffer->Start == Buffer->End) {
        Buffer->Start = Position;
        Buffer->End = Position;
        return;
    }

    if (Position < Buffer->Start) {
        u64 Move = Buffer->Start - Position;
        MoveMemory(Buffer->Pointer + Buffer->End - Move, Buffer->Pointer + Position, Move);
        Buffer->Start -= Move;
        Buffer->End -= Move;
    } else {
        u64 Move = Position - Buffer->Start;
        MoveMemory(Buffer->Pointer + Buffer->Start, Buffer->Pointer + Buffer->End, Move);
        Buffer->Start += Move;
        Buffer->End += Move;
    }
}

GapBuffer CreateGapBuffer(u32 Capacity) {
    GapBuffer Result = {};

    Capacity = AlignPow2(Capacity, GetLargePageSize());

    Result.Pointer = (u8 *) ReserveMemoryLargeIfPossible(Capacity);
    Result.Capacity = Capacity;
    Result.Start = 0;
    Result.End = MAX_GAP_SIZE;
    Result.Length = 0;

    return Result;
}

void DestroyGapBuffer(GapBuffer *Buffer) {
    ReleaseMemory(Buffer->Pointer, Buffer->Capacity);
}

void LoadSourceFile(GapBuffer *Buffer, String Path) {
    String Content = ReadFile(Path);
    if (!Content.Pointer) {
        Print("File '%.*s' does not exist\n", (int)Path.Length, Path.Pointer);
        return;
    }

    // remove carriage returns
    u8 *Source = Content.Pointer;
    u8 *SourceEnd = Content.Pointer + Content.Length;
    u8 *Dest = Buffer->Pointer;
    while (Source < SourceEnd) {
        if (*Source != '\r') {
            *Dest = *Source;
            Dest++;
            Buffer->Start++;
            Buffer->Length++;
        }
        Source++;
    }

    Buffer->Pointer[Buffer->Length] = 0;
    Buffer->End = Buffer->Start + MAX_GAP_SIZE;

    HeapFree(Content.Pointer);
}

u64 InsertChar(GapBuffer *Buffer, u8 Char, u64 Position) {
    Assert(Buffer->Length < Buffer->cap - MAX_GAP_SIZE);

    if (Position > Buffer->Length) {
        return Position;
    }

    MoveGap(Buffer, Position);

    if (Buffer->Start == Buffer->End) {
        u64 Shift = MAX_GAP_SIZE;
        MoveMemory(Buffer->Pointer + Buffer->End + Shift, Buffer->Pointer + Buffer->End, Buffer->Length - Buffer->End);
        Buffer->End += Shift;
    }
    Buffer->Pointer[Buffer->Start++] = Char;
    Buffer->Length++;

    return Position + 1;
}

u64 InsertString(GapBuffer *Buffer, String Str, u64 Position) {
    Assert(Buffer->Length + Str.Length < Buffer->cap - MAX_GAP_SIZE);

    u64 InsertLength = Str.Length;

    if (Position > Buffer->Length) {
        return Position;
    }

    MoveGap(Buffer, Position);

    while (Str.Length > 0) {
        u64 GapSize = Buffer->End - Buffer->Start;
        if (GapSize == 0) {
            u64 Shift = MAX_GAP_SIZE;
            MoveMemory(Buffer->Pointer + Buffer->End + Shift, Buffer->Pointer + Buffer->End, Buffer->Length - Buffer->End);
            Buffer->End += Shift;
            GapSize = Shift;
        }

        u64 CopyLength = Min(Str.Length, GapSize);
        CopyMemory(Buffer->Pointer + Buffer->Start, Str.Pointer, CopyLength);

        Str.Pointer += CopyLength;
        Str.Length -= CopyLength;

        Buffer->Start += CopyLength;
        Buffer->Length += CopyLength;
    }

    return Position + InsertLength;
}

u64 InsertLine(GapBuffer *Buffer, u64 Position, b32 AutoIndent) {
    if (Position > Buffer->Length) {
        return Position;
    }

    if (AutoIndent) {
        u64 Indent = LineIndent(Buffer, Position);

        u64 LCC = CursorBack(Buffer, Position);
        while (LCC > 0 && IsWhitespace((*Buffer)[LCC])) {
            LCC = CursorBack(Buffer, LCC);
        }

        u8 LastChar = (*Buffer)[LCC];

        if (LastChar == '{') {
            Indent += TAB_SIZE;
        }

        Position = InsertChar(Buffer, '\n', Position);

        for (u64 i = 0; i < Indent / TAB_SIZE; ++i) {
            Position = InsertChar(Buffer, '\t', Position);
        }

        for (u64 i = 0; i < Indent % TAB_SIZE; ++i) {
            Position = InsertChar(Buffer, ' ', Position);
        }
    } else {
        Position = InsertChar(Buffer, '\n', Position);
    }

    return Position;
}

u64 DeleteChar(GapBuffer *Buffer, u64 Position) {
    if (Position >= Buffer->Length) {
        return Position;
    }

    MoveGap(Buffer, Position);

    u32 Length = UTF8Length(Buffer->Pointer[Buffer->End]);
    Buffer->End += Length;
    Buffer->Length -= Length;

    return Position;
}

u64 DeleteCharBack(GapBuffer *Buffer, u64 Position) {
   // TODO:
    return Position;
}

u64 DeleteChars(GapBuffer *Buffer, u64 Position, u64 N) {
    if (Position + N >= Buffer->Length) {
        return Position;
    }

    MoveGap(Buffer, Position);

    while (N > 0 && Buffer->End < Buffer->Capacity) {
        u32 Length = UTF8Length(Buffer->Pointer[Buffer->End]);
        Buffer->End += Length;
        Buffer->Length -= Length;
        N--;
    }

    return Position;
}

String StringFromGapBuffer(GapBuffer *Buffer, Arena *A) {
    String Result = {};
    Result.Pointer = PushArray(A, u8, Buffer->Length + 1);
    Result.Length = Buffer->Length;

    if (Buffer->Start > 0) {
        CopyMemory(Result.Pointer, Buffer->Pointer, Buffer->Start);
    }

    u64 SPL = Buffer->Length - Buffer->Start;
    if (SPL > 0) {
        CopyMemory(Result.Pointer + Buffer->Start, Buffer->Pointer + Buffer->End, SPL);
    }
    Result.Pointer[Result.Length] = 0;

    return Result;
}

int GetLineFromGapBuffer(GapBuffer *Buffer, u64 Cursor, char *Output, int OutputSize) {
    int Length = 0;

    if (Buffer->Length > 0) {
        int LineLength = GetLineLength(Buffer, Cursor);
        u8 Char = (*Buffer)[Cursor];

        Length = Min(LineLength, OutputSize);
        for (int i = 0; i < Length; ++i) {
            Output[i] = Char;
            Cursor = CursorNext(Buffer, Cursor);
            Char = (*Buffer)[Cursor];
        }
    }

    return Length;
}

u64 GetLineLength(GapBuffer *Buffer, u64 Cursor) {
    return CursorLineEnd(Buffer, Cursor) - CursorLineBegin(Buffer, Cursor);
}

Pane CreatePane(u64 Capacity, float X, float Y, float Width, float Height) {
    Pane Result = {};
    Result.X = X;
    Result.Y = Y;
    Result.Width = Width;
    Result.Height = Height;

    Result.Buffer = CreateGapBuffer(Capacity);

    return Result;
}

void DestroyPane(Pane P) {
    DestroyGapBuffer(&P.Buffer);
}

void PaneCursorBack(Pane *P) {
    P->Cursor = CursorBackNormal(&P->Buffer, P->Cursor);
    PaneResetColStore(P);
}

void PaneCursorNext(Pane *P) {
    P->Cursor = CursorNextNormal(&P->Buffer, P->Cursor);
    PaneResetColStore(P);
}

void PaneSetCursor(Pane *P, u64 Cursor) {
    P->Cursor = Cursor;
    PaneResetColStore(P);

    // We do this every frame now
    // UpdateScroll(P);
}

void PaneResetColStore(Pane *P) {
    P->CursorStore = -1;
}

u64 CursorBack(GapBuffer *Buffer, u64 Cursor) {
    if (Cursor > 0) {
        Cursor--;
    }

    return Cursor;
}

u64 CursorNext(GapBuffer *Buffer, u64 Cursor) {
    if (Cursor < Buffer->Length) {
        Cursor++;
    }

    return Cursor;
}

u64 CursorBackNormal(GapBuffer *Buffer, u64 Cursor) {
    if (Cursor > 0) {
        if ((*Buffer)[Cursor - 1] != '\n') {
            Cursor--;
        }
    }

    return Cursor;
}

u64 CursorNextNormal(GapBuffer *Buffer, u64 Cursor) {
    if (Cursor < Buffer->Length) {
        if ((*Buffer)[Cursor + 1] != '\n') {
            Cursor++;
        }
    }

    return Cursor;
}

u64 CursorLineBegin(GapBuffer *Buffer, u64 Cursor) {
    Cursor = CursorBack(Buffer, Cursor);

    while (Cursor > 0) {
        u8 Char = (*Buffer)[Cursor];
        if (Char == '\n') {
            return CursorNext(Buffer, Cursor);
        }

        Cursor = CursorBack(Buffer, Cursor);
    }

    return 0;
}

u64 CursorLineEnd(GapBuffer *Buffer, u64 Cursor) {
    while (Cursor < Buffer->Length) {
        u8 Char = (*Buffer)[Cursor];
        if (Char == '\n') {
            return Cursor;
        }

        Cursor = CursorNext(Buffer, Cursor);
    }

    return Buffer->Length;
}

u64 CursorNextLineBegin(GapBuffer *Buffer, u64 Cursor) {
    return CursorNext(Buffer, CursorLineEnd(Buffer, Cursor));
}

u64 CursorPrevLineBegin(GapBuffer *Buffer, u64 Cursor) {
    return CursorLineBegin(Buffer, CursorBack(Buffer, CursorLineBegin(Buffer, Cursor)));
}

u64 CursorNextLineEnd(GapBuffer *Buffer, u64 Cursor) {
    return CursorLineEnd(Buffer, CursorNextLineBegin(Buffer, Cursor));
}

u64 CursorPrevLineEnd(GapBuffer *Buffer, u64 Cursor) {
    return CursorBack(Buffer, CursorLineBegin(Buffer, Cursor));
}

u64 CursorColumn(GapBuffer *Buffer, u64 Cursor) {
    return Cursor - CursorLineBegin(Buffer, Cursor);
}

internal u64 CursorSkipWhitespace(u64 Cursor, GapBuffer *Buffer) {
    u8 Char = (*Buffer)[Cursor];

    Cursor = CursorNext(Buffer, Cursor);

    Char = (*Buffer)[Cursor];

    while (IsWhitespace(Char) && Cursor < Buffer->Length) {
        Cursor = CursorNext(Buffer, Cursor);

        Char = (*Buffer)[Cursor];
    }

    return Cursor;
}

internal u64 CursorSkipWhitespaceReverse(u64 Cursor, GapBuffer *Buffer) {
    u8 Char = (*Buffer)[Cursor];

    Cursor = CursorBack(Buffer, Cursor);

    Char = (*Buffer)[Cursor];

    while (IsWhitespace(Char) && Cursor > 0) {
        Cursor = CursorBack(Buffer, Cursor);

        Char = (*Buffer)[Cursor];
    }

    return Cursor;
}

u64 CursorPrevWord(GapBuffer *Buffer, u64 Cursor) {
    u8 Char = (*Buffer)[Cursor];
    if (IsWhitespace(Char)) {
        Cursor = CursorSkipWhitespaceReverse(Cursor, Buffer);
    } else {
        int StartType = CharType(Char);

        Cursor = CursorBack(Buffer, Cursor);
        Char   = (*Buffer)[Cursor];

        if (IsWhitespace(Char)) {
            Cursor        = CursorSkipWhitespaceReverse(Cursor, Buffer);
            Char          = (*Buffer)[Cursor];
            StartType = CharType(Char);
        }

        if (CharType(Char) == StartType) {
            while (CharType(Char) == StartType && Cursor > 0) {
                Cursor = CursorBack(Buffer, Cursor);

                Char = (*Buffer)[Cursor];
            }

            if (Cursor != 0) {
                Cursor = CursorNext(Buffer, Cursor);
            }
        }
    }

    return Cursor;
}

u64 CursorEndOfWord(GapBuffer *Buffer, u64 Cursor) {
   u8 Char = (*Buffer)[Cursor];

   if (IsWhitespace(Char)) {
      Cursor = CursorSkipWhitespace(Cursor, Buffer);
   } else {
      Cursor = CursorNext(Buffer, Cursor);

      int StartType = CharType(Char);

      Char = (*Buffer)[Cursor];

      if (IsWhitespace(Char)) {
         Cursor = CursorSkipWhitespace(Cursor, Buffer);

         Char          = (*Buffer)[Cursor];
         StartType = CharType(Char);
      }

      if (CharType(Char) == StartType) {
         while (CharType(Char) == StartType && Cursor < Buffer->Length) {
            Cursor = CursorNext(Buffer, Cursor);

            Char = (*Buffer)[Cursor];
         }

         Cursor = CursorBack(Buffer, Cursor);
      }
   }

   return Cursor;
}

u64 CursorNextWord(GapBuffer *Buffer, u64 Cursor) {
    u8 Char = (*Buffer)[Cursor];

    if (IsWhitespace(Char)) {
        Cursor = CursorSkipWhitespace(Cursor, Buffer);
    } else {
        int StartType = CharType(Char);

        Cursor = CursorNext(Buffer, Cursor);

        Char = (*Buffer)[Cursor];

        if (StartType != CharType(Char)) {
            return Cursor;
        }

        if (IsWhitespace(Char)) {
            Cursor = CursorSkipWhitespace(Cursor, Buffer);
        } else {
            Cursor = CursorBack(Buffer, Cursor);
            Cursor = CursorEndOfWord(Buffer, Cursor);
            Cursor = CursorNext(Buffer, Cursor);

            Char = (*Buffer)[Cursor];

            if (IsWhitespace(Char)) {
                Cursor = CursorSkipWhitespace(Cursor, Buffer);
            }
        }
    }

    return Cursor;
}

u64 CursorParagraphUp(GapBuffer *Buffer, u64 Cursor) {
    Cursor = CursorBack(Buffer, Cursor);

    while (Cursor > 0) {
        if (Cursor > 1) {
            if ((*Buffer)[Cursor] == '\n' && (*Buffer)[Cursor - 1] == '\n') {
                return Cursor;
            }
        }

        Cursor = CursorBack(Buffer, Cursor);
    }

    return Cursor;
}

u64 CursorParagraphDown(GapBuffer *Buffer, u64 Cursor) {
    Cursor = CursorNext(Buffer, Cursor);

    while (Cursor < Buffer->Length) {
        if (Cursor < Buffer->Length - 1) {
            if ((*Buffer)[Cursor] == '\n' && (*Buffer)[Cursor + 1] == '\n') {
                return Cursor + 1;
            }
        }

        Cursor = CursorNext(Buffer, Cursor);
    }

    return Cursor;
}

u32 LineIndent(GapBuffer *Buffer, u64 Cursor) {
    u32 Result = 0;

    u64 Start = CursorLineBegin(Buffer, Cursor);
    for (u64 i = Start; i < Cursor; ++i) {
        u8 Char = (*Buffer)[i];
        if (Char == '\t') {
            Result += TAB_SIZE;
        } else if (Char == ' ') {
            Result++;
        } else {
            break;
        }
    }

    return Result;
}

u32 BraceMatchingIndentation(GapBuffer *Buffer, u64 Cursor) {
    Cursor = CursorBack(Buffer, Cursor);
    Cursor = CursorBack(Buffer, Cursor);

    u32 Score = 1;

    while (Cursor > 0) {
        u8 c = (*Buffer)[Cursor];

        if (c == '}') {
            Score++;
        }

        if (c == '{') {
            Score--;

            if (Score == 0) {
                return LineIndent(Buffer, Cursor);
            }
        }

        Cursor = CursorBack(Buffer, Cursor);
    }

    return 0;
}
