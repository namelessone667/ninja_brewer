#ifndef PiLink_h_
#define PiLink_h_

#include "application.h"
#include <stdarg.h>
#include <stdio.h>
#include "theApp.h"
#include "JSONKeys.h"

//#define PILINK_SERIAL

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

#define DEVICE_ATTRIB_INDEX "i"
#define DEVICE_ATTRIB_CHAMBER "c"
#define DEVICE_ATTRIB_BEER "b"
#define DEVICE_ATTRIB_FUNCTION "f"
#define DEVICE_ATTRIB_HARDWARE "h"
#define DEVICE_ATTRIB_PIN "p"
#define DEVICE_ATTRIB_INVERT "x"
#define DEVICE_ATTRIB_DEACTIVATED "d"
#define DEVICE_ATTRIB_ADDRESS "a"
#define DEVICE_ATTRIB_CALIBRATEADJUST "j"	// value to add to temp sensors to bring to correct temperature
#define DEVICE_ATTRIB_VALUE "v"		// print current values
#define DEVICE_ATTRIB_WRITE "w"		// write value to device
#define DEVICE_ATTRIB_TYPE "t"
#define DEVICE_TYPE_SENSOR 1
#define DEVICE_TYPE_ACTUATOR 3


#ifdef PILINK_SERIAL
#define piStream Serial
#endif
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
typedef void (*ParseJsonCallback)(const char* key, const char* val, void* data);

enum DeviceFunction {
	DEVICE_NONE = 0,			// used as a sentry to mark end of list
	// chamber devices
	DEVICE_CHAMBER_DOOR = 1,	// switch sensor
	DEVICE_CHAMBER_HEAT = 2,
	DEVICE_CHAMBER_COOL = 3,
	DEVICE_CHAMBER_LIGHT = 4,		// actuator
	DEVICE_CHAMBER_TEMP = 5,
	DEVICE_CHAMBER_ROOM_TEMP = 6,	// temp sensors
	DEVICE_CHAMBER_FAN = 7,			// a fan in the chamber
	DEVICE_CHAMBER_RESERVED1 = 8,	// reserved for future use
	// carboy devices
	DEVICE_BEER_FIRST = 9,
	DEVICE_BEER_TEMP = DEVICE_BEER_FIRST,									// primary beer temp sensor
	DEVICE_BEER_TEMP2 = 10,								// secondary beer temp sensor
	DEVICE_BEER_HEAT = 11, DEVICE_BEER_COOL = 12,				// individual actuators
	DEVICE_BEER_SG = 13,									// SG sensor
	DEVICE_BEER_CAPPER = 14,
	DEVICE_PTC_COOL = 15,	// reserved
	DEVICE_MAX = 16
};

enum DeviceHardware {
	DEVICE_HARDWARE_NONE = 0,
	DEVICE_HARDWARE_PIN = 1,			// a digital pin, either input or output
	DEVICE_HARDWARE_ONEWIRE_TEMP = 2,	// a onewire temperature sensor
};

//TODO: implement as Publisher
//TODO: publish changes to CC and CV
class PiLink
{
public:
  // There can only be one PiLink object, so functions are static
	static void init(void)
  {
#ifdef PILINK_SERIAL
    piStream.begin(57600);
#else
		server.begin();
		Serial.println("TCPServer running on port 23");
#endif
  };

  static int read(int retries = 0)
  {
#ifdef PILINK_SERIAL
	  return piStream.read();
#else
		int i = 0;
		if(piStream.connected())
		{
			while(piStream.available() == 0)
			{
				if(i++ >= retries)
					return -1;
				delay(100);
			}
			return piStream.read();
		}
		else return -1;
#endif
  };

	static bool available()
	{
#ifndef PILINK_SERIAL
		if(piStream.connected() == false)
		{
			piStream = server.available();
		}

		if(piStream.connected())
			return piStream.available();
		else
			return false;
#endif
		return (piStream.available() > 0);
	};

