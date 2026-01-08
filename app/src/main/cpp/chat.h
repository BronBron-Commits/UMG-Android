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
    bool open; // Flag to indicate if the chat input is active (keyboard is open)
    Rectangle inputBox;
    Rectangle sendButton;
    Rectangle backspaceButton; 
    
    char text[CHAT_MAX_TEXT];    // Current typing text
    int length;
    
    char sentText[CHAT_MAX_TEXT]; // Text to display in bubble
    int sentLength;
    
    int activeFinger;
    float bubbleTimer;
    
    bool backspacePressed;
    float backspaceHoldTimer; // Timer for continuous delete
} ChatState;

void Chat_Init(ChatState *chat);
void Chat_Update(ChatState *chat, float dt);

bool Chat_HandleTouch(ChatState *chat, Vector2 touch, int finger);

void Chat_DrawUI(ChatState *chat);

void Chat_DrawBubble(ChatState *chat, Vector2 playerPos, float cameraX);

#endif
