#ifndef PiLink_h_
#define PiLink_h_

#include "application.h"
#include <stdarg.h>
#include <stdio.h>
#include "theApp.h"
#include "JSONKeys.h"


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

#define	MODE_FRIDGE_CONSTANT 'f'
#define MODE_BEER_CONSTANT 'b'
#define MODE_BEER_PROFILE 'p'
#define MODE_OFF 'o'
#define MODE_TEST 't'


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
						case 's': // Control settings requested
							sendControlSettings();
							break;
						case 'c': // Control constants requested
							sendControlConstants();
							break;
						case 'v': // Control variables requested
							sendControlVariables();
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

	// Send settings as JSON string
	static void sendControlSettings(void)
	{
		NinjaModel model = theApp::getInstance().getModel();
		char tempString[12], mode;
		if(model.StandBy)
			mode = MODE_OFF;
		else if(theApp::getInstance().getTemperatureProfile().IsActiveTemperatureProfile())
			mode = MODE_BEER_PROFILE;
		else if(model.PIDMode == PID_MANUAL)
			mode = MODE_FRIDGE_CONSTANT;
		else
			mode = MODE_BEER_CONSTANT;

		printResponse('S');
		sendJsonPair(JSONKEY_mode, mode);
		sendJsonPair(JSONKEY_beerSetting, fixedPointToString(tempString, model.SetPoint, 2, 12));
		sendJsonPair(JSONKEY_fridgeSetting, fixedPointToString(tempString, model.Output, 2, 12));
		sendJsonPair(JSONKEY_heatEstimator, fixedPointToString(tempString, 0, 3, 12));
		sendJsonPair(JSONKEY_coolEstimator, fixedPointToString(tempString, model.PeakEstimator, 3, 12));
		sendJsonClose();
	};

	static void sendControlConstants(void)
	{
		NinjaModel model = theApp::getInstance().getModel();
		//jsonOutputBase = (uint8_t*)&tempControl.cc;
		//sendJsonValues('C', jsonOutputCCMap, sizeof(jsonOutputCCMap)/sizeof(jsonOutputCCMap[0]));
		printResponse('C');

		sendJsonPair(JSONKEY_tempFormat, 'C');
		sendJsonPair(JSONKEY_tempSettingMin, model.MinTemperature.Get());
		sendJsonPair(JSONKEY_tempSettingMax, model.MaxTemperature.Get());
		sendJsonPair(JSONKEY_pidMax, (double)0); //not used for now, the max diff between StePoint and Output

		sendJsonPair(JSONKEY_Kp, model.PID_Kp.Get());
		sendJsonPair(JSONKEY_Ki, model.PID_Ki.Get());
		sendJsonPair(JSONKEY_Kd, model.PID_Kd.Get());

		sendJsonPair(JSONKEY_iMaxError, (double)0); //not used for now, the max error to consider when updating PID integrator
		sendJsonPair(JSONKEY_idleRangeHigh, model.IdleDiff.Get());
		sendJsonPair(JSONKEY_idleRangeLow, model.IdleDiff.Get());
		sendJsonPair(JSONKEY_heatingTargetUpper, (double)0); //not used, heating estimator peak error
		sendJsonPair(JSONKEY_heatingTargetLower, (double)0);//not used, heating estimator peak error
		sendJsonPair(JSONKEY_coolingTargetUpper, model.PeakDiff.Get());
		sendJsonPair(JSONKEY_coolingTargetLower, model.PeakDiff.Get()*-1);
		sendJsonPair(JSONKEY_maxHeatTimeForEstimate, (uint16_t)0); //not used, heat estimator max time
		sendJsonPair(JSONKEY_maxCoolTimeForEstimate, model.PeakMaxTime.Get());

		sendJsonPair(JSONKEY_minCoolTime, model.CoolMinOn.Get());
		sendJsonPair(JSONKEY_minCoolIdleTime, model.CoolMinOff.Get());
		sendJsonPair(JSONKEY_minHeatTime, (uint16_t)0); //minimum heat time not defined
		sendJsonPair(JSONKEY_minHeatIdleTime, model.HeatMinOff.Get());
		sendJsonPair(JSONKEY_mutexDeadTime, model.MinIdleTime.Get());

		sendJsonPair(JSONKEY_fridgeFastFilter, (uint8_t)0); //not used
		sendJsonPair(JSONKEY_fridgeSlowFilter, (uint8_t)0); //not used
		sendJsonPair(JSONKEY_fridgeSlopeFilter, (uint8_t)0); //not used
		sendJsonPair(JSONKEY_beerFastFilter, (uint8_t)0); //not used
		sendJsonPair(JSONKEY_beerSlowFilter, (uint8_t)0); //not used
		sendJsonPair(JSONKEY_beerSlopeFilter, (uint8_t)0); //not used

		sendJsonPair(JSONKEY_lightAsHeater, (uint8_t)0); //not used
		sendJsonPair(JSONKEY_rotaryHalfSteps, (uint8_t)0); //not used

		sendJsonClose();
	};

	static void sendControlVariables(void)
	{
		//jsonOutputBase = (uint8_t*)&tempControl.cv;
		//sendJsonValues('V', jsonOutputCVMap, sizeof(jsonOutputCVMap)/sizeof(jsonOutputCVMap[0]));
		NinjaModel model = theApp::getInstance().getModel();
		printResponse('V');

		double error = model.SetPoint.Get() - model.BeerTemp.Get();

		sendJsonPair(JSONKEY_beerDiff, error);
		sendJsonPair(JSONKEY_diffIntegral, error);
		sendJsonPair(JSONKEY_beerSlope, 0);
		sendJsonPair(JSONKEY_p, theApp::getInstance().GetPIDPTerm());
		sendJsonPair(JSONKEY_i, theApp::getInstance().GetPIDITerm());
		sendJsonPair(JSONKEY_d, theApp::getInstance().GetPIDDTerm());
		sendJsonPair(JSONKEY_estimatedPeak, 0); //not used
		sendJsonPair(JSONKEY_negPeakEstimate, 0); //not used
		sendJsonPair(JSONKEY_posPeakEstimate, 0); //not used
		sendJsonPair(JSONKEY_negPeak, 0); //not used
		sendJsonPair(JSONKEY_posPeak, 0); //not used

		sendJsonClose();
	};

	static char * fixedPointToString(char * s, double value, uint8_t numDecimals, uint8_t maxLength){
		snprintf(s, maxLength, "%.*f", numDecimals, value);
		return s;
	}

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

	static void sendJsonPair(const char * name, double val)
  {
	   printJsonName(name);
	   print("%f",val);
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

	static void sendJsonPair(const char * name, int val)
  {
	   printJsonName(name);
	   print("%d", val);
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
