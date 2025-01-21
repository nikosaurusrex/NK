#define CTRL  (1 << 8)
#define ALT   (1 << 9)
#define SHIFT (1 << 10)

#define MAX_NORMAL_LENGTH 8
global u8  NormalBuffer[MAX_NORMAL_LENGTH];
global u32 NormalIndex = 0;

#define SHORTCUT(Name) \
    internal void ShortcutFunction##Name(TextEditor *Ed); \
    EShortcut    Shortcut##Name = {#Name, ShortcutFunction##Name}; \
    void        ShortcutFunction##Name(TextEditor *Ed)

internal b32 NormalModeGetShortcut(TextEditor *Ed, EShortcut *Out);

SHORTCUT(Null) {
}

SHORTCUT(InsertChar) {
    EInputEvent InputEvent = Ed->InputEvent;

    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    u8 Char = InputEvent.Char;

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

SHORTCUT(CursorLeft) {
    PaneCursorNext(Ed->ActivePane);
}

SHORTCUT(CursorRight) {
    PaneCursorNext(Ed->ActivePane);
}

SHORTCUT(CursorUp) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    if (P->CursorStore < 0) {
        P->CursorStore = (s64) CursorColumn(Buffer, P->Cursor);
    }

    u64 BeginningOfPrevLine = CursorPrevLineBegin(Buffer, P->Cursor);
    u64 PrevLineLength = GetLineLength(Buffer, BeginningOfPrevLine);

    P->Cursor = BeginningOfPrevLine + Min(PrevLineLength, (u64)P->CursorStore);
}

SHORTCUT(CursorDown) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    if (P->CursorStore < 0) {
        P->CursorStore = (s64) CursorColumn(Buffer, P->Cursor);
    }

    u64 BeginningOfNextLine = CursorNextLineBegin(Buffer, P->Cursor);
    u64 NextLineLength = GetLineLength(Buffer, BeginningOfNextLine);

    P->Cursor = BeginningOfNextLine + Min(NextLineLength, (u64)P->CursorStore);
}

SHORTCUT(DeleteForwards) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, DeleteChar(Buffer, P->Cursor));
}

SHORTCUT(DeleteBackwards) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    if (P->Cursor == 0) {
        return;
    }

    PaneSetCursor(P, DeleteChar(Buffer, P->Cursor - 1));
}

SHORTCUT(InsertNewLine) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, InsertLine(Buffer, P->Cursor, 1));
}

SHORTCUT(InsertTab) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, InsertChar(Buffer, '\t', P->Cursor));
}

SHORTCUT(NormalMode) {
    Ed->Mode = ED_NORMAL;
    NormalIndex = 0;
    NormalBuffer[NormalIndex] = 0;
}

SHORTCUT(NormalCursorBack) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, CursorBackNormal(Buffer, P->Cursor));
}

SHORTCUT(NormalCursorNext) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, CursorNextNormal(Buffer, P->Cursor));
}

SHORTCUT(InsertBeginningOfLine) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, CursorLineBegin(Buffer, P->Cursor));
    Ed->Mode = ED_INSERT;
}

SHORTCUT(InsertEndOfLine) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    u64 End = CursorLineEnd(Buffer, P->Cursor);
    PaneSetCursor(P, End);
    Ed->Mode = ED_INSERT;
}

SHORTCUT(InsertMode) {
    Ed->Mode = ED_INSERT;
}

SHORTCUT(InsertModeNext) {
    Pane *P = Ed->ActivePane;

    Ed->Mode = ED_INSERT;
    PaneCursorNext(P);
}

SHORTCUT(GoWordNext) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, CursorNextWord(Buffer, P->Cursor));
}

SHORTCUT(GoWordEnd) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, CursorEndOfWord(Buffer, P->Cursor));
}

SHORTCUT(GoWordPrev) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, CursorPrevWord(Buffer, P->Cursor));
}

SHORTCUT(GotoBufferBegin) {
    Pane *P = Ed->ActivePane;

    PaneSetCursor(P, 0);
}

SHORTCUT(GotoBufferEnd) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, Buffer->Length);
}

SHORTCUT(NewLineBefore) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    P->Cursor = CursorPrevLineEnd(Buffer, P->Cursor);
    PaneSetCursor(P, InsertLine(Buffer, P->Cursor, 1));
    Ed->Mode = ED_INSERT;
}

SHORTCUT(NewLineAfter) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    P->Cursor = CursorLineEnd(Buffer, P->Cursor);
    PaneSetCursor(P, InsertLine(Buffer, P->Cursor, 1));
    Ed->Mode = ED_INSERT;
}

