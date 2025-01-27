enum {
    MAX_KEYMAP_CHILDREN = 128,
    SHORTCUT_NODE_BUCKET = 8,
    MAX_NORMAL_MODE_BUFFER = 12
};

typedef void (*ShortcutFunction)(TextEditor *Ed);
struct ShortcutNode {
    char Keys[SHORTCUT_NODE_BUCKET];
    ShortcutNode *Children[SHORTCUT_NODE_BUCKET]; 
    ShortcutFunction Function;
};

struct Keymap {
    ShortcutNode *Children[MAX_KEYMAP_CHILDREN]; 
    ShortcutFunction Function;
};

struct CopyBuffer {
    u8 *Pointer;
    u64 Length;
    u64 Capacity;
};

global Keymap *NormalModeMap;
global Keymap *VisualModeMap;
global ShortcutNode *ParseNode;

global CopyBuffer YoinkBuffer;

enum {
    KEYMAP_CTRL = 0x1,
    KEYMAP_ALT = 0x2,
    KEYMAP_SHIFT = 0x4
};

void ChangeToInsertMode(TextEditor *Ed) {
    Ed->Mode = ED_INSERT;
}

void ChangeToInsertModeNextChar(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;

    Ed->Mode = ED_INSERT;
    PaneCursorNext(P);
}

void ChangeToInsertModeBeginningOfLine(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, CursorSkipWhitespace(CursorLineBegin(Buffer, P->Cursor), Buffer));
    Ed->Mode = ED_INSERT;
}

void ChangeToInsertModeEndOfLine(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    u64 End = CursorLineEnd(Buffer, P->Cursor);
    PaneSetCursor(P, End);
    Ed->Mode = ED_INSERT;
}

void InsertChar(TextEditor *Ed, char Char) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    P->Cursor = InsertChar(Buffer, Char, P->Cursor);

    if (Char == '}') {
        u32 Indent = BraceMatchingIndentation(Buffer, P->Cursor);

        u64 Start = CursorLineBegin(Buffer, P->Cursor);
        u32 Leading = 0;
        for (u64 i = Start; i < P->Cursor; ++i) {
            if (IsWhitespace((*Buffer)[i])) {
                Leading++;
            } else {
                break;
            }
        }

        if (Leading > Indent) {
            u32 Del = Leading - Indent;

            DeleteChars(Buffer, Start, Del);

            PaneSetCursor(P, Start + Indent + 1);
        }
    }
}

void InsertNewLine(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, InsertLine(Buffer, P->Cursor, 1));
}

void InsertTab(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, InsertChar(Buffer, '\t', P->Cursor));
}

void DeleteCharForwards(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, DeleteChar(Buffer, P->Cursor));
}

void DeleteCharBackwards(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    if (P->Cursor == 0) {
        return;
    }

    PaneSetCursor(P, DeleteChar(Buffer, P->Cursor - 1));
}

void MoveCursorLeft(TextEditor *Ed) {
    PaneCursorNext(Ed->ActivePane);
}

void MoveCursorRight(TextEditor *Ed) {
    PaneCursorNext(Ed->ActivePane);
}

void MoveCursorUp(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    if (P->CursorStore < 0) {
        P->CursorStore = (s64) CursorColumn(Buffer, P->Cursor);
    }

    u64 BeginningOfPrevLine = CursorPrevLineBegin(Buffer, P->Cursor);
    u64 PrevLineLength = GetLineLength(Buffer, BeginningOfPrevLine);

    P->Cursor = BeginningOfPrevLine + Min(PrevLineLength, (u64)P->CursorStore);
}

void MoveCursorDown(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    if (P->CursorStore < 0) {
        P->CursorStore = (s64) CursorColumn(Buffer, P->Cursor);
    }

    u64 BeginningOfNextLine = CursorNextLineBegin(Buffer, P->Cursor);
    u64 NextLineLength = GetLineLength(Buffer, BeginningOfNextLine);

    P->Cursor = BeginningOfNextLine + Min(NextLineLength, (u64)P->CursorStore);
}

void ChangeToNormalMode(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;

    Ed->Mode = ED_NORMAL;
    ParseNode = 0;

    PaneCursorBack(P);
}

void ClearNormalMode(TextEditor *Ed) {
    ParseNode = 0;
}

void MoveCursorBackLineStop(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, CursorBackNormal(Buffer, P->Cursor));
}

void MoveCursorNextLineStop(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, CursorNextNormal(Buffer, P->Cursor));
}

void GotoBufferBegin(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;

    PaneSetCursor(P, 0);
}

void GotoBufferEnd(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, Buffer->Length);
}

void GotoNextWord(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, CursorNextWord(Buffer, P->Cursor));
}

void GotoPrevWord(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, CursorPrevWord(Buffer, P->Cursor));
}

void GotoEndOfWord(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, CursorEndOfWord(Buffer, P->Cursor));
}

