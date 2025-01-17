#include "../ThirdParty/stb_build.cpp"

enum {
    UI_UNIFORM_WINDOW_SIZE = 1,
    UI_UNIFORM_TEXTURE = 2,

    UI_ATLAS_WIDTH = 512,
    UI_ATLAS_HEIGHT = 512,

    UI_RENDER_BUFFER_SIZE = KiloBytes(16)
};

enum {
    UI_RECTANGLE = 0,
    UI_TEXT
};

struct UIRenderCommand {
    Vec2 Position;
    Vec2 Size;
    u32 Color;
    float Rounding;
    u32 Border; // 8bit thickness, 24bit rgb 
    u32 Type;
    Vec2 UVPosition;
    Vec2 UVSize;
};
#define UI_RENDER_GROUP_MAX_ELEMENTS (UI_RENDER_BUFFER_SIZE/sizeof(UIRenderCommand))

struct UIRenderGroup {
    UIRenderCommand *Commands;
    UIRenderGroup *Next;
    u32 CommandCount;
};

struct UIFont {
    u32 Texture;
    stbtt_fontinfo Info;
    float Height;
    float Scale;
    float Ascent;
    stbtt_packedchar GlyphData[96];
};

enum {
    UI_INTERACTION_TOGGLE = 0,
    UI_INTERACTION_PRESS = 1
};

enum {
    UI_ACTIVE = 0x1,
    UI_HOVERED = 0x2
};

enum {
    UI_DRAW_ACTIVE = 0x1,
    UI_DRAW_HOVERED = 0x2,
    UI_DRAW_BORDER = 0x4,
    UI_DRAW_ROUNDED = 0x8
};

struct UIBox {
    Vec2 Position;
    Vec2 Size;
};

struct UIInteraction {
    union {
        b8 *Bool;
    };
    u32 Type;
};

struct UIState {
    Arena RenderGroupArena;

    UIRenderGroup *FirstRenderGroup;
    UIRenderGroup *CurrentRenderGroup;

    u32 VAO;
    u32 UBO;
    u32 Shader;

    UIFont Font;
};

global UIState UI;

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
    GLuint Result = 0;

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
    u32 Result = 0;

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

    HeapFree(VertSource.Pointer);
    HeapFree(FragSource.Pointer);

    return Result;
}

void InitUI() {
    UI.RenderGroupArena = CreateArena(Megabytes(16));

    glGenVertexArrays(1, &UI.VAO);
    glBindVertexArray(UI.VAO);

    glGenBuffers(1, &UI.UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, UI.UBO);
    glBufferData(GL_UNIFORM_BUFFER, UI_RENDER_BUFFER_SIZE, 0, GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, UI.UBO, 0, UI_RENDER_BUFFER_SIZE);

    UI.Shader = LoadUIShaders();

    // Load Font
    String FontPath = "NK/UI/Roboto.ttf";
    int FontSize = 20;

    // TODO: free somewhere
    String Content = ReadFile(FontPath);

    if (!Content.Pointer) {
        Print("Font file not found: %s!\n", FontPath);
        Exit(1);
    }

    u8 *Atlas = (u8 *) HeapAlloc(UI_ATLAS_WIDTH * UI_ATLAS_HEIGHT);

    UIFont *Font = &UI.Font;
    stbtt_pack_context PackContext;
    stbtt_PackBegin(&PackContext, Atlas, UI_ATLAS_WIDTH, UI_ATLAS_HEIGHT, 0, 1, 0);
    stbtt_PackSetOversampling(&PackContext, 2, 2);
    stbtt_PackFontRange(&PackContext, Content.Pointer, 0, FontSize, 32, 96, Font->GlyphData);
    stbtt_PackEnd(&PackContext);

    glGenTextures(1, &Font->Texture);
    glBindTexture(GL_TEXTURE_2D, Font->Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, UI_ATLAS_WIDTH, UI_ATLAS_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, Atlas);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbtt_InitFont(&Font->Info, Content.Pointer, 0);
    int Ascent, Descent, LineGap;
    stbtt_GetFontVMetrics(&Font->Info, &Ascent, &Descent, &LineGap); 

    float PixelScale = stbtt_ScaleForPixelHeight(&Font->Info, FontSize);
    Font->Height = float(Ascent - Descent) * PixelScale;
    Font->Scale = PixelScale;
    Font->Ascent = float(Ascent) * Font->Scale;

    HeapFree(Atlas);
}

