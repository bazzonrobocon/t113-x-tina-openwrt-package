#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "aw_dlna.h"

int main(int argc, char** argv) {

    if (argc != 1)
    {
        printf ("**************************\n");
        printf ("usage: ./tplayer_cast_demo\n");
        printf ("**************************\n");
        return -1;
    }

    if(((access("/dev/zero",F_OK)) < 0)||((access("/dev/fb0",F_OK)) < 0))
    {
        printf("/dev/zero OR /dev/fb0 is not exit\n");
    }
    else
    {
        system("dd if=/dev/zero of=/dev/fb0");//clean the framebuffer
    }

    static char *device_name = "ZAirPlayer";
    AWDlnaT *awd = NULL;
    awd = AWD_Instance(device_name, NULL, NULL);
    AWD_Start(awd);

    while(1)
    {
        usleep(5000000);
    }

    return 0;
}
