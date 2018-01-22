#include "theAppUI.h"
#include "globals.h"
#include "theApp.h"

theAppUI *theAppUI::_helperInstance;

theAppUI::theAppUI(theApp *controller) :
  _encoder(ENCODER_PIN_A, ENCODER_PIN_B),
  _lcd(0x38, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE),
  _tempConfig(defaultAppConfig),
  _menuActive(false)
{
  _helperInstance = this;
  _controller = controller;
}

void theAppUI::init()
{
    // 1. init GUI (LCD, buttons, encoder, menu)
    // 2. load config from EEPROM
    // 3. init sensors, PIDs, relays
    // 4. init publishers, connect to cloud

    // initialize buttons
    _btn_left.assign(LEFT_BUTTON_PIN);
    _btn_left.setMode(OneShot);
    _btn_left.check();

    _btn_right.assign(RIGHT_BUTTON_PIN);
    _btn_right.setMode(OneShot);
    _btn_right.check();

    // initialize encoder
    _encoder_position = _encoder.read();

    // init LCD
    _mainmenu.begin(&_lcd,16,2);
    _mainmenu.drawUsrScreen("Initializing...\n\n");

    theAppUI::buildMenu();
}

void theAppUI::buildMenu()
{
    _menu *r,*s1,*s2;

    // add menu root item
    r = _mainmenu.addMenu(MW_ROOT,NULL,F("Settings"));

    _mainmenu.addMenu(MW_VAR, r, F("Target temp"))->addVar(MW_AUTO_DOUBLE,&_tempConfig.setpoint,0,30,0.1);

    s1 = _mainmenu.addMenu(MW_SUBMENU, r, F("PID param"));
    _mainmenu.addMenu(MW_VAR,s1, F("PID Kp"))->addVar(MW_AUTO_DOUBLE,&_tempConfig.pid_Kp,0,10,0.5);
    _mainmenu.addMenu(MW_VAR,s1, F("PID Ki"))->addVar(MW_AUTO_DOUBLE,&_tempConfig.pid_Ki,0,0.01,0.00001);
    s2 = _mainmenu.addMenu(MW_VAR,s1, F("PID Mode"));
      s2->addVar(MW_LIST,&_tempConfig.pid_mode);
      s2->addItem(MW_LIST, F("MANUAL"));
      s2->addItem(MW_LIST, F("AUTO"));

    s1 = _mainmenu.addMenu(MW_SUBMENU, r, F("Heat PID param"));
    _mainmenu.addMenu(MW_VAR,s1, F("Heat PID Kp"))->addVar(MW_AUTO_DOUBLE,&_tempConfig.heatpid_Kd,0,10,0.5);
    _mainmenu.addMenu(MW_VAR,s1, F("Heat PID Ki"))->addVar(MW_AUTO_DOUBLE,&_tempConfig.heatpid_Ki,0,0.1,0.0001);
    s2=_mainmenu.addMenu(MW_VAR,s1, F("Heat PID Mode"));
      s2->addVar(MW_LIST,&_tempConfig.heatpid_mode);
      s2->addItem(MW_LIST, F("MANUAL"));
      s2->addItem(MW_LIST, F("AUTO"));
    _mainmenu.addMenu(MW_VAR,s1, F("Manual Output"))->addVar(MW_AUTO_DOUBLE,&_tempConfig.heatOutput,0,25,0.5);

    _mainmenu.addMenu(MW_VAR, r, F("Save & Exit"))->addVar(MW_ACTION,saveAndExitMenuHelper);
    _mainmenu.addMenu(MW_VAR, r, F("Discard changes"))->addVar(MW_ACTION,discardChangesAndExitMenuHelper);

    // override menu navigation
    _mainmenu.addUsrNav(scanNavButtonsHelper, 4);
}

void theAppUI::draw()
{
  if(_menuActive)
  {
    _mainmenu.draw();
  }
  else
  {
    // check if button was pressed, if yes -> enter menu
    if(_btn_left.check()==ON)
    {
        enterMainMenu();
        return;
    }

    //_mainmenu.drawUsrScreen("Running...\n\n");
    AppState appState = _controller->getAppStateValues();
    AppConfig appConfig = _controller->getAppConfigValues();
    //construct the text
    char lcd_text[34];
    sprintf(lcd_text, "F:%4.1fC B:%4.1fC\nT:%4.1fC H:%2f%%%c%c\n", appState.fridgeTemp, appState.beerTemp, appConfig.setpoint, 15.0, 'I', '-');
    _mainmenu.drawUsrScreen(lcd_text);
  }
}

void theAppUI::discardChangesAndExitMenuHelper()
{
  _helperInstance->discardChangesAndExitMenuHelper();
}

void theAppUI::saveAndExitMenuHelper()
{
  _helperInstance->saveAndExitMenu();
}

int theAppUI::scanNavButtonsHelper()
{
  return _helperInstance->scanNavButtons();
}

void theAppUI::enterMainMenu()
{
    _menuActive = true;
    _encoder_position = _encoder.read();
    _btn_left.check();
    _btn_right.check();
    _tempConfig = _controller->getAppConfigValues();
    _mainmenu.cur_menu = _mainmenu.root;
    _mainmenu.draw();
}

int theAppUI::scanNavButtons()
{

    long newPosition = _encoder.read();

     if(newPosition > _encoder_position+3)
     {
        _encoder_position += 4;
        return MW_BTD;
     }
     if(newPosition < _encoder_position-3)
     {
        _encoder_position -= 4;
        return MW_BTU;
     }
     if(_btn_left.check()==ON)
     {
        return MW_BTE;
     }
     if(_btn_right.check()==ON)
     {
        return MW_BTC;
     }

     return MW_BTNULL;
}

void theAppUI::saveAndExitMenu()
{
  _controller->setNewAppConfigValues(_tempConfig);
  _menuActive = false;
}

void theAppUI::discardChangesAndExitMenu()
{
  _menuActive = false;
}
