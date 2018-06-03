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

theAppUI::theAppUI(theApp *controller) :
  _encoder(ENCODER_PIN_A, ENCODER_PIN_B),
  _lcd(0x38, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE),
  _ninjaMenu(&_lcd, 16,2, this),
  _menuActive(false),
  _reinitLCD(false)
{
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
    _ninjaMenu.Begin();
    _lcd.createChar(2, backslash_char);
    _ninjaMenu.DrawUsrScreen("Initializing...\n\n");

    theAppUI::buildMenu();
}

void theAppUI::buildMenu()
{

    /* NINJA MENU */
    SubNinjaMenuItem* rootMenuItem = new SubNinjaMenuItem(F("Settings"));

    rootMenuItem->AddSubMenu(new BindedPropertyNinjaMenuItem<double>(F("Target temp"), _controller->getModel().SetPoint, 0,30,0.1,1 ));
    rootMenuItem->AddSubMenu(new BindedPropertyNinjaMenuItem<bool>(F("Stand By"), _controller->getModel().StandBy ));

    rootMenuItem->AddSubMenu((new SubNinjaMenuItem(F("PID param")))
      ->AddSubMenu(new BindedPropertyNinjaMenuItem<double>(F("PID Kp"), _controller->getModel().PID_Kp, 0, 10, 0.1, 1))
      ->AddSubMenu(new BindedPropertyNinjaMenuItem<double>(F("PID Ki"), _controller->getModel().PID_Ki, 0,0.01,0.00001,5))
      ->AddSubMenu((new OptionsPropertyNinjaMenuItem<int>(F("PID Mode"), _controller->getModel().PIDMode, PID_MANUAL))
        ->AddOption(PID_MANUAL, "MANUAL")
        ->AddOption(PID_AUTOMATIC, "AUTOMATIC")
      )
    );
    rootMenuItem->AddSubMenu((new SubNinjaMenuItem(F("Heat PID param")))
      ->AddSubMenu(new BindedPropertyNinjaMenuItem<double>(F("Heat PID Kp"), _controller->getModel().HeatPID_Kp, 0, 10, 0.1, 1))
      ->AddSubMenu(new BindedPropertyNinjaMenuItem<double>(F("Heat PID Ki"), _controller->getModel().HeatPID_Ki, 0,0.1,0.0001,4))
      ->AddSubMenu(new BindedPropertyNinjaMenuItem<double>(F("Manual Output"), _controller->getModel().HeatOutput, 0,25,0.5,1))
      ->AddSubMenu((new OptionsPropertyNinjaMenuItem<int>(F("Heat PID Mode"), _controller->getModel().HeatPIDMode, PID_MANUAL))
        ->AddOption(PID_MANUAL, "MANUAL")
        ->AddOption(PID_AUTOMATIC, "AUTOMATIC")
      )
    );
    rootMenuItem->AddSubMenu((new SubNinjaMenuItem(F("Controller param")))
      ->AddSubMenu(new BindedPropertyNinjaMenuItem<double>(F("Idle Diff"), _controller->getModel().IdleDiff, 0,5,0.1,1))
      ->AddSubMenu(new BindedPropertyNinjaMenuItem<double>(F("Peak Diff"), _controller->getModel().PeakDiff, 0,1,0.01,2))
      ->AddSubMenu(new BindedPropertyNinjaMenuItem<int>(F("Cool Min ON"), _controller->getModel().CoolMinOn, 120,600,10,0))
      ->AddSubMenu(new BindedPropertyNinjaMenuItem<int>(F("Cool Min OFF"), _controller->getModel().CoolMinOff, 120,600,10,0))
      ->AddSubMenu(new BindedPropertyNinjaMenuItem<int>(F("Cool Max ON"), _controller->getModel().CoolMaxOn, 600,7200,60,0))
      ->AddSubMenu(new BindedPropertyNinjaMenuItem<int>(F("Peak Max Time"), _controller->getModel().PeakMaxTime, 60,3600,60,0))
      ->AddSubMenu(new BindedPropertyNinjaMenuItem<int>(F("Peak Max Wait"), _controller->getModel().PeakMaxWait, 60,3600,60,0))
      ->AddSubMenu(new BindedPropertyNinjaMenuItem<int>(F("Heat Min OFF"), _controller->getModel().HeatMinOff, 0,3600,10,0))
      ->AddSubMenu(new BindedPropertyNinjaMenuItem<int>(F("Heat Window"), _controller->getModel().HeatWindow, 5,600,1,0))
      ->AddSubMenu(new BindedPropertyNinjaMenuItem<int>(F("Idle Min ON"), _controller->getModel().MinIdleTime, 0,3600,10,0))
      ->AddSubMenu(new BindedPropertyNinjaMenuItem<double>(F("No Heat Below"), _controller->getModel().NoHeatBelow, -10,50,1,1))
      ->AddSubMenu(new BindedPropertyNinjaMenuItem<double>(F("No Cool Above"), _controller->getModel().NoCoolAbove, -10,50,1,1))
      ->AddSubMenu((new OptionsPropertyNinjaMenuItem<opMode>(F("Mode"), _controller->getModel().ControllerMode, COOLER_HEATER))
        ->AddOption(COOLER_HEATER, "COOL/HEAT")
        ->AddOption(COOLER_ONLY, "COOL ONLY")
        ->AddOption(HEATER_ONLY, "HEAT ONLY")
      )
    );

    rootMenuItem->AddSubMenu(new CommandNinjaMenuItem<NinjaCommandChain>(F("Save changes"), *((new NinjaCommandChain())->AddCommand(new SaveChangesNinjaMenuCommand(_ninjaMenu))->AddCommand(new ExitMenuCommand(*this)))));
    rootMenuItem->AddSubMenu(new CommandNinjaMenuItem<NinjaCommandChain>(F("Discard changes"), *((new NinjaCommandChain())->AddCommand(new DiscardChangesNinjaMenuCommand(_ninjaMenu))->AddCommand(new ExitMenuCommand(*this)))));
    _ninjaMenu.SetRootMenuItem(rootMenuItem);
}

