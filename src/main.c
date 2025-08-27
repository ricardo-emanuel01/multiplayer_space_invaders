#include "../lib/game.h"


int main(int argc, char *argv[]) {
    if (argc < 1) return -1;
    int ret = mainLoop(argv[1]);
    if (ret != 0) return ret;

    return 0;
}
