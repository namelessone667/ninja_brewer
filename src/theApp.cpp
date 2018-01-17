#include "theApp.h"


menwiz theApp::_manimenu;
Encoder theApp::_encoder(ENCODER_PIN_A, ENCODER_PIN_B);
LiquidCrystal_I2C theApp::_lcd(0x38, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
bool theApp::_menuActive = false;


void theApp::init()
{
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

    // add menu root item
    _manimenu.addMenu(MW_ROOT,NULL,F("Settings"));
}

void theApp::run()
{

}

void theApp::draw()
{
  if(_menuActive)
  {
    _manimenu.draw();
  }
  else
  {
    //get beer and fridge temperatures
    //get control status (controlled fermentation true/false)
    //get fridge state (IDLE, COOL, HEAT)
    //get progress indicator char
    //construct the text
    char fridge_state_char = 'X';

    /*byte fridge_state = getFridgeState(0);

    if(fridge_state == COOL)
      fridge_state_char = 'C';
    else if(fridge_state == IDLE)
      fridge_state_char = 'I';
    else if(fridge_state == HEAT)
      fridge_state_char = 'H';
    else
      fridge_state_char = 'E';*/

    char lcd_text[34];
    sprintf(lcd_text, "F:%4.1fC B:%4.1fC\nT:%4.1fC H:%2f%%%c%c\n", fridge->getFilter(), beer->getFilter(), Output, heatOutput, fridge_state_char, '-');
    _manimenu.drawUsrScreen(lcd_text);
  }
}

//DEPRECATED
void theApp::drawUsrScreen(char *text)
{
    _manimenu.drawUsrScreen(text);
}

//TODO Implement LCD output using MENWIZ library
//TODO create structures to store onewire device addresses
//TODO implement dynamic onewire device discovery and initialization
//TODO implement capability to add new onewire devices throug menu

int theApp::scanNavButtons()
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
