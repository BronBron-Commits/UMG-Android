#ifndef CHAT_H
#define CHAT_H

#include "raylib.h"
#include <stdbool.h>

#define CHAT_MAX_TEXT 128

// Define screen dimensions if not already defined
#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 480
#endif

#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 800
#endif

typedef struct ChatState {
    bool open;
    Rectangle button;
    Rectangle inputBox;
    char text[CHAT_MAX_TEXT];
    int length;
    int activeFinger;
    float bubbleTimer;
} ChatState;

void Chat_Init(ChatState *chat);
void Chat_Update(ChatState *chat, float dt);

// Returns true if the touch was consumed by chat
bool Chat_HandleTouch(ChatState *chat, Vector2 touch, int finger);

void Chat_DrawButton(ChatState *chat);
void Chat_DrawBubble(ChatState *chat, Vector2 playerPos, float cameraX);

#endif
