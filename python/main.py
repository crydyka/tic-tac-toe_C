import pyray as rl

CELL_SIZE = 200
MAP_SIZE = 3
GLOBAL_PADDING = 20
GLOBAL_END = MAP_SIZE * CELL_SIZE + GLOBAL_PADDING
CELL_PADDING = 20
CELL_END = CELL_SIZE - CELL_PADDING

# Map Cell Values

MC_NONE = 0
MC_X = 1
MC_O = 2

# Game Result

GR_NONE = 0
GR_X = 1
GR_O = 2
GR_DRAW = 3

class TTT_Gamestate:
    def __init__(self, parent=None):
        if parent == None:
            self.clear()
            return

        self.map = [row[:] for row in parent.map]
        self.filled_cells = parent.filled_cells
        self.game_result = parent.game_result
        self.isNextX = parent.isNextX

    def clear(self) -> None:
        self.map = [[0 for j in range(3)] for i in range(3)]
        self.filled_cells = 0
        self.game_result = GR_NONE
        self.isNextX = True

    def move(self, x: int, y: int) -> int:
        def isVertical() -> bool:
            for i in range(1, MAP_SIZE):
                if self.map[0][x] != self.map[i][x]:
                    return False

            return True

        def isHorisontal() -> bool:
            for i in range(1, MAP_SIZE):
                if self.map[y][0] != self.map[y][i]:
                    return False

            return True

        def checkMainD() -> bool:
            first = self.map[0][0]

            if not first:
                return 0

            for i in range(1, MAP_SIZE):
                if self.map[i][i] != first:
                    return False

            return True

        def checkSlaveD() -> bool:
            first = self.map[MAP_SIZE - 1][0]

            if not first:
                return 0

            for i in range(1, MAP_SIZE):
                if self.map[MAP_SIZE - i - 1][i] != first:
                    return False

            return True

        def isDiagonals() -> bool:
            if x == y and checkMainD():
                return True

            return x == (MAP_SIZE - y - 1) and checkSlaveD()

        self.map[y][x] = MC_X if self.isNextX else MC_O

        if (
            isVertical() or
            isHorisontal() or
            isDiagonals()
        ):
            self.game_result = GR_X if self.isNextX else GR_O
            return self.game_result

        self.filled_cells += 1

        if self.filled_cells == 9:
            self.game_result = GR_DRAW
            return self.game_result

        self.isNextX = not self.isNextX
        return self.game_result

    def findBestMove(self) -> tuple[int, int]:
        out_x = 0
        out_y = 0

        def rec(gs: TTT_Gamestate, first: bool) -> int:
            nonlocal out_x, out_y

            ret_x = 0
            ret_y = 0
            ret = -1

            new_gs = TTT_Gamestate(gs)

            for y in range(MAP_SIZE):
                for x in range(MAP_SIZE):
                    if new_gs.map[y][x]:
                        continue

                    mov_ret = new_gs.move(x, y)

                    if mov_ret:
                        score = mov_ret != GR_DRAW
                    else:
                        score = -rec(new_gs, False)

                    if score == 1:
                        if first:
                            out_x = x
                            out_y = y

                        return 1

                    if score == 0 and ret != 0:
                        if first:
                            ret_x = x
                            ret_y = y

                        ret = 0

                    new_gs = TTT_Gamestate(gs)
            
            if first:
                out_x = ret_x
                out_y = ret_y

            return ret

        rec(self, True)
        return out_x, out_y

class TTT_Game:
    def __init__(self):
        self.texture_X = None
        self.texture_O = None
        self.gs = TTT_Gamestate()

    def update_screen(self) -> None:
        rl.begin_drawing()

        rl.clear_background(rl.BLACK)

        if self.gs.game_result == GR_NONE:
            fg_color = rl.Color(0xC1, 0xCE, 0xE3, 0xFF)
        elif self.gs.game_result == GR_X:
            fg_color = rl.Color(0x48, 0xD4, 0x28, 0xFF)
        elif self.gs.game_result == GR_O:
            fg_color = rl.Color(0xD4, 0x28, 0x28, 0xFF)
        else:
            fg_color = rl.Color(0xC9, 0xD4, 0x28, 0xFF)

        for y in range(MAP_SIZE):
            for x in range(MAP_SIZE):
                rl.draw_rectangle(
                    x * CELL_SIZE + GLOBAL_PADDING + CELL_PADDING,
                    y * CELL_SIZE + GLOBAL_PADDING + CELL_PADDING,
                    CELL_SIZE - 2 * CELL_PADDING,
                    CELL_SIZE - 2 * CELL_PADDING,
                    fg_color
                )

                if self.gs.map[y][x] == MC_X:
                    texture = self.texture_X
                elif self.gs.map[y][x] == MC_O:
                    texture = self.texture_O
                else:
                    continue

                rl.draw_texture(
                    texture,
                    x * CELL_SIZE + GLOBAL_PADDING + CELL_PADDING,
                    y * CELL_SIZE + GLOBAL_PADDING + CELL_PADDING,
                    rl.WHITE
                );

        rl.end_drawing()

    def update_game(self) -> None:
        if self.gs.game_result:
            if not rl.is_key_pressed(rl.KEY_ENTER):
                return

            self.gs.clear()

        if not rl.is_mouse_button_pressed(rl.MOUSE_BUTTON_LEFT):
            return

        mouse_x = rl.get_mouse_x()
        mouse_y = rl.get_mouse_y()

        if (
            mouse_x < GLOBAL_PADDING or mouse_x >= GLOBAL_END or
            mouse_y < GLOBAL_PADDING or mouse_y >= GLOBAL_END
        ):
            return

        inmap_x = mouse_x - GLOBAL_PADDING
        inmap_y = mouse_y - GLOBAL_PADDING

        incell_x = inmap_x % CELL_SIZE
        incell_y = inmap_y % CELL_SIZE

        if (
            incell_x < CELL_PADDING or incell_x >= CELL_END or
            incell_y < CELL_PADDING or incell_y >= CELL_END
        ):
            return

        x = inmap_x // CELL_SIZE
        y = inmap_y // CELL_SIZE

        if self.gs.map[y][x]:
            return

        if self.gs.move(x, y):
            return

        engine_x, engine_y = self.gs.findBestMove()
        self.gs.move(engine_x, engine_y)

    def mainloop(self) -> None:
        rl.init_window(
            MAP_SIZE * CELL_SIZE + 2 * GLOBAL_PADDING,
            MAP_SIZE * CELL_SIZE + 2 * GLOBAL_PADDING,
            "Tic Tac Toe"
        )

        rl.set_target_fps(60)

        self.texture_X = rl.load_texture("resources/TTT_X.png")
        self.texture_O = rl.load_texture("resources/TTT_O.png")

        while not rl.window_should_close():
            self.update_screen()
            self.update_game()

        rl.close_window()

def main():
    game = TTT_Game()
    game.mainloop()

if __name__ == "__main__":
    main()
