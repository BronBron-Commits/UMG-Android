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
    Rectangle inputBox, sendButton;
    
    int inputHeight = 44;
    int padding = 10;
    int sendWidth = 70;
    int bottomY = chat->open ? (SCREEN_HEIGHT / 2 - inputHeight - padding) : (SCREEN_HEIGHT - inputHeight - padding);
    
    int availableWidth = SCREEN_WIDTH - (padding * 2);
    int inputWidth = availableWidth - sendWidth - padding;
    
    inputBox = (Rectangle){(float)padding, (float)bottomY, (float)inputWidth, (float)inputHeight };
    sendButton = (Rectangle){ inputBox.x + inputBox.width + padding, (float)bottomY, (float)sendWidth, (float)inputHeight };

    // If a finger is already interacting with the chat UI, only listen to that finger.
    if (chat->activeFinger != -1 && chat->activeFinger != finger) return false;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (CheckCollisionPointRec(touch, sendButton))
        {
            if (chat->open) // "SEND" button pressed
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
                HideKeyboard();
                chat->activeFinger = -1;

            } else { // "CHAT" button pressed
                chat->open = true;
                ShowKeyboard();
                chat->activeFinger = finger;
            }
            return true;
        }

        if (CheckCollisionPointRec(touch, inputBox))
        {
            if (!chat->open)
            {
                chat->open = true;
                ShowKeyboard();
            }
            chat->activeFinger = finger;
            return true; // Consumed touch
        }
        
        // Tap-away to close
        if (chat->open)
        {
             chat->open = false;
             HideKeyboard();
             chat->activeFinger = -1;
             return true; // Consumed touch
        }
    }
    
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

void Chat_DrawUI(ChatState *chat)
{
    Rectangle inputBox, sendButton;
    
    int inputHeight = 44;
    int padding = 10;
    int sendWidth = 70;
    int bottomY = chat->open ? (SCREEN_HEIGHT / 2 - inputHeight - padding) : (SCREEN_HEIGHT - inputHeight - padding);
    
    int availableWidth = SCREEN_WIDTH - (padding * 2);
    int inputWidth = availableWidth - sendWidth - padding;
    
    inputBox = (Rectangle){(float)padding, (float)bottomY, (float)inputWidth, (float)inputHeight };
    sendButton = (Rectangle){ inputBox.x + inputBox.width + padding, (float)bottomY, (float)sendWidth, (float)inputHeight };
    
    // Draw Input Box
    DrawRectangleRec(inputBox, WHITE);
    DrawRectangleLinesEx(inputBox, 2, BLACK);
    DrawText(chat->text, (int)inputBox.x + 5, (int)inputBox.y + 12, 20, BLACK);
    
    // Draw Send/Chat Button
    const char *buttonText = chat->open ? "SEND" : "CHAT";
    Color buttonColor = chat->open ? GREEN : DARKBLUE;
    DrawRectangleRec(sendButton, buttonColor);
    DrawRectangleLinesEx(sendButton, 2, BLACK);
    DrawText(buttonText, (int)sendButton.x + 12, (int)sendButton.y + 14, 14, BLACK);

    if (chat->open)
    {
        // Character count above input
        DrawText(TextFormat("%i/%i", chat->length, CHAT_MAX_TEXT - 1), (int)inputBox.x, (int)inputBox.y - 12, 10, LIGHTGRAY);

        // Draw blinking cursor
        if (((int)(GetTime()*2.0f))%2 == 0)
        {
            int textWidth = MeasureText(chat->text, 20);
            DrawRectangle((int)inputBox.x + 5 + textWidth, (int)inputBox.y + 12, 2, 20, BLACK);
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
