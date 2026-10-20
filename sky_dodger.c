// Sky Dodger: a tiny touch game written in C with raylib
// Drag your finger left and right to dodge the falling blocks.

#include "raylib.h"

#define MAX_BLOCKS 30

// ---------- Step 1: describe the things in our game ----------
typedef struct {
    Rectangle rect;   // position (x, y) and size (width, height)
    float speed;      // pixels per second
    bool active;      // is this block on screen?
} Block;

typedef enum { TITLE, PLAYING, GAME_OVER } GameState;

// ---------- Step 2: game data ----------
Rectangle player;
Block blocks[MAX_BLOCKS];
GameState state = TITLE;
float score = 0;        // seconds survived
float best = 0;         // best score this session
float spawnTimer = 0;   // counts down to the next block
bool wasTouching = false; // was the finger down last frame?

// ---------- Step 3: reset everything for a new round ----------
void ResetGame(int W, int H)
{
    player.width  = W / 7.0f;
    player.height = W / 7.0f;
    player.x = W / 2.0f - player.width / 2.0f;
    player.y = H - player.height * 2.0f;

    for (int i = 0; i < MAX_BLOCKS; i++) blocks[i].active = false;
    score = 0;
    spawnTimer = 0;
}

// ---------- Step 4: add a new falling block ----------
void SpawnBlock(int W)
{
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (!blocks[i].active) {
            float size = (float)GetRandomValue(W / 10, W / 5);
            blocks[i].rect = (Rectangle){ (float)GetRandomValue(0, W - (int)size), -size, size, size };
            blocks[i].speed = W * 0.6f + score * 15.0f;   // faster as you survive longer
            blocks[i].active = true;
            return;
        }
    }
}

int main(void)
{
    // ---------- Step 5: open the window (full screen on a phone) ----------
    InitWindow(450, 800, "Sky Dodger");
    SetTargetFPS(60);

    int W = GetScreenWidth();
    int H = GetScreenHeight();
    ResetGame(W, H);

    // ---------- Step 6: the game loop runs 60 times per second ----------
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();   // seconds since last frame

        // ===== INPUT: finger on a phone, mouse on a computer =====
        bool fingerOnPhone = GetTouchPointCount() > 0;
        bool touching = fingerOnPhone || IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        int fingerX = fingerOnPhone ? GetTouchX() : GetMouseX();
        bool tapped = touching && !wasTouching;   // finger just went down
        wasTouching = touching;

        // ===== UPDATE: change the game data =====
        if (state == TITLE || state == GAME_OVER) {
            if (tapped) {
                ResetGame(W, H);
                state = PLAYING;
            }
        }
        else if (state == PLAYING) {
            score += dt;

            // Move player to where the finger is
            if (touching) {
                player.x = fingerX - player.width / 2.0f;
            }
            if (player.x < 0) player.x = 0;
            if (player.x > W - player.width) player.x = W - player.width;

            // Spawn blocks more often as time goes on
            spawnTimer -= dt;
            if (spawnTimer <= 0) {
                SpawnBlock(W);
                float interval = 0.7f - score * 0.01f;
                spawnTimer = (interval < 0.25f) ? 0.25f : interval;
            }

            // Move blocks and check for hits
            for (int i = 0; i < MAX_BLOCKS; i++) {
                if (!blocks[i].active) continue;
                blocks[i].rect.y += blocks[i].speed * dt;
                if (blocks[i].rect.y > H) blocks[i].active = false;
                if (CheckCollisionRecs(player, blocks[i].rect)) {
                    state = GAME_OVER;
                    if (score > best) best = score;
                }
            }
        }

        // ===== DRAW: show the game data on screen =====
        BeginDrawing();
        ClearBackground((Color){ 20, 24, 40, 255 });

        int big = W / 9;     // text sizes scale with screen width
        int small = W / 18;

        if (state == TITLE) {
            DrawText("SKY DODGER", (W - MeasureText("SKY DODGER", big)) / 2, H / 3, big, RAYWHITE);
            DrawText("Tap to start", (W - MeasureText("Tap to start", small)) / 2, H / 2, small, LIGHTGRAY);
        }
        else {
            for (int i = 0; i < MAX_BLOCKS; i++)
                if (blocks[i].active) DrawRectangleRec(blocks[i].rect, ORANGE);

            DrawRectangleRec(player, SKYBLUE);
            DrawText(TextFormat("%.1f", score), 20, 40, big, RAYWHITE);

            if (state == GAME_OVER) {
                DrawText("GAME OVER", (W - MeasureText("GAME OVER", big)) / 2, H / 3, big, RED);
                const char *b = TextFormat("Best: %.1f", best);
                DrawText(b, (W - MeasureText(b, small)) / 2, H / 3 + big + 20, small, RAYWHITE);
                DrawText("Tap to play again", (W - MeasureText("Tap to play again", small)) / 2, H / 2, small, LIGHTGRAY);
            }
        }
        EndDrawing();
    }

    // ---------- Step 7: clean up ----------
    CloseWindow();
    return 0;
}