SHORTCUT(SkipParagraphUp) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, CursorParagraphUp(Buffer, P->Cursor));
}

SHORTCUT(SkipParagraphDown) {
    Pane *P = Ed->ActivePane;
    GapBuffer *Buffer = &P->Buffer;

    PaneSetCursor(P, CursorParagraphDown(Buffer, P->Cursor));
}

SHORTCUT(NormalModeClear) {
    NormalIndex = 0;
    NormalBuffer[NormalIndex] = 0;
}

SHORTCUT(NormalHandle) {
    if (NormalIndex + 2 >= MAX_NORMAL_LENGTH) {
        return;
    }

    EInputEvent InputEvent = Ed->InputEvent;
    char Char = InputEvent.Char;
    u32 KeyCombination = InputEvent.KeyCombination;

    if (KeyCombination & CTRL) {
        NormalBuffer[NormalIndex] = '^';
        NormalBuffer[NormalIndex] = Char;
        NormalIndex += 2;
    } else {
        NormalBuffer[NormalIndex] = Char;
        NormalIndex++;
    }

    NormalBuffer[NormalIndex] = 0;

    EShortcut Shortcut;
    b32 Exists = NormalModeGetShortcut(Ed, &Shortcut);
    if (Exists) {
        Shortcut.Function(Ed);
        ShortcutFunctionNormalModeClear(Ed);
    }
}

SHORTCUT(VisualMode) {
    Pane *P = Ed->ActivePane;

    Ed->Mode = ED_VISUAL;

    P->Visual = P->Cursor;
}

SHORTCUT(VisualModeLine) {
    Ed->Mode = ED_VISUAL_LINE;
}

SHORTCUT(VisualLineDown) {
    ShortcutFunctionCursorDown(Ed);
}

SHORTCUT(VisualLineUp) {
}

SHORTCUT(VisualLineBufferBegin) {
}

SHORTCUT(VisualLineBufferEnd) {
}

SHORTCUT(VisualDelete) {
}

SHORTCUT(VisualYoink) {
}

SHORTCUT(YoinkSelection) {
}

SHORTCUT(YoinkPaste) {
}

/* TODO: definetly rework this! */
b32 NormalModeGetShortcut(TextEditor *Ed, EShortcut *Shortcut) {
    switch (NormalBuffer[0]) {
    // one letter shortcuts
    case 'x':
        *Shortcut = ShortcutDeleteForwards;
        break;
    case 'v':
        *Shortcut = ShortcutVisualMode;
        break;
    case 'V':
        *Shortcut = ShortcutVisualModeLine;
        break;
    case 'h':
        *Shortcut = ShortcutNormalCursorBack;
        break;
    case 'l':
        *Shortcut = ShortcutNormalCursorNext;
        break;
    case 'j':
        *Shortcut = ShortcutCursorDown;
        break;
    case 'k':
        *Shortcut = ShortcutCursorUp;
        break;
    case 'w':
        *Shortcut = ShortcutGoWordNext;
        break;
    case 'e':
        *Shortcut = ShortcutGoWordEnd;
        break;
    case 'b':
        *Shortcut = ShortcutGoWordPrev;
        break;
    case 'y':
        *Shortcut = ShortcutYoinkSelection;
        break;
    case 'p':
        *Shortcut = ShortcutYoinkPaste;
        break;
    case 'I':
        *Shortcut = ShortcutInsertBeginningOfLine;
        break;
    case 'A':
        *Shortcut = ShortcutInsertEndOfLine;
        break;
    case 'i':
        *Shortcut = ShortcutInsertMode;
        break;
    case 'a':
        *Shortcut = ShortcutInsertModeNext;
        break;
    case 'o':
        *Shortcut = ShortcutNewLineAfter;
        break;
    case 'O':
        *Shortcut = ShortcutNewLineBefore;
        break;
    case 'G':
        *Shortcut = ShortcutGotoBufferEnd;
        break;
    case '{':
        *Shortcut = ShortcutSkipParagraphUp;
        break;
    case '}':
        *Shortcut = ShortcutSkipParagraphDown;
        break;
    default: {
        // two letter Shortcuts
        char Char1 = NormalBuffer[0];
        if (NormalIndex == 2 && !IsDigit(Char1)) {
            char Char2 = NormalBuffer[1];

            if (Char1 == 'g' && Char2 == 'g') {
                ShortcutFunctionGotoBufferBegin(Ed);
                ShortcutFunctionNormalModeClear(Ed);
                return 0;
            } else if (Char1 == 'd') {
                if (Char2 == 'd') {
                    ShortcutFunctionNormalModeClear(Ed);
                    return 0;
                } else if (Char2 == 'w') {
                    ShortcutFunctionNormalModeClear(Ed);
                    return 0;
                } else {
                    return 0;
                }
            } else if (Char1 == 'c' && Char2 == 'w') {
                ShortcutFunctionNormalModeClear(Ed);
                ShortcutFunctionInsertMode(Ed);
                return 0;
            } else {
                return 0;
            }
        } else {
            return 0;
        }
    }
    }

    return 1;
}

