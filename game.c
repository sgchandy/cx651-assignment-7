#include "game.h"
#include <stdlib.h>
#include <stddef.h>

/*
Takes as input a pointer to a connect4 game board, a player (0/1), and a int column.
Places a new piece in `col` on the game board.
Returns the row the piece landed in, or -1 if the move is illegal.
*/
int make_move(struct game* g, int player, int col) {
    if (col < 0 || col >= COLS || player != g->turn)
        return -1;

    // drop the piece to the lowest empty row in the column
    for (int row = ROWS - 1; row >= 0; row--) {
        if (g->board.cells[row * COLS + col] == EMPTY) {
            g->board.cells[row * COLS + col] = player;
            g->turn = 1 - player;
            return row;
        }
    }
    return -1;  // column is full
}

/*
Takes as input a pointer to a connect4 game board.
Returns 0 or 1 if player 0 or 1 has won respectively.
Returns -1 if there is no winner.
*/
int check_win(struct game* g) {
    // directions: horizontal, vertical, diagonal down-right, diagonal down-left
    int dr[] = {0, 1, 1, 1};
    int dc[] = {1, 0, 1, -1};

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            int p = g->board.cells[row * COLS + col];
            if (p == EMPTY)
                continue;

            for (int d = 0; d < 4; d++) {
                int k = 1;
                while (k < 4) {
                    int r = row + dr[d] * k;
                    int c = col + dc[d] * k;
                    if (r < 0 || r >= ROWS || c < 0 || c >= COLS ||
                        g->board.cells[r * COLS + c] != p)
                        break;
                    k++;
                }
                if (k == 4)
                    return p;
            }
        }
    }
    return -1;
}

/*
Initializes an existing game with an empty board.
*/
void new_game(struct game* g, int gameId) {
    g->gameId = gameId;
    g->turn = 0;
    for (int i = 0; i < ROWS * COLS; i++)
        g->board.cells[i] = EMPTY;
}


