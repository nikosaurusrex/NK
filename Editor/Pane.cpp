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

void PaneLoadFile(Pane *P, String File) {
    GapBuffer *Buffer = &P->Buffer;
    String Content = ReadFile(File);
    if (!Content.Pointer) {
        Print("File '%.*s' does not exist\n", (int)File.Length, File.Pointer);
        return;
    }

    P->File = File;

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
