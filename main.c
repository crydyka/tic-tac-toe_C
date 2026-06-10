#include <raylib.h>
#include <stdint.h>
#include <string.h>

#define CELL_SIZE (200)
#define MAP_SIZE (3)
#define GLOBAL_PADDING (20)
#define GLOBAL_END (MAP_SIZE * CELL_SIZE + GLOBAL_PADDING)
#define CELL_PADDING (20)
#define CELL_END (CELL_SIZE - CELL_PADDING)

// Map Cell Values

enum {
    MC_NONE = 0,
    MC_X,
    MC_O
};

typedef struct {
    uint8_t map[MAP_SIZE][MAP_SIZE];
    int filled_cells;
    int game_result;
    bool isNextX;
} ttt_gamestate;

// Game Result

enum {
    GR_NONE = 0,
    GR_X,
    GR_O,
    GR_DRAW
};

static ttt_gamestate main_gs = {
    .map = {0},
    .filled_cells = 0,
    .game_result = GR_NONE,
    .isNextX = 1
};

static Texture2D texture_X;
static Texture2D texture_O;
static int infbm__x;
static int infbm__y;

static inline int inmove__isVertical(ttt_gamestate *gs, int x) {
    for (int i = 1; i < MAP_SIZE; i++)
        if (gs->map[0][x] != gs->map[i][x])
            return 0;

    return 1;
}

static inline int inmove__isHorisontal(ttt_gamestate *gs, int y) {
    for (int i = 1; i < MAP_SIZE; i++)
        if (gs->map[y][0] != gs->map[y][i])
            return 0;

    return 1;
}

static inline int inmove__checkMainD(ttt_gamestate *gs) {
    int first = gs->map[0][0];
    
    if (!first)
        return 0;

    for (int i = 1; i < MAP_SIZE; i++) {
        if (gs->map[i][i] != first)
            return 0;
    }

    return 1;
}

static inline int inmove__checkSlaveD(ttt_gamestate *gs) {
    int first = gs->map[MAP_SIZE - 1][0];
    
    if (!first)
        return 0;

    for (int i = 1; i < MAP_SIZE; i++) {
        if (gs->map[MAP_SIZE - i - 1][i] != first)
            return 0;
    }

    return 1;
}

static inline int inmove__isDiagonals(ttt_gamestate *gs, int x, int y) {
    if (x == y && inmove__checkMainD(gs))
        return 1;

    return x == (MAP_SIZE - y - 1) && inmove__checkSlaveD(gs);
}

static int move(ttt_gamestate *gs, int x, int y) {
    gs->map[y][x] = gs->isNextX ? MC_X : MC_O;

    if (
        inmove__isVertical(gs, x) || 
        inmove__isHorisontal(gs, y) ||
        inmove__isDiagonals(gs, x, y)
    ) {
        gs->game_result = gs->isNextX ? GR_X : GR_O;
        return gs->game_result;
    }

    if (++gs->filled_cells == 9) {
        gs->game_result = GR_DRAW;
        return gs->game_result;
    }

    gs->isNextX = !gs->isNextX;
    return gs->game_result;
}

static inline void infbm__reload_gs(ttt_gamestate *src, ttt_gamestate *dst) {
    memcpy(src, dst, sizeof(ttt_gamestate));
}

static int infbm__rec(ttt_gamestate *gs, bool first) {
    int ret_x = 0;
    int ret_y = 0;
    int ret = -1;

    ttt_gamestate new_gs;
    infbm__reload_gs(&new_gs, gs);

    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            if (new_gs.map[y][x])
                continue;

            int mov_ret = move(&new_gs, x, y);

            switch (
                mov_ret ?
                mov_ret != GR_DRAW :
                -infbm__rec(&new_gs, 0)
            ) {
            case 1:
                if (!first)
                    return 1;

                infbm__x = x;
                infbm__y = y;
                return 1;
            case 0:
                if (!ret)
                    break;

                if (first) {
                    ret_x = x;
                    ret_y = y;
                }

                ret = 0;
            case -1:
            default:
                break;
            }

            infbm__reload_gs(&new_gs, gs);
        }
    }

    if (first) {
        infbm__x = ret_x;
        infbm__y = ret_y;
    }

    return ret;
}

