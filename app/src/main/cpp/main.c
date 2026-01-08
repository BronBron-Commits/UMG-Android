#include <android_native_app_glue.h>
#include <math.h>
#include "raylib.h"
#include "raymath.h"
#include "chat.h"

#if defined(PLATFORM_ANDROID)
#include <jni.h>
#include <android/native_activity.h>
// Use GetAndroidApp() from Raylib instead of declaring extern struct android_app *app;
struct android_app *GetAndroidApp(void);
#endif

/* =============================
   CONFIG (PORTRAIT)
============================= */
#define SCREEN_WIDTH   480
#define SCREEN_HEIGHT  800
#define WORLD_WIDTH    4000.0f
#define GROUND_Y       520.0f

#define GRAVITY        0.6f
#define JUMP_VELOCITY -12.0f
#define MAX_JUMPS      2

#define DAY_AMBIENT    0.40f
#define NIGHT_AMBIENT  0.75f

/* =============================
   ANDROID HAPTIC FEEDBACK
============================= */
#if defined(PLATFORM_ANDROID)
static void TriggerHapticFeedback(int durationMs)
{
    struct android_app *app = GetAndroidApp();
    if (!app || !app->activity) return;

    JNIEnv *env = NULL;
    JavaVM *vm = app->activity->vm;
    (*vm)->AttachCurrentThread(vm, &env, NULL);

    jobject activity = app->activity->clazz;
    jclass activityClass = (*env)->GetObjectClass(env, activity);

    jmethodID getService = (*env)->GetMethodID(
            env,
            activityClass,
            "getSystemService",
            "(Ljava/lang/String;)Ljava/lang/Object;"
    );

    jstring vibStr = (*env)->NewStringUTF(env, "vibrator");
    jobject vibrator = (*env)->CallObjectMethod(env, activity, getService, vibStr);

    jclass vibClass = (*env)->GetObjectClass(env, vibrator);
    jmethodID vibrate = (*env)->GetMethodID(env, vibClass, "vibrate", "(J)V");

    (*env)->CallVoidMethod(env, vibrator, vibrate, (jlong)durationMs);

    (*vm)->DetachCurrentThread(vm);
}
#endif

/* =============================
   COORDINATE TRANSFORMATION
============================= */
Vector2 TouchToGame(Vector2 touch) {
    return (Vector2) {
        .x = touch.x * (float)SCREEN_WIDTH / GetScreenWidth(),
        .y = touch.y * (float)SCREEN_HEIGHT / GetScreenHeight()
    };
}

/* =============================
   VIRTUAL JOYSTICK
============================= */
typedef struct {
    Vector2 base;
    Vector2 knob;
    float radius;
    bool active;
    Vector2 delta;
    int finger;
} VirtualJoystick;

/* =============================
   TEXT HELPERS
============================= */
void DrawOutlinedText(const char *text, int x, int y, int size, Color textColor, Color outline)
{
    DrawText(text, x-1, y,   size, outline);
    DrawText(text, x+1, y,   size, outline);
    DrawText(text, x,   y-1, size, outline);
    DrawText(text, x,   y+1, size, outline);
    DrawText(text, x,   y,   size, textColor);
}

/* =============================
   PLAYER
============================= */
void DrawPlayer(Vector2 pos, Vector2 dir, float speed, float time)
{
    float facing = (dir.x >= 0) ? 1.0f : -1.0f;

    float walkPhase = time * 8.0f * Clamp(speed, 0.0f, 1.0f);
    float legSwing = sinf(walkPhase) * 8.0f;
    float armSwing = sinf(walkPhase + PI) * 6.0f;

    Vector2 torso = { pos.x, pos.y - 10 };
    Vector2 head  = { pos.x, pos.y - 36 };
    Vector2 hip   = { pos.x, pos.y };

    Color shirt = GREEN;
    Color pants = DARKGREEN;
    Color skin  = BEIGE;

    DrawLineEx(hip,
               (Vector2){ hip.x + facing * 6 + legSwing * facing, hip.y + 22 }, 4, pants);
    DrawLineEx(hip,
               (Vector2){ hip.x - facing * 6 - legSwing * facing, hip.y + 22 }, 4, pants);

    DrawRectanglePro(
            (Rectangle){ torso.x, torso.y, 18, 28 },
            (Vector2){ 9, 14 }, 0, shirt);

    DrawLineEx(
            (Vector2){ torso.x + facing * 9, torso.y - 6 },
            (Vector2){ torso.x + facing * (15 + armSwing), torso.y + 6 }, 3, shirt);
    DrawLineEx(
            (Vector2){ torso.x - facing * 9, torso.y - 6 },
            (Vector2){ torso.x - facing * (15 + armSwing), torso.y + 6 }, 3, shirt);

    DrawCircleV(head, 10, skin);
    DrawCircleV((Vector2){ head.x + facing * 10, head.y }, 3, ORANGE);
    DrawCircleV((Vector2){ head.x + facing * 4, head.y - 2 }, 1.5f, BLACK);
}

