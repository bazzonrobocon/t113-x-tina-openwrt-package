#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "backplaydemo.h"

int main(int argc, char**argv )
{
    int mode = 0; /*defaultly loop mode*/
    if (argc == 3) {
        mode = atoi(argv[2]);
    }
    if (argc == 2 || argc == 3) {
        printf("start the background player\n");
        char inputurl[512];
        memset(inputurl, 0, 512);
        strncpy(inputurl, argv[1], 512);
        if (access(inputurl, 0)) {
            printf("[backplaydemo]------media file for backplaydemo isn't exist\n");
            return -1;
        }
        backplaydemo(inputurl, mode);
    }
    return 0;
}