void DestroyUI() {
    glDeleteTextures(1, &UI.Font.Texture);
    glDeleteProgram(UI.Shader);
    glDeleteBuffers(1, &UI.UBO);
    glDeleteVertexArrays(1, &UI.VAO);
    FreeArena(&UI.RenderGroupArena);
}

void BeginUIFrame(Window *Win) {
    int Width = Win->Size.X;
    int Height = Win->Size.Y;

    glUseProgram(UI.Shader);
    glUniform2f(UI_UNIFORM_WINDOW_SIZE, float(Width), float(Height));

    glViewport(0, 0, Width, Height);

    ResetArena(&UI.RenderGroupArena);
    UI.FirstRenderGroup = 0;
    UI.CurrentRenderGroup = 0;
}

void EndUIFrame() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(UI.Shader);
    glBindVertexArray(UI.VAO);

    glBindBuffer(GL_UNIFORM_BUFFER, UI.UBO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, UI.Font.Texture);
    glUniform1i(UI_UNIFORM_TEXTURE, 0);

    UIRenderGroup *RenderGroup = UI.FirstRenderGroup;
    while (RenderGroup) {
        u64 Size = RenderGroup->CommandCount * sizeof(UIRenderCommand);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, Size, RenderGroup->Commands);

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, RenderGroup->CommandCount);

        RenderGroup = RenderGroup->Next;
    }
}

UIRenderGroup *PushRenderGroup() {
    UIRenderGroup *Result = PushStruct(&UI.RenderGroupArena, UIRenderGroup);
    Result->Commands = PushArray(&UI.RenderGroupArena, UIRenderCommand, UI_RENDER_GROUP_MAX_ELEMENTS); 
    Result->Next = 0;
    Result->CommandCount = 0;

    return Result;
}

void PushRenderCommand(UIRenderCommand Command) {
    UIRenderGroup *RenderGroup = UI.CurrentRenderGroup;

    if (!RenderGroup) {
        RenderGroup = PushRenderGroup();

        UI.FirstRenderGroup = RenderGroup;
        UI.CurrentRenderGroup = RenderGroup;
    }

    if (RenderGroup->CommandCount >= UI_RENDER_GROUP_MAX_ELEMENTS) {
        RenderGroup = PushRenderGroup();

        UI.CurrentRenderGroup->Next = RenderGroup;
        UI.CurrentRenderGroup = RenderGroup;
    }

    RenderGroup->Commands[RenderGroup->CommandCount] = Command;
    RenderGroup->CommandCount++;
}

void PushRectangle(UIBox Box, u32 Color, float Rounding, u32 Border) {
    UIRenderCommand Command = {};
    Command.Position = Box.Position;
    Command.Size = Box.Size;
    Command.Color = Color;
    Command.Rounding = Rounding;
    Command.Border = Border;
    Command.Type = UI_RECTANGLE;
    Command.UVPosition = Vec2();
    Command.UVSize = Vec2();

    PushRenderCommand(Command);
}

void PushText(String Text, Vec2 Position, u32 Color) {
    float PosX = Position.X;
    float PosY = Position.Y;

    for (u32 i = 0; i < u32(Text.Length); ++i) {
        char Char = Text[i] - 32;

        UIFont *Font = &UI.Font;
        stbtt_aligned_quad Quad;
        stbtt_GetPackedQuad(Font->GlyphData, UI_ATLAS_WIDTH, UI_ATLAS_HEIGHT, Char, &PosX, &PosY, &Quad, 1);

        UIRenderCommand Command = {};
        Command.Position = Vec2(Quad.x0, Quad.y0);
        Command.Size = Vec2(Quad.x1 - Quad.x0, Quad.y1 - Quad.y0);
        Command.Color = Color;
        Command.Rounding = 0;
        Command.Border = 0;
        Command.Type = UI_TEXT;
        Command.UVPosition = Vec2(Quad.s0, Quad.t0);
        Command.UVSize = Vec2(Quad.s1 - Quad.s0, Quad.t1 - Quad.t0);

        PushRenderCommand(Command);
    }
}

