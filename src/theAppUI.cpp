#include "theAppUI.h"
#include "globals.h"
#include "theApp.h"

uint8_t backslash_char[8] = {
    0b00000,
    0b10000,
    0b01000,
    0b00100,
    0b00010,
    0b00001,
    0b00000,
    0b00000
};

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
    _lcd.createChar(2, backslash_char);
    _mainmenu.drawUsrScreen("Initializing...\n\n");

    theAppUI::buildMenu();
}

void theAppUI::buildMenu()
{
    _menu *r,*s1,*s2;

    // add menu root item
    r = _mainmenu.addMenu(MW_ROOT,NULL,F("Settings"));

    _mainmenu.addMenu(MW_VAR, r, F("Target temp"))->addVar(MW_AUTO_DOUBLE,&_tempConfig.setpoint,0,30,0.1,1);

    s1 = _mainmenu.addMenu(MW_SUBMENU, r, F("PID param"));
    _mainmenu.addMenu(MW_VAR,s1, F("PID Kp"))->addVar(MW_AUTO_DOUBLE,&_tempConfig.pid_Kp,0,10,0.5,1);
    _mainmenu.addMenu(MW_VAR,s1, F("PID Ki"))->addVar(MW_AUTO_DOUBLE,&_tempConfig.pid_Ki,0,0.01,0.00001,5);
    s2 = _mainmenu.addMenu(MW_VAR,s1, F("PID Mode"));
      s2->addVar(MW_LIST,&_tempConfig.pid_mode);
      s2->addItem(MW_LIST, F("MANUAL"));
      s2->addItem(MW_LIST, F("AUTO"));

    s1 = _mainmenu.addMenu(MW_SUBMENU, r, F("Heat PID param"));
    _mainmenu.addMenu(MW_VAR,s1, F("Heat PID Kp"))->addVar(MW_AUTO_DOUBLE,&_tempConfig.heatpid_Kd,0,10,0.5,1);
    _mainmenu.addMenu(MW_VAR,s1, F("Heat PID Ki"))->addVar(MW_AUTO_DOUBLE,&_tempConfig.heatpid_Ki,0,0.1,0.0001,4);
    s2=_mainmenu.addMenu(MW_VAR,s1, F("Heat PID Mode"));
      s2->addVar(MW_LIST,&_tempConfig.heatpid_mode);
      s2->addItem(MW_LIST, F("MANUAL"));
      s2->addItem(MW_LIST, F("AUTO"));
    _mainmenu.addMenu(MW_VAR,s1, F("Manual Output"))->addVar(MW_AUTO_DOUBLE,&_tempConfig.heatOutput,0,25,0.5,1);

    s1 = _mainmenu.addMenu(MW_SUBMENU, r, F("Controller param"));
    _mainmenu.addMenu(MW_VAR,s1, F("Idle Diff"))->addVar(MW_AUTO_DOUBLE,&_tempConfig.idleDiff,0,5,0.1,1);
    _mainmenu.addMenu(MW_VAR,s1, F("Peak Diff"))->addVar(MW_AUTO_DOUBLE,&_tempConfig.peakDiff,0,1,0.01,2);
    _mainmenu.addMenu(MW_VAR,s1, F("Cool Min ON"))->addVar(MW_AUTO_INT,&_tempConfig.coolMinOn,120,600,10);
    _mainmenu.addMenu(MW_VAR,s1, F("Cool Min OFF"))->addVar(MW_AUTO_INT,&_tempConfig.coolMinOff,120,600,10);
    _mainmenu.addMenu(MW_VAR,s1, F("Cool Max ON"))->addVar(MW_AUTO_INT,&_tempConfig.coolMaxOn,600,7200,60);
    _mainmenu.addMenu(MW_VAR,s1, F("Peak Max Time"))->addVar(MW_AUTO_INT,&_tempConfig.peakMaxTime,60,3600,60);
    _mainmenu.addMenu(MW_VAR,s1, F("Peak Max Wait"))->addVar(MW_AUTO_INT,&_tempConfig.peakMaxWait,60,3600,60);
    _mainmenu.addMenu(MW_VAR,s1, F("Heat Min OFF"))->addVar(MW_AUTO_INT,&_tempConfig.heatMinOff,0,3600,10);
    _mainmenu.addMenu(MW_VAR,s1, F("Heat Window"))->addVar(MW_AUTO_INT,&_tempConfig.heatWindow,5,600,1);
    _mainmenu.addMenu(MW_VAR,s1, F("Idle Min ON"))->addVar(MW_AUTO_INT,&_tempConfig.minIdleTime,0,3600,10);
    _mainmenu.addMenu(MW_VAR,s1, F("No Heat Below"))->addVar(MW_AUTO_DOUBLE,&_tempConfig.no_heat_below,-10,50,1,1);
    _mainmenu.addMenu(MW_VAR,s1, F("No Cool Above"))->addVar(MW_AUTO_DOUBLE,&_tempConfig.no_cool_above,-10,50,1,1);
    s2 = _mainmenu.addMenu(MW_VAR,s1, F("Mode"));
      s2->addVar(MW_LIST,(int*)(&_tempConfig.controller_mode));
      s2->addItem(MW_LIST, F("COOL\\HEAT"));
      s2->addItem(MW_LIST, F("COOL ONLY"));
      s2->addItem(MW_LIST, F("HEAT ONLY"));
    _mainmenu.addMenu(MW_VAR,s1, F("Stand By"))->addVar(MW_BOOLEAN,&_tempConfig.standBy);

    _mainmenu.addMenu(MW_VAR, r, F("Save & Exit"))->addVar(MW_ACTION,saveAndExitMenuHelper);
    _mainmenu.addMenu(MW_VAR, r, F("Discard changes"))->addVar(MW_ACTION,discardChangesAndExitMenuHelper);

    // override menu navigation
    _mainmenu.addUsrNav(scanNavButtonsHelper, 4);
}

