#include "chat.h"
#include "raylib.h"
#include <string.h>

#if defined(PLATFORM_ANDROID)
#include <jni.h>
#include <android/native_activity.h>
#include <android_native_app_glue.h>

// Forward declaration to get the android_app state
struct android_app* GetAndroidApp(void);

static void ShowKeyboard(void) {
    struct android_app* app = GetAndroidApp();
    if (!app || !app->activity) return;

    JNIEnv* env = NULL;
    JavaVM* vm = app->activity->vm;
    (*vm)->AttachCurrentThread(vm, &env, NULL);

    jobject activity = app->activity->clazz;
    jclass activityClass = (*env)->GetObjectClass(env, activity);

    jmethodID getSystemService = (*env)->GetMethodID(env, activityClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jstring serviceName = (*env)->NewStringUTF(env, "input_method");
    jobject inputMethodManager = (*env)->CallObjectMethod(env, activity, getSystemService, serviceName);

    jclass immClass = (*env)->GetObjectClass(env, inputMethodManager);
    jmethodID showSoftInput = (*env)->GetMethodID(env, immClass, "showSoftInput", "(Landroid/view/View;I)Z");

    jmethodID getWindow = (*env)->GetMethodID(env, activityClass, "getWindow", "()Landroid/view/Window;");
    jobject window = (*env)->CallObjectMethod(env, activity, getWindow);
    jclass windowClass = (*env)->GetObjectClass(env, window);
    jmethodID getDecorView = (*env)->GetMethodID(env, windowClass, "getDecorView", "()Landroid/view/View;");
    jobject decorView = (*env)->CallObjectMethod(env, window, getDecorView);

    (*env)->CallBooleanMethod(env, inputMethodManager, showSoftInput, decorView, 0);

    (*vm)->DetachCurrentThread(vm);
}

static void HideKeyboard(void) {
    struct android_app* app = GetAndroidApp();
    if (!app || !app->activity) return;

    JNIEnv* env = NULL;
    JavaVM* vm = app->activity->vm;
    (*vm)->AttachCurrentThread(vm, &env, NULL);

    jobject activity = app->activity->clazz;
    jclass activityClass = (*env)->GetObjectClass(env, activity);

    jmethodID getSystemService = (*env)->GetMethodID(env, activityClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jstring serviceName = (*env)->NewStringUTF(env, "input_method");
    jobject inputMethodManager = (*env)->CallObjectMethod(env, activity, getSystemService, serviceName);

    jclass immClass = (*env)->GetObjectClass(env, inputMethodManager);
    jmethodID hideSoftInputFromWindow = (*env)->GetMethodID(env, immClass, "hideSoftInputFromWindow", "(Landroid/os/IBinder;I)Z");

    jmethodID getWindow = (*env)->GetMethodID(env, activityClass, "getWindow", "()Landroid/view/Window;");
    jobject window = (*env)->CallObjectMethod(env, activity, getWindow);
    jclass windowClass = (*env)->GetObjectClass(env, window);
    jmethodID getDecorView = (*env)->GetMethodID(env, windowClass, "getDecorView", "()Landroid/view/View;");
    jobject decorView = (*env)->CallObjectMethod(env, window, getDecorView);

    jclass decorViewClass = (*env)->GetObjectClass(env, decorView);
    jmethodID getWindowToken = (*env)->GetMethodID(env, decorViewClass, "getWindowToken", "()Landroid/os/IBinder;");
    jobject windowToken = (*env)->CallObjectMethod(env, decorView, getWindowToken);

    (*env)->CallBooleanMethod(env, inputMethodManager, hideSoftInputFromWindow, windowToken, 0);

    (*vm)->DetachCurrentThread(vm);
}

#else
// Stubs for non-android platforms
static void ShowKeyboard(void) {}
static void HideKeyboard(void) {}
#endif

void Chat_Init(ChatState *chat)
{
    memset(chat, 0, sizeof(ChatState));

    // Chat open button top-left
    chat->button = (Rectangle){ 20, 20, 80, 48 };
    
    // Input box centered at the bottom
    int inputWidth = SCREEN_WIDTH - 120;
    chat->inputBox = (Rectangle){ 
        (SCREEN_WIDTH - inputWidth) / 2 - 30, // Shifted slightly left to make room for send button
        SCREEN_HEIGHT - 350, // Move it up significantly to avoid keyboard overlap
        inputWidth, 
        44 
    };

    // Send button to the right of input box
    chat->sendButton = (Rectangle){
        chat->inputBox.x + chat->inputBox.width + 10,
        chat->inputBox.y,
        60,
        44
    };

    chat->open = false;
    chat->length = 0;
    chat->activeFinger = -1;
    chat->bubbleTimer = 0.0f;
    chat->text[0] = '\0';
    
    chat->sentLength = 0;
    chat->sentText[0] = '\0';
}

bool Chat_HandleTouch(ChatState *chat, Vector2 touch, int finger)
{
    // If a finger is already interacting with the chat UI, only listen to that finger.
    if (chat->activeFinger != -1 && chat->activeFinger != finger) return false;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(touch, chat->button))
    {
        chat->open = !chat->open;
        if (chat->open) {
            ShowKeyboard();
            chat->activeFinger = finger;
        } else {
            HideKeyboard();
            chat->activeFinger = -1;
        }
        return true; // Consumed touch
    }

    if (chat->open)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(touch, chat->inputBox))
        {
            ShowKeyboard(); 
            chat->activeFinger = finger;
            return true; // Consumed touch
        }

        // Handle Send Button
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(touch, chat->sendButton))
        {
            // Send message logic
            if (chat->length > 0)
            {
                strcpy(chat->sentText, chat->text);
                chat->sentLength = chat->length;
                chat->bubbleTimer = 5.0f;
                
                // Clear input
                chat->text[0] = '\0';
                chat->length = 0;
            }
            
            chat->open = false;
            HideKeyboard();
            chat->activeFinger = finger;
            return true;
        }
    }
    
    // If touch is released, reset the active finger
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && chat->activeFinger == finger)
    {
        chat->activeFinger = -1;
    }

    return false;
}

