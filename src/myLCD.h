//#include <LCD.h>
#include "LiquidCrystal_I2C.h"
#include "fridge.h"

void initLCD();

void progressIndicator();

void printTemperature(float, int);
void printPIDOutputTemp(double temp);
void printPIDOutputPercent(int);
void printFridgeState(byte);
void printHeatPIDPercent(double);
void printLineToLCD(int rownum, const char* message);
void printLineToLCD(int rownum, int columnum, const char* message);
void printDebugLetter(const char* debug);
void printAddress(int rownum, byte* address);
