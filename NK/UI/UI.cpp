#include <dwrite.h>
#include <dxgi.h>
#include <d2d1.h>

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

void InitUI(Window *Win, float FontSize) {
    InitUIRendering(Win, FontSize);
}

void DestroyUI() {
    DeinitUIRendering();
}

void BeginUIFrame(Window *Win) {
    BeginUIRendering(Win);
}

void EndUIFrame() {
    EndUIRendering();
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

    u32 Color = UI_SECONDARY_BG;
    /*
    if (Active) {
        if (Hovered) {
            Color = UI_BUTTON_ACTIVE_HOVERED_BG; 
        } else {
            Color = UI_BUTTON_ACTIVE_BG; 
        }
    } else if (Hovered) {
        Color = UI_BUTTON_HOVERED_BG;
    }*/

    DrawRectangle(Box.Position, Box.Size, Color);
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
    UIInteraction Interaction = {{&Result}, UI_INTERACTION_PRESS}; 

    u32 InteractionFlags = HandleUIInteraction(Box, Interaction);
    u32 RenderFlags = UI_DRAW_HOVERED | UI_DRAW_BORDER;

    DrawBox(Box, InteractionFlags, RenderFlags);
    DrawTextCentered(Text, Box.Position, Box.Size, UI_PRIMARY_FG);
    
    return Result;
}

b8 UIToggleButton(b8 *Value, String Text, Vec2 Position, Vec2 Size) {
    b8 Result = 0;

    UIBox Box = {Position, Size};
    UIInteraction Interaction = {{Value}, UI_INTERACTION_TOGGLE}; 

    u32 InteractionFlags = HandleUIInteraction(Box, Interaction);
    u32 RenderFlags = UI_DRAW_ACTIVE | UI_DRAW_HOVERED | UI_DRAW_BORDER;

    DrawBox(Box, InteractionFlags, RenderFlags);
    DrawTextCentered(Text, Box.Position, Box.Size, UI_PRIMARY_FG);

    Result = *Value;
    return Result;
}
