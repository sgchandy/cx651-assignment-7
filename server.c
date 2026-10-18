#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "game.h"
#include "save.h"

#define PORT 8080

// builds the response describing game db for the given player
void fill_response(struct response* resp, struct gameDB* db, int player) {
    memset(resp, 0, sizeof(*resp));
    resp->ok = 1;
    resp->gameId = db->g.gameId;
    resp->player = player;
    resp->started = db->num_players == 2;
    resp->turn = db->g.turn;
    resp->winner = check_win(&db->g);
    memcpy(resp->board, db->g.board.cells, sizeof(resp->board));

    if (resp->winner == -1) {
        int full = 1;
        for (int i = 0; i < ROWS * COLS; i++)
            if (db->g.board.cells[i] == EMPTY)
                full = 0;
        if (full)
            resp->winner = TIE;
    }
}

// sends the current state of db to the given player
void send_state(int sock_fd, struct gameDB* db, int player) {
    struct response resp;
    fill_response(&resp, db, player);
    sendto(sock_fd, &resp, sizeof(resp), 0,
           (struct sockaddr *)&db->players[player], sizeof(db->players[player]));
}

int main(int argc, char* argv[]) {
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    printf("Message board server listening on port %d\n", PORT);

    struct gameDB* games = malloc(MAX_GAMES * sizeof(struct gameDB));
    int num_games = 0;

    while (1) {
        struct request req;
        socklen_t addr_len = sizeof(client_addr);
        if (recvfrom(sock_fd, &req, sizeof(req), 0,
                     (struct sockaddr *)&client_addr, &addr_len) != sizeof(req))
            continue;

        if (req.type == REQ_CREATE && num_games < MAX_GAMES) {
            struct gameDB* db = &games[num_games];
            new_game(&db->g, num_games);
            db->num_players = 1;
            db->players[0] = client_addr;
            num_games++;
            send_state(sock_fd, db, 0);
        } else if (req.type == REQ_JOIN && req.gameId >= 0 && req.gameId < num_games &&
                   games[req.gameId].num_players == 1) {
            struct gameDB* db = &games[req.gameId];
            db->num_players = 2;
            db->players[1] = client_addr;
            // tell both players the game has started
            send_state(sock_fd, db, 0);
            send_state(sock_fd, db, 1);
        } else if (req.type == REQ_MOVE && req.gameId >= 0 && req.gameId < num_games &&
                   (req.player == 0 || req.player == 1)) {
            struct gameDB* db = &games[req.gameId];
            if (db->num_players == 2 && check_win(&db->g) == -1 &&
                make_move(&db->g, req.player, req.col) != -1) {
                send_state(sock_fd, db, 0);
                send_state(sock_fd, db, 1);
            } else {
                // illegal move: resend the unchanged state so the player can retry
                send_state(sock_fd, db, req.player);
            }
        } else {
            struct response resp;
            memset(&resp, 0, sizeof(resp));
            sendto(sock_fd, &resp, sizeof(resp), 0,
                   (struct sockaddr *)&client_addr, addr_len);
        }
    }

    free(games);
    close(sock_fd);

    return 0;
}