#include "application.h"
#include "buttons.h"
#include <Encoder.h>
#include "globals.h"
#include "MENWIZ.h"
#include "LiquidCrystal_I2C.h"

#ifndef theApp_h
#define theApp_h

extern probe* fridge;
extern probe* beer;
extern double Output, heatOutput;

class theApp
{
    public:
        static void init();
        static void run();
        static void drawUsrScreen(char *text);
        static void draw();
    private:
        static Button _btn_left;
        static Button _btn_right;
        static Encoder _encoder;
        static LiquidCrystal_I2C _lcd;
        static menwiz _menu;
        static long _encoder_position;
        static bool _menuActive = false;

        static int scanNavButtons();
};

#endif