void InsertNewLineBefore(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    P->Cursor = CursorPrevLineEnd(Buffer, P->Cursor);
    PaneSetCursor(P, InsertLine(Buffer, P->Cursor, 1));
    Ed->Mode = ED_INSERT;
}

void InsertNewLineAfter(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    P->Cursor = CursorLineEnd(Buffer, P->Cursor);
    PaneSetCursor(P, InsertLine(Buffer, P->Cursor, 1));
    Ed->Mode = ED_INSERT;
}

void SkipParagraphUp(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, CursorParagraphUp(Buffer, P->Cursor));
}

void SkipParagraphDown(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, CursorParagraphDown(Buffer, P->Cursor));
}

void ChangeToVisualMode(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;

    Ed->Mode = ED_VISUAL;
    P->VisualCursor = P->Cursor;
    ParseNode = 0;
}

void PasteYoinkBufferNextChar(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneCursorNext(P);

    if (YoinkBuffer.Length > 0) {
        P->Cursor = InsertString(Buffer, String(YoinkBuffer.Pointer, YoinkBuffer.Length), P->Cursor);
    }
}

void PasteYoinkBuffer(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    if (YoinkBuffer.Length > 0) {
        P->Cursor = InsertString(Buffer, String(YoinkBuffer.Pointer, YoinkBuffer.Length), P->Cursor);
    }
}

void CopySelection(Pane *P, GapBuffer *Buffer) {
    u64 VisualStart, VisualEnd;
    if (P->Cursor > P->VisualCursor) {
        VisualStart = P->VisualCursor;
        VisualEnd = P->Cursor;
    } else {
        VisualStart = P->Cursor;
        VisualEnd = P->VisualCursor;
    }

    u64 VisualLength = VisualEnd - VisualStart + 1;
    YoinkBuffer.Length = VisualLength;

    if (YoinkBuffer.Capacity < VisualLength) {
        YoinkBuffer.Capacity = VisualLength;
        HeapFree(YoinkBuffer.Pointer);
        YoinkBuffer.Pointer = (u8 *) HeapAlloc(YoinkBuffer.Capacity);
    }

    for (u64 i = 0; i < VisualLength; ++i) {
        YoinkBuffer.Pointer[i] = (*Buffer)[VisualStart + i];
    }
}

void DeleteSelection(Pane *P, GapBuffer *Buffer) {
    u64 VisualStart, VisualEnd;
    if (P->Cursor > P->VisualCursor) {
        VisualStart = P->VisualCursor;
        VisualEnd = P->Cursor;
    } else {
        VisualStart = P->Cursor;
        VisualEnd = P->VisualCursor;
    }

    u64 VisualLength = VisualEnd - VisualStart + 1;
    
    DeleteChars(Buffer, VisualStart, VisualLength);
}

void YoinkInVisualMode(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    CopySelection(P, Buffer);

    Ed->Mode = ED_NORMAL;
    P->Cursor = P->VisualCursor;
    ParseNode = 0;
}

void DeleteInVisualMode(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    CopySelection(P, Buffer);
    DeleteSelection(P, Buffer);

    Ed->Mode = ED_NORMAL;
    P->Cursor = P->VisualCursor;
    ParseNode = 0;
}

void ChangeInVisualMode(TextEditor *Ed) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    CopySelection(P, Buffer);
    DeleteSelection(P, Buffer);

    Ed->Mode = ED_INSERT;
    P->Cursor = P->VisualCursor;
    ParseNode = 0;
}

void OpenExplorer(TextEditor *Ed) {
}

void InsertShortcut(Arena *A, Keymap *Kmap, const char *Shortcut, ShortcutFunction Function) {
    const char *Pointer = Shortcut;

    // Top level
    u8 Index = *Pointer;
    ShortcutNode *Current = Kmap->Children[Index];
    if (!Current) {
        Current = PushStruct(A, ShortcutNode);
        Kmap->Children[Index] = Current;
    }
    Pointer++;

    // Buckets
    while (*Pointer) {
        char Key = *Pointer;

        for (int i = 0; i < SHORTCUT_NODE_BUCKET; ++i) {
            if (Current->Keys[i] == Key) {
                Current = Current->Children[i];
                break;
            } else if (Current->Keys[i] == 0) {
                Current->Keys[i] = Key;
                Current->Children[i] = PushStruct(A, ShortcutNode);
                Current = Current->Children[i];
                break;
            }
        }

        Pointer++;
    }

    Current->Function = Function;
}

