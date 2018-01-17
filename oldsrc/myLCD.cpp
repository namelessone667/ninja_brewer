#include "myLCD.h"

//If you use the I2C LCD Shield by catalex, you should change 0x27 to 0x38 or 0x20
//LiquidCrystal_I2C lcd(0x38, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
LiquidCrystal_I2C lcd(0x38, 16, 2);
// Addr, En, Rw, Rs, d4, d5, d6, d7, backlighpin, polarity

int progress = 0;

uint8_t backslash[8] = {
    0b00000,
    0b10000,
    0b01000,
    0b00100,
    0b00010,
    0b00001,
    0b00000,
    0b00000
};

void initLCD()
{
  //lcd.init();
  lcd.begin(16,2);
  lcd.backlight();
  lcd.createChar(0, backslash);
}

void printTemperature(float tempC, int probe_num)
{
  lcd.setCursor(0 + probe_num * 8, 0);
  if(probe_num == 0)
    lcd.print("F");
  else
    lcd.print("B");

  lcd.print(":");

  if (tempC == -127.00)
  {
    lcd.print("ERROR");
  }
  else
  {
    tempC = round(tempC*10.0)/10.0;
    lcd.print(tempC);
    lcd.setCursor(0 + probe_num * 8 + 6, 0);
    lcd.print("C ");
  }
}

void printPIDOutputPercent(int percent)
{
  lcd.setCursor(8, 1);
  lcd.print("H:");
  lcd.print(percent);
  lcd.print("%");
  //if(percent < 100)
  //  lcd.print(" ");
  if(percent < 10)
    lcd.print(" ");
}

void printPIDOutputTemp(double temp)
{
  temp = round(temp*10.0)/10.0;
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(temp);
  lcd.setCursor(6, 1);
  lcd.print("C ");
}

void progressIndicator()
{
  char progressChar;

  if(progress == 4)
    progress = 0;

  switch(progress++)
  {
    case 0:
      progressChar = 45;
      break;
    case 1:
      progressChar = 0;
      break;
    case 2:
      progressChar = 124;
      break;
    case 3:
      progressChar = 47;
      break;
  }

  lcd.setCursor(14, 1);
  lcd.print(progressChar);

}

void printFridgeState(byte state)
{
  lcd.setCursor(15, 1);
  if(state == COOL)
    lcd.print("C");
  else if(state == IDLE)
    lcd.print("I");
  else if(state == HEAT)
    lcd.print("H");
  else
    lcd.print("E");
}

void printLineToLCD(int rownum, const char* message)
{
  lcd.setCursor(0, rownum);
  lcd.print("                ");
  lcd.setCursor(0, rownum);
  lcd.print(message);
}

void printLineToLCD(int rownum, int columnum, const char* message)
{
  lcd.setCursor(columnum, rownum);
  //lcd.print("                ");
  //lcd.setCursor(0, rownum);
  lcd.print(message);
}

void printDebugLetter(const char* debug)
{
    lcd.setCursor(13, 1);
    lcd.print(debug);
}

void printAddress(int rownum, byte* address)
{
    String addressString = "";
    for(int i = 0; i < 8; i++)
    {
        addressString += String(address[i], HEX);
        if(i < 7)
        addressString += ",";

    }
    printLineToLCD(rownum, addressString);
}
