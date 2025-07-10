#ifndef __BACKPLAY_H
#define __BACKPLAY_H

enum BackplayModeType {
    BACKPLAY_MODE_LOOP = 0,
    BACKPLAY_MODE_ONCE,
    BACKPLAY_MODE_DIR,
};

void backplaydemo(char* InputUrl, int mode);
int BackDemoQuit(void);
#endif
