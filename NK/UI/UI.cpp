global UIState UI;

#define UI_MAX_RENDER_COMMAND_BUFFER_SIZE KiloBytes(16)

enum {
    UI_UNIFORM_WINDOW_SIZE = 1
};

void PrintShaderLog(GLuint Shader) {
    int Length = 0, CharsWritten = 0;
    char *Log;
    glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &Length);

    if (Length <= 0) {
        return;
    }

    Log = (char *) HeapAlloc(Length);
    glGetShaderInfoLog(Shader, Length, &CharsWritten, Log);
    Print("Shader Info Log: %s\n", Log);
    Exit(1);

    HeapFree(Log);
}

void PrintProgramLog(GLuint Program) {
    int Length = 0, CharsWritten = 0;
    char *Log;
    glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &Length);

    if (Length <= 0) {
        return;
    }

    Log = (char *) HeapAlloc(Length);
    glGetProgramInfoLog(Program, Length, &CharsWritten, Log);
    Print("Program Info Log: %s\n", Log);
    Exit(1);

    HeapFree(Log);
}

GLuint CompileShader(String Source, GLenum Type, const char *StageName) {
    GLuint Result = {};

    Result = glCreateShader(Type);
    char *Pointer = (char *) Source.Pointer;
    glShaderSource(Result, 1, &Pointer, 0);

    GLint Success = 0;

    glCompileShader(Result);
    glGetShaderiv(Result, GL_COMPILE_STATUS, &Success);
    if (Success != 1) {
        PrintShaderLog(Result);
    }

    return Result;
}

u32 LoadUIShaders() {
    u32 Result;

    String VertSource = ReadFile("NK/UI/Vertex.glsl");
    String FragSource = ReadFile("NK/UI/Fragment.glsl");

    if (!VertSource.Pointer) {
        PrintLiteral("UI vertex shader file not found!\n");
        Exit(1);
    }

    if (!FragSource.Pointer) {
        PrintLiteral("UI fragment shader file not found!\n");
        Exit(1);
    }

    GLuint VertexShader = CompileShader(VertSource, GL_VERTEX_SHADER, "vertex");
    GLuint FragmentShader = CompileShader(FragSource, GL_FRAGMENT_SHADER, "fragment");

    Result = glCreateProgram();

    glAttachShader(Result, VertexShader);
    glAttachShader(Result, FragmentShader);

    glLinkProgram(Result);

    GLint Success = 0;
    glGetProgramiv(Result, GL_LINK_STATUS, &Success);
    if (Success != 1) {
        PrintProgramLog(Result);
    }

    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

    return Result;
}

void InitUI() {
    UI.RenderCommands = CreateArena(UI_MAX_RENDER_COMMAND_BUFFER_SIZE);

    glGenVertexArrays(1, &UI.VAO);
    glBindVertexArray(UI.VAO);

    glGenBuffers(1, &UI.UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, UI.UBO);
    glBufferData(GL_UNIFORM_BUFFER, UI_MAX_RENDER_COMMAND_BUFFER_SIZE, 0, GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, UI.UBO, 0, UI_MAX_RENDER_COMMAND_BUFFER_SIZE);

    UI.Shader = LoadUIShaders();
}

void DestroyUI() {
    glDeleteProgram(UI.Shader);
    glDeleteBuffers(1, &UI.UBO);
    glDeleteVertexArrays(1, &UI.VAO);
    FreeArena(&UI.RenderCommands);
}

void PushRectangle(Vec2 Position, Vec2 Size, u32 Color, float Rounding, u32 Border) {
    UIRenderCommand *Command = PushStruct(&UI.RenderCommands, UIRenderCommand);
    Command->Position = Position;
    Command->Size = Size;
    Command->Color = Color;
    Command->Rounding = Rounding;
    Command->Border = Border;
    Command->Type = UI_RECTANGLE;
}

void PushText(String Text, Vec2 Position, u32 Color ) {
    float PosX = Position.X;
    float PosY = Position.Y;

    for (u32 i = 0; i < u32(Text.Length); ++i) {
        char Char = Text[i];
        
        // TODO: set these (scale ?)
        float GlyphWidth = 0.f;
        float GlyphHeight = 0.f;

        UIRenderCommand *Command = PushStruct(&UI.RenderCommands, UIRenderCommand);
        Command->Position = Vec2(PosX, PosY);
        Command->Size = Vec2(GlyphWidth, GlyphHeight);
        Command->Color = Color;
        Command->Type = UI_TEXT;

        PosX += GlyphWidth; // (scale?)
    }
}

void BeginUIFrame(Window *Win) {
    UI.ResetArena = BeginTempArena(&UI.RenderCommands);

    int Width = Win->Size.X;
    int Height = Win->Size.Y;

    glUseProgram(UI.Shader);
    glUniform2f(UI_UNIFORM_WINDOW_SIZE, float(Width), float(Height));

    glViewport(0, 0, Width, Height);
}

void EndUIFrame() {
    glUseProgram(UI.Shader);
    glBindVertexArray(UI.VAO);

    glBindBuffer(GL_UNIFORM_BUFFER, UI.UBO);

    u64 Size = UI.RenderCommands.Top;
    glBufferSubData(GL_UNIFORM_BUFFER, 0, Size, UI.RenderCommands.Pointer);

    u32 Elements = u32(Size / sizeof(UIRenderCommand));
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, Elements);

    EndTempArena(UI.ResetArena);
}