static void findBestMove(ttt_gamestate *gs, int *out_x, int *out_y) {
    infbm__rec(gs, 1);
    *out_x = infbm__x;
    *out_y = infbm__y;
}

static void update_screen() {
    BeginDrawing();

    ClearBackground(BLACK);

    Color fg_color;

    switch (main_gs.game_result) {
    case GR_NONE:
        fg_color = (Color){0xC1, 0xCE, 0xE3, 0xFF};
        break;
    case GR_X:
        fg_color = (Color){0x48, 0xD4, 0x28, 0xFF};
        break;
    case GR_O:
        fg_color = (Color){0xD4, 0x28, 0x28, 0xFF};
        break;
    case GR_DRAW:
        fg_color = (Color){0xC9, 0xD4, 0x28, 0xFF};
    default:
        break;
    }

    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            DrawRectangle(
                x * CELL_SIZE + GLOBAL_PADDING + CELL_PADDING,
                y * CELL_SIZE + GLOBAL_PADDING + CELL_PADDING,
                CELL_SIZE - 2 * CELL_PADDING,
                CELL_SIZE - 2 * CELL_PADDING,
                fg_color
            );

            Texture2D texture;

            switch (main_gs.map[y][x]) {
            case MC_X:
                texture = texture_X;
                break;
            case MC_O:
                texture = texture_O;
                break;
            default:
                continue;
            }

            DrawTexture(
                texture,
                x * CELL_SIZE + GLOBAL_PADDING + CELL_PADDING,
                y * CELL_SIZE + GLOBAL_PADDING + CELL_PADDING,
                WHITE
            );
        }
    }

    EndDrawing();
}

static void update_game() {
    if (main_gs.game_result) {
        if (!IsKeyPressed(KEY_ENTER))
            return;

        memset(main_gs.map, 0, sizeof(main_gs.map));
        main_gs.filled_cells = 0;
        main_gs.game_result = GR_NONE;
        main_gs.isNextX = 1;
    }

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        return;

    int mouse_x = GetMouseX();
    int mouse_y = GetMouseY();

    if (
        mouse_x < GLOBAL_PADDING || mouse_x >= GLOBAL_END ||
        mouse_y < GLOBAL_PADDING || mouse_y >= GLOBAL_END
    )
        return;

    int inmap_x = mouse_x - GLOBAL_PADDING;
    int inmap_y = mouse_y - GLOBAL_PADDING;

    int incell_x = inmap_x % CELL_SIZE;
    int incell_y = inmap_y % CELL_SIZE;

    if (
        incell_x < CELL_PADDING || incell_x >= CELL_END ||
        incell_y < CELL_PADDING || incell_y >= CELL_END
    )
        return;

    int x = inmap_x / CELL_SIZE;
    int y = inmap_y / CELL_SIZE;

    if (main_gs.map[y][x])
        return;

    if (move(&main_gs, x, y))
        return;

    int engine_x;
    int engine_y;

    findBestMove(&main_gs, &engine_x, &engine_y);
    move(&main_gs, engine_x, engine_y);
}

int main() {
    InitWindow(
        MAP_SIZE * CELL_SIZE + 2 * GLOBAL_PADDING,
        MAP_SIZE * CELL_SIZE + 2 * GLOBAL_PADDING,
        "Tic Tac Toe"
    );

    texture_X = LoadTexture("resources/TTT_X.png");
    texture_O = LoadTexture("resources/TTT_O.png");

    while (!WindowShouldClose()) {
        update_screen();
        update_game();
    }

    CloseWindow();
    return 0;
}
