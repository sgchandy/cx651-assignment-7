#include <arpa/inet.h>
#include <sys/socket.h>


#ifndef COMMON_H
#define COMMON_H

// Define any structs you may find helpful here.

#define ROWS 6
#define COLS 7
#define EMPTY -1

// flattened 7x6 board, row 0 is the top; cells hold 0/1 for a player or EMPTY
struct board {
    int cells[ROWS * COLS];
};

struct game {
    int gameId;
    int turn;   // player (0/1) whose turn it is
    struct board board;
};

#endif