void PushTextCentered(String Text, UIBox Box, u32 Color) {
    Vec2 Position = Box.Position;
    Vec2 Size = Box.Size;

    float TextWidth = 0;
    for (u32 i = 0; i < u32(Text.Length); ++i) {
        char Char = Text[i];

        int AdvanceWidth, LeftSideBearing;
        stbtt_GetCodepointHMetrics(&UI.Font.Info, Char, &AdvanceWidth, &LeftSideBearing);

        TextWidth += float(AdvanceWidth) * UI.Font.Scale;
    }

    Vec2 TextPos = Vec2(
        Position.X + (Size.X - TextWidth) * .5f,
        Position.Y + (Size.Y - UI.Font.Height) * .5f + UI.Font.Ascent
    );

    PushText(Text, TextPos, Color);
}

void DrawBox(UIBox Box, u32 InteractionFlags, u32 DrawFlags) {
    b32 Active = (InteractionFlags & UI_ACTIVE) && (DrawFlags & UI_DRAW_ACTIVE);
    b32 Hovered = (InteractionFlags & UI_HOVERED) && (DrawFlags & UI_DRAW_HOVERED);

    u32 Border = 0;
    if (DrawFlags & UI_DRAW_BORDER) {
        if (Hovered) {
            Border = 0x02999999;
        } else {
            Border = 0x01666666;
        }
    }

    u32 Rounding = 0;
    if (DrawFlags & UI_DRAW_ROUNDED) {
        Rounding = 10;
    }

    u32 Color = 0x161616;
    if (Active) {
        if (Hovered) {
            Color = 0x353535; 
        } else {
            Color = 0x292929; 
        }
    } else if (Hovered) {
        Color = 0x222222;
    }

    PushRectangle(Box, Color, Rounding, Border);
}

b32 IsHovered(UIBox Box) {
    Int2 MousePos = GetMousePosition();
    return MousePos.X > Box.Position.X &&
            MousePos.Y > Box.Position.Y &&
            MousePos.X < Box.Position.X + Box.Size.X &&
            MousePos.Y < Box.Position.Y + Box.Size.Y;
}

u32 HandleUIInteraction(UIBox Box, UIInteraction Interaction) {
    u32 Result = 0;

    b32 Hovered = IsHovered(Box);
    b32 Clicked = WasButtonPressed(MOUSE_BUTTON_LEFT);

    if (Hovered) {
        Result |= UI_HOVERED;
    }

    switch (Interaction.Type) {
    case UI_INTERACTION_TOGGLE: {
        b8 Value = *Interaction.Bool;
        if (Hovered && Clicked) {
            *Interaction.Bool = !Value;
        }
        if (Value) {
            Result |= UI_ACTIVE;
        }
    } break;
    case UI_INTERACTION_PRESS: {
        if (Hovered && Clicked) {
            *Interaction.Bool = 1;
            Result |= UI_ACTIVE;
        }
    } break;
    }

    return Result;
}

b8 UIButton(String Text, Vec2 Position, Vec2 Size) {
    b8 Result = 0;

    UIBox Box = {Position, Size};
    UIInteraction Interaction = {&Result, UI_INTERACTION_PRESS}; 

    u32 InteractionFlags = HandleUIInteraction(Box, Interaction);
    u32 RenderFlags = UI_DRAW_HOVERED | UI_DRAW_BORDER;

    DrawBox(Box, InteractionFlags, RenderFlags);
    PushTextCentered(Text, Box, 0xFFFFFF);

    return Result;
}

b8 UIToggleButton(b8 *Value, String Text, Vec2 Position, Vec2 Size) {
    b8 Result = 0;

    UIBox Box = {Position, Size};
    UIInteraction Interaction = {Value, UI_INTERACTION_TOGGLE}; 

    u32 InteractionFlags = HandleUIInteraction(Box, Interaction);
    u32 RenderFlags = UI_DRAW_ACTIVE | UI_DRAW_HOVERED | UI_DRAW_BORDER;

    DrawBox(Box, InteractionFlags, RenderFlags);
    PushTextCentered(Text, Box, 0xFFFFFF);

    Result = *Value;
    return Result;
}
