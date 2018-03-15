#ifndef theAppUI_h
#define theAppUI_h

#include "Model.h"
#include "Encoder.h"
#include "Buttons.h"
#include "MENWIZ.h"
#include "LiquidCrystal_I2C.h"
#include "NinjaMenu.h"

#define PROGRESS_BAR_INTERVAL 200

class theApp;

class theAppUI : public INinjaMenuNavigationHandler
{
  public:
      theAppUI(theApp *controller);
      void init();
      void draw();
      void enterMainMenu();
      void reinitLCD();
      NinjaMenuNavigation ScanNavigationButtons();
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
      NinjaMenu _ninjaMenu;
      long _encoder_position;
      bool _menuActive;
      bool _reinitLCD;
};

#endif
