#include "chat.h"
#include <string.h>

void Chat_Init(ChatState *chat)
{
    chat->open = false;
    chat->text[0] = '\0';
    chat->bubbleTimer = 0.0f;
}

void Chat_HandleInput(ChatState *chat)
{
    if (IsKeyPressed(KEY_ENTER))
    {
        chat->open = !chat->open;
        chat->bubbleTimer = 3.0f;
    }

    if (!chat->open) return;

    int key = GetCharPressed();
    while (key > 0)
    {
        int len = strlen(chat->text);
        if (len < CHAT_MAX_TEXT - 1)
        {
            chat->text[len] = (char)key;
            chat->text[len + 1] = '\0';
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE))
    {
        int len = strlen(chat->text);
        if (len > 0) chat->text[len - 1] = '\0';
    }
}

void Chat_Update(ChatState *chat, float dt)
{
    if (chat->bubbleTimer > 0.0f)
        chat->bubbleTimer -= dt;
}

void Chat_DrawBubble(ChatState *chat, Vector2 playerPos, float cameraX)
{
    if (chat->bubbleTimer <= 0.0f || chat->text[0] == '\0')
        return;

    Vector2 pos = {
            playerPos.x - cameraX,
            playerPos.y - 80
    };

    int width = MeasureText(chat->text, 16) + 16;

    DrawRectangle(pos.x - width/2, pos.y - 24, width, 24, RAYWHITE);
    DrawRectangleLines(pos.x - width/2, pos.y - 24, width, 24, BLACK);
    DrawText(chat->text, pos.x - width/2 + 8, pos.y - 20, 16, BLACK);
}

void Chat_DrawButton(ChatState *chat)
{
    Rectangle btn = { 10, 10, 80, 30 };
    DrawRectangleRec(btn, Fade(SKYBLUE, 0.7f));
    DrawRectangleLinesEx(btn, 2, BLACK);
    DrawText("CHAT", btn.x + 18, btn.y + 8, 14, BLACK);
}
