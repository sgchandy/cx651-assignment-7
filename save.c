#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "common.h"


/*
Saves server game data to specified filepath.
Returns 0 on success, -1 on failure.
*/
int savegame(struct gameDB* game, char* filepath) {
    int fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        return -1;

    ssize_t n = write(fd, game, sizeof(*game));
    close(fd);
    return n == sizeof(*game) ? 0 : -1;
}

/*
Loads a file written by savegame into game.
Returns 0 on success, -1 on failure.
*/
int loadgame(struct gameDB* game, char* filepath) {
    int fd = open(filepath, O_RDONLY);
    if (fd < 0)
        return -1;

    ssize_t n = read(fd, game, sizeof(*game));
    close(fd);
    return n == sizeof(*game) ? 0 : -1;
}
