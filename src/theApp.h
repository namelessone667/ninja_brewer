#include "application.h"
#include "buttons.h"
#include <Encoder.h>
#include "globals.h"
#include "MENWIZ.h"
#include "LiquidCrystal_I2C.h"
#include "probe.h"
#include "Model.h"

#ifndef theApp_h
#define theApp_h

class theApp
{
    public:
        theApp();
        void init();
        void run();
    private:
        Model _model;
        class theAppUI
        {
          public:
              static void init();
              static int scanNavButtons();
              static void draw();
              static void enterMainMenu();
              static void setMenuConfigValues(AppConfig app_config);
              static AppConfig getMenuConfigValues();
          private:
              static void buildMenu();
              static AppConfig _tempConfig;
              static Button _btn_left;
              static Button _btn_right;
              static Encoder _encoder;
              static LiquidCrystal_I2C _lcd;
              static menwiz _mainmenu;
              static long _encoder_position;
              static bool _menuActive;
        };
};

#endif