void theAppUI::draw()
{
  if(_reinitLCD)
  {
    _lcd.begin(16,2);
    _reinitLCD = false;
  }

  switch(_controller->getModel().AppState)
  {
    case INIT:
      _ninjaMenu.DrawUsrScreen("Initializing...\n");
      break;
    case RUNNING:
      if(_menuActive)
      {
        _ninjaMenu.DrawMenu();
      }
      else
      {
        // check if button was pressed, if yes -> enter menu
        if(_btn_left.check()==ON)
        {
            enterMainMenu();
            return;
        }

        String text = String::format("F:%4.1fC B:%4.1fC", _controller->getModel().FridgeTemp.Get(), _controller->getModel().BeerTemp.Get())
                        .substring(0,16);
        text.concat("\n");
        String text2 = String::format("T:%4.1fC", _controller->getModel().SetPoint.Get()).substring(0, 16);
        while(text2.length() < 14)
          text2.concat(' ');

        switch(_controller->getModel().ControllerState)
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
        text.concat(text2);
        text.concat("\n");

        _ninjaMenu.DrawUsrScreen(text);
      }
      break;
    case UNDEFINED:
      _ninjaMenu.DrawUsrScreen("ERROR\nUNDEFINED STATE\n");
      break;
    case IN_ERROR:
      _ninjaMenu.DrawUsrScreen("ERROR\n" +  _controller->getErrorMessage() + "\n");
      break;
  }

}

void theAppUI::enterMainMenu()
{
    _menuActive = true;
    _encoder_position = _encoder.read();
    _btn_left.check();
    _btn_right.check();

    _ninjaMenu.Reset();
    _ninjaMenu.DrawMenu();
}

void theAppUI::ExitMenu()
{
  _menuActive = false;
}

NinjaMenuNavigation theAppUI::ScanNavigationButtons()
{
  long newPosition = _encoder.read();

   if(newPosition > _encoder_position+3)
   {
      _encoder_position += 4;
      return NINJAMENU_LEFT;
   }
   if(newPosition < _encoder_position-3)
   {
      _encoder_position -= 4;
      return NINJAMENU_RIGHT;
   }
   if(_btn_left.check()==ON)
   {
      return NINJAMENU_ENTER;
   }
   if(_btn_right.check()==ON)
   {
      return NINJAMENU_BACK;
   }

   return NINJAMENU_NONE;
}

void theAppUI::saveAndExitMenu()
{
  _ninjaMenu.SaveChanges();
  _controller->saveState();
  _menuActive = false;
}

void theAppUI::discardChangesAndExitMenu()
{
  _ninjaMenu.DiscardChanges();
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

void theAppUI::reinitLCD()
{
  _reinitLCD = true;
}
