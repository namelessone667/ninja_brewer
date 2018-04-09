#ifndef theAppUI_h
#define theAppUI_h

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
      void ExitMenu();
  private:
      void buildMenu();
      void saveAndExitMenu();
      int scanNavButtons();
      void discardChangesAndExitMenu();
      char getProgressbarSymbol();

      theApp *_controller;
      Encoder _encoder;
      LiquidCrystal_I2C _lcd;
      Button _btn_left;
      Button _btn_right;
      NinjaMenu _ninjaMenu;
      long _encoder_position;
      bool _menuActive;
      bool _reinitLCD;
};

class ExitMenuCommand : public INinjaCommand
{
public:
  ExitMenuCommand(theAppUI& ui) : m_ui(ui)
  {

  }

  void Execute()
  {
    m_ui.ExitMenu();
  }
private:
  theAppUI& m_ui;
};

#endif