	static void receive(void)
  {
		/*if (piStream.connected()) {
				// A TCP client has connected to the server.
				while (piStream.available()) {
				int c = piStream.read();

				// By the way, the actual telnet program will sent a bunch of telnet escape sequences
				// which appears as random garbage characters in serial. This isn't really a bug,
				// it's just that in the interest of clarity this isn't really a telnet server.
				piStream.write(c);
				Serial.write(c);
			}
		}
			else {
				// Check for an incoming connection. This is called repeatedly until a connection is made.
				piStream = server.available();
			}

		return;*/
		//const Logger& logger = theApp::getInstance().getLogger();
      while (available())
      {
        char inByte = read();
				logger().info(String::format("PiLink received: %c", inByte));
		      switch(inByte)
          {
            case ' ':
        		case '\n':
        		case '\r':
        		case 1:
        		case 3:
        		case 29:
        		case 31:
        		case '\'':
        		case 251:
        		case 253:
        		case 255:
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
						case 'd': // list devices in eeprom order
							if(read() == '{')
								while(read() != '}');
	 						openListResponse('d');
	 						listDevices();
	 						closeListResponse();
	 						break;
						case 'h':
							if(read() == '{')
								while(read() != '}');
							openListResponse('h');
							//listDevices();
							closeListResponse();
							break;
							case 'j': // Receive settings as json
								receiveJson();
								break;
            case 'l': // Display content requested
							printLCDtext();
         			break;
          }
      }
  };

	static void printLCDtext()
	{
		openListResponse('L');

		NinjaModel& model = theApp::getInstance().getModel();
		bool heatingEnabled = (model.ControllerMode.Get() == HEATER_ONLY || model.ControllerMode.Get() == COOLER_HEATER);
		// char stringBuffer[21];
		// String buffer = theApp::getInstance().getLCDText();
		// buffer.replace(2, ' ');
		// for(uint8_t i=0;i<4;i++)
		// {
		// 	int eol = buffer.indexOf('\n');
		// 	buffer.substring(0, eol == -1 ? buffer.length() : eol).toCharArray(stringBuffer, 20);
		// 	print("\"%s\"", stringBuffer);
		// 	char close = (i<3) ? ',':']';
		// 	print(close);
		// 	buffer = buffer.substring(eol+1);
		// }
		char stringBuffer[21];
	  String::format("F:%4.1fC  SET:%4.1fC", model.FridgeTemp.Get(), model.SetPoint.Get()).toCharArray(stringBuffer, 20);
		print("\"%s\",", stringBuffer);

	  String::format("B:%4.1fC  PID:%4.1fC", model.BeerTemp.Get(), model.Output.Get()).toCharArray(stringBuffer, 20);
	  print("\"%s\",", stringBuffer);

		if(heatingEnabled)
	  	String::format("PID:%s   HPID:%s", model.PIDMode == PID_MANUAL ? "MAN " : "AUTO", model.HeatPIDMode == PID_MANUAL ? "MAN " : "AUTO").toCharArray(stringBuffer, 20);
		else
			String::format("PID:%s", model.PIDMode == PID_MANUAL ? "MAN " : "AUTO").toCharArray(stringBuffer, 20);
	  print("\"%s\",", stringBuffer);

		String text;

		if(heatingEnabled)
	  	text = String::format("HEAT:%4.1f%%", model.HeatOutput.Get()).substring(0, 20);

	  while(text.length() < 18)
	    text.concat(' ');

	  switch(model.ControllerState)
	  {
	    case IDLE:
	      text.concat('I');
	      break;
	    case COOL:
	      text.concat('C');
	      break;
	    case HEAT:
	      text.concat('H');
	      break;
	    default:
	      text.concat('X');
	      break;
	  }

	  text.toCharArray(stringBuffer, 20);
	  print("\"%s\"]", stringBuffer);

		printNewLine();
	};

	static void receiveJson(void)
	{
		parseJson(&processJsonPair, NULL);
		return;
	};

	static void parseJson(ParseJsonCallback fn, void* data)
	{
		char key[30];
		char val[30];
		*key = 0;
		*val = 0;
		bool next = true;
		// read first open brace
		int c = read(10);
		if (c!='{')
		{
			logger().error("Json parsing error, expected '{' instead of '%c'", c);
			return;
		}
		do {
			next = parseJsonToken(key) && parseJsonToken(val);
			if (val[0] && key[0])
				fn(key, val, data);
		} while (next);
	};

	static bool parseJsonToken(char* val)
	{
		uint8_t index = 0;
		val[0] = 0;
		bool result = true;
		for(;;) // get value
		{
			int character = read(10);
			if (index==29 || character == '}' || character==-1) {
				result = false;
				break;
			}
			if(character == ',' || character==':')  // end of value
				break;
			if(character == ' ' || character == '"'){
				; // skip spaces and apostrophes
			}
			else
				val[index++] = character;
		}
		val[index]=0; // null terminate string
		return result;
	};