EShortcut *
KeymapGetShortcut(EKeymap *Keymap, u32 KeyCombination) {
    Assert(KeyCombination <= MAX_KEY_COMBINATIONS);

    return Keymap->Shortcuts + KeyCombination;
}

void FillKeymapNull(EKeymap *Keymap) {
    for (int i = 0; i < MAX_KEY_COMBINATIONS; ++i) {
        Keymap->Shortcuts[i] = ShortcutNull;
    }
}

void CreateDefaultKeymaps(TextEditor *Ed) {
    // Insert mode Keymap
    EKeymap *Keymap = Ed->Keymaps + ED_INSERT;
    FillKeymapNull(Keymap);

    for (char Char = ' '; Char <= '~'; ++Char) {
        Keymap->Shortcuts[int(Char)] = ShortcutInsertChar;
        Keymap->Shortcuts[Char | SHIFT] = ShortcutInsertChar;
    }

    Keymap->Shortcuts[KEY_ENTER] = ShortcutInsertNewLine;
    Keymap->Shortcuts[KEY_ENTER | SHIFT] = ShortcutInsertNewLine;
    Keymap->Shortcuts[KEY_TAB] = ShortcutInsertTab;
    Keymap->Shortcuts[KEY_BACKSPACE] = ShortcutDeleteBackwards;
    Keymap->Shortcuts[KEY_BACKSPACE | SHIFT] = ShortcutDeleteBackwards;
    Keymap->Shortcuts[KEY_ESCAPE] = ShortcutNormalMode;

    Keymap->Shortcuts[KEY_LEFT] = ShortcutCursorLeft;
    Keymap->Shortcuts[KEY_RIGHT] = ShortcutCursorRight;
    Keymap->Shortcuts[KEY_UP] = ShortcutCursorUp;
    Keymap->Shortcuts[KEY_DOWN] = ShortcutCursorDown;

    // Normal Keymap
    Keymap = Ed->Keymaps + ED_NORMAL;
    FillKeymapNull(Keymap);

    for (char Char = ' '; Char <= '~'; ++Char) {
        Keymap->Shortcuts[int(Char)] = ShortcutNormalHandle;
        Keymap->Shortcuts[Char | SHIFT] = ShortcutNormalHandle;
        Keymap->Shortcuts[Char | CTRL] = ShortcutNormalHandle;
    }

    Keymap->Shortcuts[KEY_ESCAPE] = ShortcutNormalModeClear;

    // Visual Keymap
    Keymap = Ed->Keymaps + ED_VISUAL;
    FillKeymapNull(Keymap);

    Keymap->Shortcuts['L'] = ShortcutNormalCursorNext;
    Keymap->Shortcuts['H'] = ShortcutNormalCursorBack;
    Keymap->Shortcuts['J'] = ShortcutCursorDown;
    Keymap->Shortcuts['K'] = ShortcutCursorUp;
    Keymap->Shortcuts['D'] = ShortcutVisualDelete;
    Keymap->Shortcuts['Y'] = ShortcutVisualYoink;
    Keymap->Shortcuts['W'] = ShortcutGoWordNext;
    Keymap->Shortcuts['E'] = ShortcutGoWordEnd;
    Keymap->Shortcuts['B'] = ShortcutGoWordPrev;
    Keymap->Shortcuts['G'] = ShortcutGotoBufferBegin;
    Keymap->Shortcuts['G' | SHIFT] = ShortcutGotoBufferEnd;
    Keymap->Shortcuts[KEY_ESCAPE] = ShortcutNormalMode;

    // Visual line
    Keymap = Ed->Keymaps + ED_VISUAL_LINE;
    FillKeymapNull(Keymap);

    Keymap->Shortcuts['J'] = ShortcutVisualLineDown;
    Keymap->Shortcuts['K'] = ShortcutVisualLineUp;
    Keymap->Shortcuts['D'] = ShortcutVisualDelete;
    Keymap->Shortcuts['Y'] = ShortcutVisualYoink;
    Keymap->Shortcuts['G'] = ShortcutVisualLineBufferBegin;
    Keymap->Shortcuts['G' | SHIFT] = ShortcutVisualLineBufferEnd;
    Keymap->Shortcuts[KEY_ESCAPE] = ShortcutNormalMode;
}
