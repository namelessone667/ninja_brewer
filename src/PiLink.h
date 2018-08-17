#ifndef PiLink_h_
#define PiLink_h_

#include "application.h"
#include <stdarg.h>
#include <stdio.h>
#include "theApp.h"


#define VERSION_STRING "0.2.4"
#define BUILD_NUMBER 0
#define BUILD_NAME "00000000"
#define BREWPI_STATIC_CONFIG 0
#define BREWPI_SIMULATE 0
#define BREWPI_BOARD '?' //unknown, use 's' for standard
#define BREWPI_LOG_MESSAGES_VERSION 1
#define PRINTF_BUFFER_SIZE 128

#define JSON_BEER_TEMP  "BeerTemp"
#define JSON_BEER_SET	"BeerSet"
#define JSON_BEER_ANN	"BeerAnn"
#define JSON_FRIDGE_TEMP "FridgeTemp"
#define JSON_FRIDGE_SET  "FridgeSet"
#define JSON_FRIDGE_ANN  "FridgeAnn"
#define JSON_STATE		"State"
#define JSON_TIME		"Time"
#define JSON_ROOM_TEMP  "RoomTemp"


#define piStream Serial
/*enum states{
	IDLE,						// 0
	STATE_OFF,					// 1
	DOOR_OPEN,					// 2 used by the Display only
	HEATING,					// 3
	COOLING,					// 4
	WAITING_TO_COOL,			// 5
	WAITING_TO_HEAT,			// 6
	WAITING_FOR_PEAK_DETECT,	// 7
	COOLING_MIN_TIME,			// 8
	HEATING_MIN_TIME,			// 9
	NUM_STATES
};*/

class PiLink
{
public:
  // There can only be one PiLink object, so functions are static
	static void init(void)
  {
    piStream.begin(57600);
  };

  static int read()
  {
	   return piStream.read();
  };

	static void receive(void)
  {
      while (piStream.available() > 0)
      {
        char inByte = read();
		      switch(inByte)
          {
            case ' ':
        		case '\n':
        		case '\r':
        		/*case 1:
        		case 3:
        		case 29:
        		case 31:
        		case '\'':
        		case 251:
        		case 253:
        		case 255:*/
        			break;
            case 'n':
        			// v version
        			// s shield type
        			// y: simulator
        			// b: board

        			print("N:{\"v\":\"");
        			print(VERSION_STRING);
        			print("\",\"n\":%d,\"c\":\"", BUILD_NUMBER);
        			print(BUILD_NAME);
        			print("\",\"s\":%d,\"y\":%d,\"b\":\"%c\",\"l\":\"%d\"}",
        				BREWPI_STATIC_CONFIG,
        				BREWPI_SIMULATE,
        				BREWPI_BOARD,
        				BREWPI_LOG_MESSAGES_VERSION);

        			printNewLine();
        			break;
            case 't': // temperatures requested
  			       printTemperatures();
  			       break;
            case 'l': // Display content requested
   			      openListResponse('L');
         			char stringBuffer[21];
              String buffer = theApp::getInstance().getLCDText();
         			for(uint8_t i=0;i<4;i++)
              {
                int eol = buffer.indexOf('\n');
                unsigned int current_row = 0;
                buffer.substring(0, eol == -1 ? buffer.length() : eol).toCharArray(stringBuffer, 20);
                print("\"%s\"", stringBuffer);
                char close = (i<3) ? ',':']';
         				print(close);
                buffer = buffer.substring(eol+1);
              }
         			printNewLine();
         			break;
          }
      }
  };

  // create a printf like interface to the Arduino Serial function. Format string stored in RAM
  static void print(const char *fmt, ... )
  {
  	va_list args;
  	va_start (args, fmt );
  	vsnprintf(printfBuff, PRINTF_BUFFER_SIZE, fmt, args);
  	va_end (args);

  	piStream.print(printfBuff);
  };

  static void print(char c)
  {
    piStream.print(c);
  };

  static void printNewLine()
  {
		piStream.println();
	};

  static void printTemperatures(void)
  {
	// print all temperatures with empty annotations
	printTemperaturesJSON(0, 0);
  };

  static void printTemperaturesJSON(const char * beerAnnotation, const char * fridgeAnnotation){
  	printResponse('T');
    NinjaModel model = theApp::getInstance().getModel();

  	sendJsonTemp(JSON_BEER_TEMP, model.BeerTemp.Get());

  	sendJsonTemp(JSON_BEER_SET, model.SetPoint.Get());

  	sendJsonAnnotation(JSON_BEER_ANN, beerAnnotation);

  	sendJsonTemp(JSON_FRIDGE_TEMP, model.FridgeTemp.Get());

  	sendJsonTemp(JSON_FRIDGE_SET, model.Output.Get());

		sendJsonAnnotation(JSON_FRIDGE_ANN, fridgeAnnotation);

  	//if (tempControl.ambientSensor->isConnected() && changed(roomTemp, t))
  	//	sendJsonTemp(PSTR(JSON_ROOM_TEMP), tempControl.getRoomTemp());
    unsigned char state;
    switch(model.ControllerState.Get())
    {
      case IDLE:
        state = 0;
        break;
      case COOL:
        state = 4;
        break;
      case HEAT:
        state = 3;
        break;
      default:
        state = 1;
        break;
    }
  	sendJsonPair(JSON_STATE, (unsigned char)state);

  	sendJsonClose();
  };

  static void printResponse(char type)
  {
	   print("%c:", type);
	   firstPair = true;
  };

  static void openListResponse(char type)
  {
	   printResponse(type);
	   print('[');
  };

  static void closeListResponse()
  {
	   print(']');
	   printNewLine();
  };

  static void sendJsonTemp(const char* name, double temp)
  {
  	char tempString[9];
    snprintf(tempString, 9, "%.3f", temp);
  	printJsonName(name);
  	print(tempString);
  };

  static void sendJsonPair(const char * name, const char * val)
  {
	   printJsonName(name);
	   piStream.print(val);

  };

  static void sendJsonPair(const char * name, char val)
  {
  	printJsonName(name);
  	print('"');
  	print(val);
  	print('"');
  };

  static void sendJsonPair(const char * name, uint16_t val)
  {
	   printJsonName(name);
	   print("%u", val);
  };

  static void sendJsonPair(const char * name, uint8_t val)
  {
	   sendJsonPair(name, (uint16_t)val);
  };

  static void printJsonName(const char * name)
  {
  	printJsonSeparator();
  	print('"');
  	print(name);
  	print("\":");
  };

  inline static void printJsonSeparator()
  {
	   print(firstPair ? '{' : ',');
	   firstPair = false;
  };

  static void sendJsonClose()
  {
	   print("}");
	   printNewLine();
  };

  static void sendJsonAnnotation(const char* name, const char* annotation)
  {
	   printJsonName(name);
	   const char* fmtAnn = annotation ? "\"%s\"" : "null";
	   print(fmtAnn, annotation);
  };

private:
  static bool firstPair;
  static char printfBuff[PRINTF_BUFFER_SIZE];
};

#endif