	static void processJsonPair(const char * key, const char * val, void* pv)
	{
		logger().info("Json received: %s = %s", key, val);
		theApp& app = theApp::getInstance();
		NinjaModel& model = app.getModel();

		if(strcmp(key, JSONKEY_mode) == 0)
		{
			setMode(val[0]);
		}
		else if(strcmp(key, JSONKEY_beerSetting) == 0)
		{
			setBeerSetting(val);
		}
		else if(strcmp(key, JSONKEY_fridgeSetting) == 0)
		{
			setFridgeSetting(val);
		}
		else if(strcmp(key, JSONKEY_Kp) == 0)
		{
			setProperty<double>(model.PID_Kp, val);
			app.saveState();
		}
		else if(strcmp(key, JSONKEY_Ki) == 0)
		{
			setProperty<double>(model.PID_Ki, val);
			app.saveState();
		}
		else if(strcmp(key, JSONKEY_Kd) == 0)
		{
			setProperty<double>(model.PID_Kd, val);
			app.saveState();
		}
		else if(strcmp(key, JSONKEY_tempSettingMin) == 0)
		{
			setProperty<double>(model.MinTemperature, val);
			app.saveState();
		}
		else if(strcmp(key, JSONKEY_tempSettingMax) == 0)
		{
			setProperty<double>(model.MaxTemperature, val);
			app.saveState();
		}
		else if(strcmp(key, JSONKEY_idleRangeHigh) == 0)
		{
			setProperty<double>(model.IdleDiff, val);
			app.saveState();
		}
		else if(strcmp(key, JSONKEY_minCoolIdleTime) == 0)
		{
			setProperty<int>(model.CoolMinOff, val);
			app.saveState();
		}
		else if(strcmp(key, JSONKEY_minHeatIdleTime) == 0)
		{
			setProperty<int>(model.HeatMinOff, val);
			app.saveState();
		}
		else if(strcmp(key, JSONKEY_minCoolTime) == 0)
		{
			setProperty<int>(model.CoolMinOn, val);
			app.saveState();
		}
		else if(strcmp(key, JSONKEY_mutexDeadTime) == 0)
		{
			setProperty<int>(model.MinIdleTime, val);
			app.saveState();
		}
		else if(strcmp(key, JSONKEY_coolEstimator) == 0)
		{
			setProperty<double>(model.PeakEstimator, val);
			app.saveState();
		}
		else if(strcmp(key, JSONKEY_coolingTargetUpper) == 0)
		{
			setProperty<double>(model.PeakDiff, val);
			app.saveState();
		}
		else if(strcmp(key, JSONKEY_maxCoolTimeForEstimate) == 0)
		{
			setProperty<int>(model.PeakMaxTime, val);
			app.saveState();
		}
		else
		{
			logger().info("Processing of key %s skipped", key);
		}
	};

	template<typename T>
	static void setProperty(Property<T>& property, const char* strVal)
	{
		T value;
		if(convertCharToVal<T>(strVal, value))
		{
			logger().info("setProperty success (strVal = %s)", strVal);
			property.Set(value);
		}
		else
		{
			logger().error("setProperty error (strVal = %s)", strVal);
		}
	};

	template<typename T>
	static bool convertCharToVal(const char* strVal, T& value);
	/*{
		logger().error("error default convertCharToVal called");
		return false;
	};*/

	static void setMode(char mode)
	{
		NinjaModel& model = theApp::getInstance().getModel();
		switch(mode)
		{
			case MODE_OFF:
				theApp::getInstance().DisableController();
				break;
			case MODE_FRIDGE_CONSTANT:
				model.PIDMode = PID_MANUAL;
#ifdef TEMP_PROFILES
				if(theApp::getInstance().getTemperatureProfile().IsActiveTemperatureProfile())
					theApp::getInstance().getTemperatureProfile().DeactivateTemperatureProfile();
#endif
				model.ExternalProfileActive = false;
				theApp::getInstance().ActivateController();
				break;
			case MODE_BEER_CONSTANT:
				model.PIDMode = PID_AUTOMATIC;
#ifdef TEMP_PROFILES
				if(theApp::getInstance().getTemperatureProfile().IsActiveTemperatureProfile())
					theApp::getInstance().getTemperatureProfile().DeactivateTemperatureProfile();
#endif
				model.ExternalProfileActive = false;
				theApp::getInstance().ActivateController();
				break;
			case MODE_BEER_PROFILE:
				model.PIDMode = PID_AUTOMATIC;
#ifdef TEMP_PROFILES
				if(theApp::getInstance().getTemperatureProfile().IsActiveTemperatureProfile() == false)
					theApp::getInstance().getTemperatureProfile().ActivateTemperatureProfile();
#endif
				model.ExternalProfileActive = true;
				theApp::getInstance().ActivateController();
				break;
			default:
				break;
		}
	};

