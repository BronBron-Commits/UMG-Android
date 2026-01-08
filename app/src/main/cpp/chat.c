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

    chat->button = (Rectangle){ 20, 20, 80, 48 };
    chat->inputBox = (Rectangle){ 40, SCREEN_HEIGHT - 260, SCREEN_WIDTH - 80, 44 };

    chat->open = false;
    chat->length = 0;
    chat->activeFinger = -1;
    chat->bubbleTimer = 0.0f;
    chat->text[0] = '\0';
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

    if (chat->open && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(touch, chat->inputBox))
    {
        ShowKeyboard(); 
        chat->activeFinger = finger;
        return true; // Consumed touch
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
    if (!chat->open) return;

    // Standard Char processing
    int key = GetCharPressed();
    while (key > 0)
    {
        if (chat->length < CHAT_MAX_TEXT - 1 && key >= 32 && key <= 125)
        {
            chat->text[chat->length++] = (char)key;
            chat->text[chat->length] = '\0';
            chat->bubbleTimer = 5.0f;
        }
        key = GetCharPressed();
    }

#if defined(PLATFORM_ANDROID)
    // Android fallback: Map keys to characters because GetCharPressed() might return nothing
    int pKey = GetKeyPressed();
    while (pKey > 0)
    {
        char c = 0;
        if (pKey >= KEY_A && pKey <= KEY_Z) c = 'a' + (pKey - KEY_A);
        else if (pKey >= KEY_ZERO && pKey <= KEY_NINE) c = '0' + (pKey - KEY_ZERO);
        else if (pKey == KEY_SPACE) c = ' ';
        
        if (c != 0 && chat->length < CHAT_MAX_TEXT - 1)
        {
            chat->text[chat->length++] = c;
            chat->text[chat->length] = '\0';
            chat->bubbleTimer = 5.0f;
        }
        pKey = GetKeyPressed();
    }
#endif

    if (IsKeyPressed(KEY_BACKSPACE) && chat->length > 0)
    {
        chat->length--;
        chat->text[chat->length] = '\0';
    }

    if (IsKeyPressed(KEY_ENTER))
    {
        chat->open = false;
        chat->activeFinger = -1;
        HideKeyboard();
    }

    if (chat->bubbleTimer > 0.0f)
    {
        chat->bubbleTimer -= dt;
        if (chat->bubbleTimer <= 0.0f)
        {
            chat->text[0] = '\0';
            chat->length = 0;
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
        DrawRectangleRec(chat->inputBox, WHITE);
        DrawRectangleLinesEx(chat->inputBox, 1, BLACK);
        DrawText(chat->text, (int)chat->inputBox.x + 5, (int)chat->inputBox.y + 8, 20, BLACK);
        DrawText(TextFormat("Chars: %i/%i", chat->length, CHAT_MAX_TEXT - 1), (int)chat->inputBox.x, (int)chat->inputBox.y + 50, 10, DARKGRAY);
    }
}

void Chat_DrawBubble(ChatState *chat, Vector2 playerPos, float cameraX)
{
    if (chat->length == 0 || chat->bubbleTimer <= 0.0f) return;

    int padding = 8;
    int fontSize = 18;
    int textWidth = MeasureText(chat->text, fontSize);

    Rectangle bubble = {
            playerPos.x - cameraX - textWidth / 2 - padding,
            playerPos.y - 90,
            (float)(textWidth + padding * 2),
            (float)(fontSize + padding * 2)
    };

    DrawRectangleRounded(bubble, 0.4f, 8, Fade(RAYWHITE, 0.95f));
    DrawRectangleRoundedLinesEx(bubble, 0.4f, 8, 2.0f, BLACK);

    DrawText(chat->text,
             (int)(bubble.x + padding),
             (int)(bubble.y + padding),
             fontSize,
             BLACK);
}
