#include "theApp.h"

Encoder theApp::theAppUI::_encoder(ENCODER_PIN_A, ENCODER_PIN_B);
LiquidCrystal_I2C theApp::theAppUI::_lcd(0x38, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
bool theApp::theAppUI::_menuActive = false;
menwiz theApp::theAppUI::_manimenu;
Button theApp::theAppUI::_btn_left;
Button theApp::theAppUI::_btn_right;
long theApp::theAppUI::_encoder_position;

theApp::theApp()
{

}

void theApp::init()
{
  theApp::theAppUI::init();
}

void theApp::run()
{
  theApp::theAppUI::draw();
}

void theApp::theAppUI::init()
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

    // override menu navigation
    _manimenu.addUsrNav(scanNavButtons, 4);
    // init LCD
    _manimenu.begin(&_lcd,16,2);
    _manimenu.drawUsrScreen("Initializing...\n\n");

    // add menu root item
    _manimenu.addMenu(MW_ROOT,NULL,F("Settings"));
}

void theApp::theAppUI::draw()
{
  if(_menuActive)
  {
    _manimenu.draw();
  }
  else
  {
    _manimenu.drawUsrScreen("Running...\n\n");
    //construct the text
    //char lcd_text[34];
    //sprintf(lcd_text, "F:%4.1fC B:%4.1fC\nT:%4.1fC H:%2f%%%c%c\n", 18.1, 18.2, 19.0, 15.0, 'I', '-');
    //_manimenu.drawUsrScreen(lcd_text);
  }
}

//TODO Implement LCD output using MENWIZ library
//TODO create structures to store onewire device addresses
//TODO implement dynamic onewire device discovery and initialization
//TODO implement capability to add new onewire devices throug menu

int theApp::theAppUI::scanNavButtons()
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