	static void setBeerSetting(const char* val)
	{
		double setpoint = strtod(val, NULL);
		if(setpoint == 0.0)
		{
			logger().error("setBeerSettings: failed to convert input argument %s to double", val);
			return;
		}
		logger().info("setBeerSettings: setting new SetPoint to %f", setpoint);
		theApp::getInstance().setNewTargetTemp(setpoint);
	};

	static void setFridgeSetting(const char* val)
	{
		if(theApp::getInstance().getModel().PIDMode != PID_MANUAL)
		{
			logger().info("setFridgeSetting: Main PID in automatic mode, setting fridge temp skipped");
			return;
		}

		double setpoint = strtod(val, NULL);
		if(setpoint == 0.0)
		{
			logger().error("setFridgeSetting: failed to convert input argument %s to double", val);
			return;
		}
		logger().info("setFridgeSetting: PID in manual mode, setting new Setpoint to %f", setpoint);
		theApp::getInstance().setNewTargetTemp(setpoint);
	};

	static void listDevices()
	{
		//NinjaModel& model = theApp::getInstance().getModel();
		int id = 0;

		printDevice(id++, DEVICE_TYPE_SENSOR, 1, 0, DEVICE_CHAMBER_TEMP, DEVICE_HARDWARE_ONEWIRE_TEMP, 0, ONE_WIRE_BUS_PIN, theApp::getInstance().GetSensor1Address(), true);
		print(',');
		printDevice(id++, DEVICE_TYPE_SENSOR, 0, 1, DEVICE_BEER_TEMP, DEVICE_HARDWARE_ONEWIRE_TEMP, 0, ONE_WIRE_BUS_PIN, theApp::getInstance().GetSensor2Address());
		print(',');
		printDevice(id++, DEVICE_TYPE_ACTUATOR, 1, 0, DEVICE_CHAMBER_HEAT, DEVICE_HARDWARE_PIN, 0, HEATER_SSR_PIN, NULL);
		print(',');
		printDevice(id++, DEVICE_TYPE_ACTUATOR, 1, 0, DEVICE_CHAMBER_COOL, DEVICE_HARDWARE_PIN, 0, COOLER_SSR_PIN, NULL);
	};

	static void printDevice(int id, int type, int chamber, int beer, int function, int hw, int deact, int pin, const char* address, bool first=false)
	{
		firstPair = true;
		sendJsonPair(DEVICE_ATTRIB_INDEX, id);
		sendJsonPair(DEVICE_ATTRIB_TYPE, type);
		sendJsonPair(DEVICE_ATTRIB_CHAMBER, chamber);
		sendJsonPair(DEVICE_ATTRIB_BEER, beer);
		sendJsonPair(DEVICE_ATTRIB_FUNCTION, function);
		sendJsonPair(DEVICE_ATTRIB_HARDWARE, hw);
		sendJsonPair(DEVICE_ATTRIB_DEACTIVATED, deact);
		sendJsonPair(DEVICE_ATTRIB_PIN, pin);
		if(address != NULL)
			sendJsonPair(DEVICE_ATTRIB_ADDRESS, address);
		sendJsonClose(false);
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
		else if(model.ExternalProfileActive)
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
		sendJsonPair(JSONKEY_idleRangeLow, 0); //not used
		sendJsonPair(JSONKEY_heatingTargetUpper, (double)0); //not used, heating estimator peak error
		sendJsonPair(JSONKEY_heatingTargetLower, (double)0);//not used, heating estimator peak error
		sendJsonPair(JSONKEY_coolingTargetUpper, model.PeakDiff.Get());
		sendJsonPair(JSONKEY_coolingTargetLower, 0); //not used
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

  static void sendJsonClose(bool newline = true)
  {
	   print("}");
		 if(newline)
	   	printNewLine();
  };

  static void sendJsonAnnotation(const char* name, const char* annotation)
  {
	   printJsonName(name);
	   const char* fmtAnn = annotation ? "\"%s\"" : "null";
	   print(fmtAnn, annotation);
  };

	static const Logger& logger()
	{
		return theApp::getInstance().getLogger();
	}

private:
  static bool firstPair;
  static char printfBuff[PRINTF_BUFFER_SIZE];
#ifndef PILINK_SERIAL
	static TCPServer server;
	static TCPClient piStream;
#endif
};

#endif
