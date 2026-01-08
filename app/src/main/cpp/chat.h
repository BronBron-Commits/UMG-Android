#ifndef CHAT_H
#define CHAT_H

#include "raylib.h"

#define CHAT_MAX_TEXT 128

typedef struct ChatState
{
    bool open;
    char text[CHAT_MAX_TEXT];
    float bubbleTimer;
} ChatState;

void Chat_Init(ChatState *chat);
void Chat_HandleInput(ChatState *chat);
void Chat_Update(ChatState *chat, float dt);
void Chat_DrawBubble(ChatState *chat, Vector2 playerPos, float cameraX);
void Chat_DrawButton(ChatState *chat);

#endif
