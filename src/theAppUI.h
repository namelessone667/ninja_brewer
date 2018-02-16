#ifndef theAppUI_h
#define theAppUI_h

#include "Model.h"
#include "Encoder.h"
#include "Buttons.h"
#include "MENWIZ.h"
#include "LiquidCrystal_I2C.h"

#define PROGRESS_BAR_INTERVAL 200

class theApp;

class theAppUI
{
  public:
      theAppUI(theApp *controller);
      void init();
      void draw();
      //void setMenuConfigValues(AppConfig app_config);
      //AppConfig getMenuConfigValues();
      void enterMainMenu();
      void reinitLCD();
  private:
      void buildMenu();
      void saveAndExitMenu();
      int scanNavButtons();
      void discardChangesAndExitMenu();
      char getProgressbarSymbol();

      static void discardChangesAndExitMenuHelper();
      static void saveAndExitMenuHelper();
      static int scanNavButtonsHelper();

      static theAppUI *_helperInstance;

      theApp *_controller;
      Encoder _encoder;
      LiquidCrystal_I2C _lcd;
      AppConfig _tempConfig;
      Button _btn_left;
      Button _btn_right;
      menwiz _mainmenu;
      long _encoder_position;
      bool _menuActive;
      bool _reinitLCD;
};

#endif