void theAppUI::draw()
{
  char lcd_text[34];

  switch(_controller->getModel()._appState.app_state)
  {
    case INIT:
      sprintf(lcd_text, "%s\n\n", "Initializing...");
      _mainmenu.drawUsrScreen(lcd_text);
      break;
    case RUNNING:
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

        String text = String::format("F:%4.1fC B:%4.1fC", _controller->getModel()._appState.fridgeTemp, _controller->getModel()._appState.beerTemp)
                        .substring(0,16);
        text.concat("\n");
        String text2 = String::format("T:%4.1fC", _controller->getModel()._appConfig.setpoint).substring(0, 16);
        while(text2.length() < 14)
          text2.concat(' ');

        switch(_controller->getModel()._appState.controller_state)
        {
          case IDLE:
            text2.concat('I');
            break;
          case COOL:
            text2.concat('C');
            break;
          case HEAT:
            text2.concat('H');
            break;
          default:
            text2.concat('X');
            break;
        }
        text2.concat(getProgressbarSymbol());
        //text2.setCharAt(10, getProgressbarSymbol());
        text.concat(text2);
        text.concat("\n");
        text.toCharArray(lcd_text, 34);

        _mainmenu.drawUsrScreen(lcd_text);
      }
      break;
    case UNDEFINED:
      sprintf(lcd_text, "%s\n%s\n", "ERROR", "UNDEFINED STATE");
      _mainmenu.drawUsrScreen(lcd_text);
      break;
    case IN_ERROR:
      char err[16];
      _controller->getErrorMessage().toCharArray(err, 16);
      sprintf(lcd_text, "%s\n%s\n", "ERROR", err);
      _mainmenu.drawUsrScreen(lcd_text);
      break;
  }

}

void theAppUI::discardChangesAndExitMenuHelper()
{
  _helperInstance->discardChangesAndExitMenu();
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
    _tempConfig = _controller->getModel()._appConfig;
    _mainmenu.cur_menu = _mainmenu.root;
    _mainmenu.cur_menu->cur_item = 0;
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

char theAppUI::getProgressbarSymbol()
{
  int progress = (millis() / PROGRESS_BAR_INTERVAL) % 4;

  switch(progress)
  {
    case 0:
      return 45;
      break;
    case 1:
      return 2;
      break;
    case 2:
      return 124;
      break;
    case 3:
      return 47;
      break;
    default:
      return 47;
      break;
  }
}