void Chat_Update(ChatState *chat, float dt)
{
    if (!chat->open)
    {
        // Still update timer if chat is closed but message is showing
        if (chat->bubbleTimer > 0.0f)
        {
            chat->bubbleTimer -= dt;
            if (chat->bubbleTimer <= 0.0f)
            {
                chat->sentText[0] = '\0';
                chat->sentLength = 0;
            }
        }
        return;
    }

    // Standard Char processing
    int key = GetCharPressed();
    while (key > 0)
    {
        if (chat->length < CHAT_MAX_TEXT - 1 && key >= 32 && key <= 125)
        {
            chat->text[chat->length++] = (char)key;
            chat->text[chat->length] = '\0';
        }
        key = GetCharPressed();
    }

#if defined(PLATFORM_ANDROID)
    int pKey = GetKeyPressed();
    while(pKey > 0)
    {
        char c = 0;
        if(pKey >= KEY_A && pKey <= KEY_Z) c = 'a' + (pKey - KEY_A);
        else if (pKey >= KEY_ZERO && pKey <= KEY_NINE) c = '0' + (pKey-KEY_ZERO);
        else if (pKey == KEY_SPACE) c = ' ';

        if (c != 0 && chat->length < CHAT_MAX_TEXT - 1)
        {
            chat->text[chat->length++] = c;
            chat->text[chat->length] = '\0';
        }
        pKey = GetKeyPressed();
    }
#endif

    if (IsKeyPressed(KEY_BACKSPACE) && chat->length > 0)
    {
        chat->length--;
        chat->text[chat->length] = '\0';
    }

    // ENTER key removed as the primary send method, replaced by on-screen button.
    // However, keeping it as an optional shortcut doesn't hurt.
    if (IsKeyPressed(KEY_ENTER))
    {
        if (chat->length > 0)
        {
            strcpy(chat->sentText, chat->text);
            chat->sentLength = chat->length;
            chat->bubbleTimer = 5.0f;
            
            chat->text[0] = '\0';
            chat->length = 0;
        }
        
        chat->open = false;
        chat->activeFinger = -1;
        HideKeyboard();
    }
    
    if (chat->bubbleTimer > 0.0f)
    {
        chat->bubbleTimer -= dt;
        if (chat->bubbleTimer <= 0.0f)
        {
            chat->sentText[0] = '\0';
            chat->sentLength = 0;
        }
    }
}

void Chat_DrawButton(ChatState *chat)
{
    DrawRectangleRounded(chat->button, 0.25f, 8, Fade(DARKBLUE, 0.8f));
    DrawText("CHAT",
             (int)(chat->button.x + 16),
             (int)(chat->button.y + 14),
             18,
             RAYWHITE);
    
    if (chat->open)
    {
        // Draw Input Box
        DrawRectangleRec(chat->inputBox, WHITE);
        DrawRectangleLinesEx(chat->inputBox, 2, BLACK);
        DrawText(chat->text, (int)chat->inputBox.x + 5, (int)chat->inputBox.y + 12, 20, BLACK);
        
        // Draw Send Button
        DrawRectangleRec(chat->sendButton, GREEN);
        DrawRectangleLinesEx(chat->sendButton, 2, BLACK);
        DrawText("SEND", (int)chat->sendButton.x + 8, (int)chat->sendButton.y + 14, 14, BLACK);

        // Character count below input
        DrawText(TextFormat("%i/%i", chat->length, CHAT_MAX_TEXT - 1), (int)chat->inputBox.x, (int)chat->inputBox.y + 50, 10, LIGHTGRAY);

        // Draw blinking cursor
        if (((int)(GetTime()*2.0f))%2 == 0)
        {
            int textWidth = MeasureText(chat->text, 20);
            DrawRectangle((int)chat->inputBox.x + 5 + textWidth, (int)chat->inputBox.y + 12, 2, 20, BLACK);
        }
    }
}

void Chat_DrawBubble(ChatState *chat, Vector2 playerPos, float cameraX)
{
    if (chat->sentLength == 0 || chat->bubbleTimer <= 0.0f) return;

    int padding = 8;
    int fontSize = 18;
    int textWidth = MeasureText(chat->sentText, fontSize);

    Rectangle bubble = {
            playerPos.x - cameraX - textWidth / 2 - padding,
            playerPos.y - 90,
            (float)(textWidth + padding * 2),
            (float)(fontSize + padding * 2)
    };

    DrawRectangleRounded(bubble, 0.4f, 8, Fade(RAYWHITE, 0.95f));
    DrawRectangleRoundedLinesEx(bubble, 0.4f, 8, 2.0f, BLACK);

    DrawText(chat->sentText,
             (int)(bubble.x + padding),
             (int)(bubble.y + padding),
             fontSize,
             BLACK);
}