void CreateKeymaps(Arena *A) {
    NormalModeMap = PushStruct(A, Keymap);
    InsertShortcut(A, NormalModeMap, "h", MoveCursorBackLineStop);
    InsertShortcut(A, NormalModeMap, "l", MoveCursorNextLineStop);
    InsertShortcut(A, NormalModeMap, "j", MoveCursorDown);
    InsertShortcut(A, NormalModeMap, "k", MoveCursorUp);
    InsertShortcut(A, NormalModeMap, "x", DeleteCharForwards);
    InsertShortcut(A, NormalModeMap, "w", GotoNextWord);
    InsertShortcut(A, NormalModeMap, "e", GotoEndOfWord);
    InsertShortcut(A, NormalModeMap, "b", GotoPrevWord);
    InsertShortcut(A, NormalModeMap, "i", ChangeToInsertMode);
    InsertShortcut(A, NormalModeMap, "I", ChangeToInsertModeBeginningOfLine);
    InsertShortcut(A, NormalModeMap, "a", ChangeToInsertModeNextChar);
    InsertShortcut(A, NormalModeMap, "A", ChangeToInsertModeEndOfLine);
    InsertShortcut(A, NormalModeMap, "gg", GotoBufferBegin);
    InsertShortcut(A, NormalModeMap, "G", GotoBufferEnd);
    InsertShortcut(A, NormalModeMap, "o", InsertNewLineAfter);
    InsertShortcut(A, NormalModeMap, "O", InsertNewLineBefore);
    InsertShortcut(A, NormalModeMap, "{", SkipParagraphUp);
    InsertShortcut(A, NormalModeMap, "}", SkipParagraphDown);
    InsertShortcut(A, NormalModeMap, "v", ChangeToVisualMode);
    InsertShortcut(A, NormalModeMap, "p", PasteYoinkBufferNextChar);
    InsertShortcut(A, NormalModeMap, "P", PasteYoinkBuffer);
    InsertShortcut(A, NormalModeMap, " e", OpenExplorer);

    VisualModeMap = PushStruct(A, Keymap);
    InsertShortcut(A, VisualModeMap, "h", MoveCursorBackLineStop);
    InsertShortcut(A, VisualModeMap, "l", MoveCursorNextLineStop);
    InsertShortcut(A, VisualModeMap, "j", MoveCursorDown);
    InsertShortcut(A, VisualModeMap, "k", MoveCursorUp);
    InsertShortcut(A, VisualModeMap, "w", GotoNextWord);
    InsertShortcut(A, VisualModeMap, "e", GotoEndOfWord);
    InsertShortcut(A, VisualModeMap, "b", GotoPrevWord);
    InsertShortcut(A, VisualModeMap, "gg", GotoBufferBegin);
    InsertShortcut(A, VisualModeMap, "G", GotoBufferEnd);
    InsertShortcut(A, VisualModeMap, "{", SkipParagraphUp);
    InsertShortcut(A, VisualModeMap, "}", SkipParagraphDown);
    InsertShortcut(A, VisualModeMap, "y", YoinkInVisualMode);
    InsertShortcut(A, VisualModeMap, "d", DeleteInVisualMode);
    InsertShortcut(A, VisualModeMap, "c", ChangeInVisualMode);

    // Allocate CopyBuffer
    YoinkBuffer.Capacity = KiloBytes(2);
    YoinkBuffer.Pointer = (u8 *) HeapAlloc(YoinkBuffer.Capacity);
    YoinkBuffer.Length = 0;
}

void HandleInsertMode(TextEditor *Ed, u8 Key, u32 ModBits) {
    if (' ' <= Key && Key <= '~') {
        InsertChar(Ed, Key);
    } else {
        switch (Key) {
        case KEY_ENTER: InsertNewLine(Ed); break;
        case KEY_TAB: InsertTab(Ed); break;
        case KEY_BACKSPACE: DeleteCharBackwards(Ed); break;
        case KEY_ESCAPE: ChangeToNormalMode(Ed); break;
        case KEY_LEFT: MoveCursorLeft(Ed); break;
        case KEY_RIGHT: MoveCursorRight(Ed); break;
        case KEY_UP: MoveCursorUp(Ed); break;
        case KEY_DOWN: MoveCursorDown(Ed); break;
        }
    }
}

void DispatchShortcut(TextEditor *Ed, Keymap *Kmap, u8 Key, u32 ModBits) {
    if (!ParseNode) {
        ParseNode = Kmap->Children[Key];
    } else {
        for (int i = 0; i < SHORTCUT_NODE_BUCKET; ++i) {
            if (ParseNode->Keys[i] == Key) {
                ParseNode = ParseNode->Children[i];
                break;
            }
        }
    }
    if (ParseNode && ParseNode->Function) {
        ParseNode->Function(Ed);
        ParseNode = 0;
    }
}

void HandleShortcut(TextEditor *Ed, u8 Key, u32 ModBits) {
    switch (Ed->Mode) {
    case ED_INSERT: {
        HandleInsertMode(Ed, Key, ModBits);
    } break;
    case ED_NORMAL: {
        if (Key != KEY_ESCAPE) {
            DispatchShortcut(Ed, NormalModeMap, Key, ModBits);
        } else {
            ParseNode = 0;
        }
    } break;
    case ED_VISUAL: {
        if (Key != KEY_ESCAPE) {
            DispatchShortcut(Ed, VisualModeMap, Key, ModBits);
        } else {
            ChangeToNormalMode(Ed);
            ParseNode = 0;
        }
    } break;
    }
}
