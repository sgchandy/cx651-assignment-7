#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "common.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 4096


// prints the 7x6 flattened board array in the correct format
void print_board(int board[]) {
 
    printf("\n");
    for (int col = 0; col < 7; col++)
        printf(" %d", col);
    printf("\n");
 
    for (int row = 0; row < 6; row++) {
        printf("|");
        for (int col = 0; col < 7; col++) {
            int cell = board[row * 7 + col];
            printf("%c|", cell == 0 ? 'X' : cell == 1 ? 'O' : '.');
        }
        printf("\n");
    }
    printf("\n");
}

// prints the game header and board from a server response
void print_game(struct response* resp) {
    printf("\nCurrent Game\n");
    printf("---------------------\n");
    printf("GameID: %d\n", resp->gameId);
    print_board(resp->board);
}

// waits for the next message from the server
void recv_response(int sock_fd, struct response* resp) {
    while (recvfrom(sock_fd, resp, sizeof(*resp), 0, NULL, NULL) != sizeof(*resp))
        ;
}


int main() {
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);


    printf("Connect 4\n");
    printf("  1) Create a new game\n");
    printf("  2) Join a game\n");
    printf("> ");
    int choice = 0;
    scanf("%d", &choice);

    struct request req;
    memset(&req, 0, sizeof(req));
    if (choice == 1) {
        req.type = REQ_CREATE;
    } else if (choice == 2) {
        req.type = REQ_JOIN;
        printf("Game ID? >");
        if (scanf("%d", &req.gameId) != 1)
            return 1;
    } else {
        printf("Invalid choice\n");
        return 1;
    }

    struct response resp;
    sendto(sock_fd, &req, sizeof(req), 0,
           (struct sockaddr *)&server_addr, sizeof(server_addr));
    recv_response(sock_fd, &resp);
    if (!resp.ok) {
        printf("Could not join game %d\n", req.gameId);
        return 1;
    }
    print_game(&resp);

    int me = resp.player;
    req.type = REQ_MOVE;
    req.gameId = resp.gameId;
    req.player = me;

    while (1) {
        if (resp.winner == me) {
            printf("YOU WIN!\n");
            break;
        } else if (resp.winner == 1 - me) {
            printf("YOU LOSE!\n");
            break;
        } else if (resp.winner == TIE) {
            printf("TIE\n");
            break;
        }

        if (resp.started && resp.turn == me) {
            printf("\nInput Move Row: > ");
            if (scanf("%d", &req.col) != 1)
                break;
            sendto(sock_fd, &req, sizeof(req), 0,
                   (struct sockaddr *)&server_addr, sizeof(server_addr));
        } else {
            printf("waiting for other player...\n");
        }
        recv_response(sock_fd, &resp);
        print_game(&resp);
    }

    close(sock_fd);

    return 0;
}