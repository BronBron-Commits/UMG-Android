#include <android_native_app_glue.h>
#include "raylib.h"
#include "raymath.h"
#include <math.h>

/* =============================
   CONFIG (PORTRAIT)
============================= */
#define SCREEN_WIDTH   480
#define SCREEN_HEIGHT  800
#define WORLD_WIDTH    4000.0f
#define GROUND_Y       520.0f

#define GRAVITY        0.6f
#define JUMP_VELOCITY -12.0f

#define DAY_AMBIENT    0.40f
#define NIGHT_AMBIENT  0.75f

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

    while (!WindowShouldClose())
    {
        float time = GetTime();
        speed = 0;

        int touches = GetTouchPointCount();
        for (int i = 0; i < touches; i++)
        {
            Vector2 p = GetTouchPosition(i);

            // Joystick
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

            // Jump
            if (jumpFinger == -1 &&
                grounded &&
                CheckCollisionPointCircle(p, jumpBtn, jumpRadius))
            {
                jumpFinger = i;
                velY = JUMP_VELOCITY;
                grounded = false;
            }
        }

        // Release handling
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

        DrawCircleV(joy.base, joy.radius, Fade(DARKGRAY,0.5f));
        DrawCircleV(joy.knob, 25, GRAY);
        DrawCircleV(jumpBtn, jumpRadius,
                    grounded?Fade(GREEN,0.6f):Fade(GRAY,0.4f));
        DrawText("JUMP", jumpBtn.x-22, jumpBtn.y-8, 16, BLACK);

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