/* =============================
   STARS
============================= */
typedef struct { Vector2 pos; float phase, speed; } Star;
#define STAR_COUNT 18

Star stars[STAR_COUNT] = {
        {{40,60},0,1.2},{{120,90},1.1,0.9},{{200,50},2.3,1.4},
        {{280,110},0.7,1.0},{{360,70},2.9,0.8},{{430,100},1.6,1.3},
        {{90,160},2.1,0.7},{{170,140},0.4,1.1},{{260,180},1.8,0.9},
        {{350,150},2.6,1.2},{{420,200},0.9,0.8},
        {{60,240},1.5,1.0},{{140,260},2.8,0.7},{{220,230},0.2,1.4},
        {{310,270},1.9,0.9},{{390,250},0.6,1.1},{{450,300},2.4,0.8},
        {{300,60},1.3,1.0}
};

void DrawStars(float nightT, float time)
{
    if (nightT <= 0.01f) return;
    BeginBlendMode(BLEND_ADDITIVE);
    for (int i = 0; i < STAR_COUNT; i++)
    {
        float twinkle = 0.6f + 0.4f * sinf(time * stars[i].speed + stars[i].phase);
        DrawCircleV(stars[i].pos, 2, Fade(RAYWHITE, nightT * twinkle));
    }
    EndBlendMode();
}

/* =============================
   BIRDS
============================= */
typedef struct { float x, y, speed, phase; } Bird;
#define BIRD_COUNT 6

Bird birds[BIRD_COUNT] = {
        { -60, 120, 0.9f, 0.0f },
        { -220, 160, 0.7f, 1.2f },
        { -140,  95, 1.1f, 2.1f },
        { -360, 140, 0.8f, 0.6f },
        { -520, 110, 1.0f, 2.7f },
        { -680, 150, 0.75f, 1.8f }
};

void UpdateBirds(float time)
{
    for (int i = 0; i < BIRD_COUNT; i++)
    {
        birds[i].x += birds[i].speed;
        birds[i].y += sinf(time * 1.2f + birds[i].phase) * 0.3f;
        if (birds[i].x > SCREEN_WIDTH + 80) birds[i].x = -100;
    }
}

void DrawBirds(float dayT, float time)
{
    if (dayT <= 0.01f) return;
    Color c = Fade(BLACK, dayT * 0.8f);

    for (int i = 0; i < BIRD_COUNT; i++)
    {
        float flap = 1.0f + sinf(time * 6.0f + birds[i].phase);
        DrawLine(birds[i].x, birds[i].y,
                 birds[i].x + 6, birds[i].y + flap, c);
        DrawLine(birds[i].x + 6, birds[i].y + flap,
                 birds[i].x + 12, birds[i].y, c);
    }
}

/* =============================
   PARALLAX
============================= */
void DrawParallax(float cameraX)
{
    float farX = -cameraX * 0.2f;
    for (int i = -1; i < 12; i++)
        DrawTriangle(
                (Vector2){farX+i*400+200,240},
                (Vector2){farX+i*400,400},
                (Vector2){farX+i*400+400,400},
                DARKPURPLE);

    float midX = -cameraX * 0.4f;
    for (int i = -1; i < 16; i++)
        DrawCircle(midX+i*260,420,160,DARKBLUE);
}

