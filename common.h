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

// server-side record of one game and the clients playing it
struct gameDB {
    struct game g;
    int num_players;
    struct sockaddr_in players[2];
};

// client -> server message types
#define REQ_CREATE 1
#define REQ_JOIN   2
#define REQ_MOVE   3

struct request {
    int type;
    int gameId;
    int player;
    int col;
};

#define TIE 2  // value of response.winner when the board fills with no winner

// server -> client message
struct response {
    int ok;       // 0 if the request failed (e.g. joining a missing game)
    int gameId;
    int player;   // which player (0/1) the recipient is
    int started;  // 1 once both players have joined
    int turn;
    int winner;   // -1 none, 0/1 player, or TIE
    int board[ROWS * COLS];
};

#endif