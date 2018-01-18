#include "application.h"
#include "buttons.h"
#include <Encoder.h>
#include "globals.h"
#include "MENWIZ.h"
#include "LiquidCrystal_I2C.h"
#include "probe.h"

#ifndef theApp_h
#define theApp_h

class theApp
{
    public:
        theApp();
        void init();
        void run();
    private:
        class theAppUI
        {
          public:
              static void init();
              static int scanNavButtons();
              static void draw();
          private:
              static Button _btn_left;
              static Button _btn_right;
              static Encoder _encoder;
              static LiquidCrystal_I2C _lcd;
              static menwiz _manimenu;
              static long _encoder_position;
              static bool _menuActive;
        };
};

#endif