/* =============================
   ANDROID ENTRY POINT
============================= */
int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "U-MG Android (Portrait)");
    SetTargetFPS(60);
    SetWindowMinSize(SCREEN_WIDTH, SCREEN_HEIGHT);

    RenderTexture2D target = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);

    Vector2 player = {200, GROUND_Y};
    Vector2 facing = {1,0};
    float speed = 0, velY = 0;
    bool grounded = true;
    int jumpsUsed = 0;
    float cameraX = 0;

    float transitionCenter = WORLD_WIDTH * 0.5f;
    float transitionWidth  = 600.0f;

    float sunX = SCREEN_WIDTH - 80;
    float sunStartY = 80, sunEndY = SCREEN_HEIGHT + 120, sunRadius = 220;

    float moonX = 80;
    float moonStartY = SCREEN_HEIGHT + 120, moonEndY = 100, moonRadius = 160;

    VirtualJoystick joy = {
            {120,SCREEN_HEIGHT-120},
            {120,SCREEN_HEIGHT-120},
            60,false,{0,0},-1
    };

    Vector2 jumpBtn = {SCREEN_WIDTH-120,SCREEN_HEIGHT-120};
    float jumpRadius = 40;
    int jumpFinger = -1;

    ChatState chat;
    Chat_Init(&chat);

    while (!WindowShouldClose())
    {
        float time = GetTime();
        speed = 0;
        float dt = GetFrameTime();
        Chat_Update(&chat, dt);

        int touches = GetTouchPointCount();
        for (int i = 0; i < touches; i++)
        {
            Vector2 p = TouchToGame(GetTouchPosition(i));

            // 1) Give chat FIRST chance to consume touch
            if (Chat_HandleTouch(&chat, p, i))
            {
                continue; // chat used it, skip gameplay input
            }

            // 2) Gameplay input ONLY if chat didn't take it
            if (joy.finger == -1 &&
                CheckCollisionPointCircle(p, joy.base, joy.radius))
            {
                joy.finger = i;
                joy.active = true;
            }

            if (i == joy.finger)
            {
                Vector2 d = Vector2Subtract(p, joy.base);
                if (Vector2Length(d) > joy.radius)
                    d = Vector2Scale(Vector2Normalize(d), joy.radius);

                joy.delta = Vector2Normalize(d);
                joy.knob = Vector2Add(joy.base, d);

                speed = fabsf(joy.delta.x);
                player.x += joy.delta.x * speed * 5.5f;
                facing.x = joy.delta.x >= 0 ? 1 : -1;
            }

            if (jumpFinger == -1 &&
                jumpsUsed < MAX_JUMPS &&
                CheckCollisionPointCircle(p, jumpBtn, jumpRadius))
            {
                jumpFinger = i;
                velY = JUMP_VELOCITY;
                grounded = false;
                jumpsUsed++;

#if defined(PLATFORM_ANDROID)
                TriggerHapticFeedback(30);
#endif
            }
        }

        if (joy.finger >= touches)
        {
            joy.finger = -1;
            joy.active = false;
            joy.knob = joy.base;
        }

        if (jumpFinger >= touches)
            jumpFinger = -1;

        velY += GRAVITY;
        player.y += velY;

        if (player.y >= GROUND_Y)
        {
            player.y = GROUND_Y;
            velY = 0;
            grounded = true;
            jumpsUsed = 0;
        }

        player.x = Clamp(player.x, 0, WORLD_WIDTH);
        cameraX = Clamp(player.x - SCREEN_WIDTH*0.4f, 0, WORLD_WIDTH-SCREEN_WIDTH);

        float t = Clamp(
                (player.x - (transitionCenter-transitionWidth*0.5f))/transitionWidth,
                0,1);

        float ambient = Lerp(DAY_AMBIENT, NIGHT_AMBIENT, t);

        UpdateBirds(time);

        BeginTextureMode(target);
        ClearBackground(SKYBLUE);

        DrawParallax(cameraX);
        DrawBirds(1.0f - t, time);

        DrawRectangle(-cameraX, GROUND_Y+24, WORLD_WIDTH, 200, DARKBROWN);
        DrawPlayer((Vector2){player.x-cameraX,player.y}, facing, speed, time);

        Chat_DrawBubble(&chat, player, cameraX);

        BeginBlendMode(BLEND_MULTIPLIED);
        DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,Fade(BLACK,ambient));
        EndBlendMode();

        DrawStars(t, time);

        BeginBlendMode(BLEND_ADDITIVE);
        DrawCircleGradient(sunX, Lerp(sunStartY,sunEndY,t),
                           sunRadius, Fade(YELLOW,1-t), Fade(BLACK,0));
        DrawCircleGradient(moonX, Lerp(moonStartY,moonEndY,t),
                           moonRadius, Fade(RAYWHITE,t), Fade(BLACK,0));
        EndBlendMode();

        float jumpScale = (jumpFinger != -1) ? 1.15f : 1.0f;
        float drawRadius = jumpRadius * jumpScale;

        DrawCircleV(
                jumpBtn,
                drawRadius,
                jumpsUsed < MAX_JUMPS ? Fade(GREEN,0.6f) : Fade(GRAY,0.4f)
        );

        DrawOutlinedText(
                "JUMP",
                jumpBtn.x - (int)(26 * jumpScale),
                jumpBtn.y - (int)(10 * jumpScale),
                (int)(20 * jumpScale),
                BLACK,
                RAYWHITE
        );

        float joyScale = joy.active ? 2.0f : 1.0f;

        DrawCircleV(
                joy.base,
                joy.radius * joyScale,
                Fade(DARKGRAY,0.5f)
        );

        DrawCircleV(
                joy.knob,
                25 * joyScale,
                GRAY
        );

        Chat_DrawButton(&chat);

        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);

        Rectangle src = {0,0,SCREEN_WIDTH,-SCREEN_HEIGHT};
        Rectangle dst = {0,0,GetScreenWidth(),GetScreenHeight()};
        DrawTexturePro(target.texture, src, dst, (Vector2){0,0}, 0, WHITE);

        EndDrawing();
    }

    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}
