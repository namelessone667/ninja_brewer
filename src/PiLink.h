#ifndef PiLink_h_
#define PiLink_h_

#include "application.h"
#include <stdarg.h>
#include <stdio.h>
#include "theApp.h"
#include "JSONKeys.h"

#define PILINK_SERIAL

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

#define TCP_TIMEOUT 5000


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

class PiLink
{
typedef void (PiLink::*ParseJsonCallback)(const char* key, const char* val, void* data);

public:
	static PiLink& getInstance();
	PiLink(PiLink const&)               = delete;
	void operator=(PiLink const&)       = delete;

	void init(void);
	void receive(void);
  void print(const char *fmt, ... );
  void print(char c);
  void printNewLine();
	void sendControlSettings(void);
	void sendControlConstants(void);
	void sendControlVariables(void);
  void printTemperatures(void);

private:
	PiLink();
	void printTemperaturesJSON(const char * beerAnnotation, const char * fridgeAnnotation);
  void printResponse(char type);
  void openListResponse(char type);
  void closeListResponse();
  void sendJsonTemp(const char* name, double temp);
	void sendJsonPair(const char * name, double val);
  void sendJsonPair(const char * name, const char * val);
  void sendJsonPair(const char * name, char val);
  void sendJsonPair(const char * name, uint16_t val);
	void sendJsonPair(const char * name, int val);
  void sendJsonPair(const char * name, uint8_t val);
  void printJsonName(const char * name);
  void printJsonSeparator();
  void sendJsonClose(bool newline = true);
  void sendJsonAnnotation(const char* name, const char* annotation);
	const Logger& logger();
	char * fixedPointToString(char * s, double value, uint8_t numDecimals, uint8_t maxLength);
	void printLCDtext();
	void receiveJson(void);
	void parseJson(ParseJsonCallback fn, void* data);
	bool parseJsonToken(char* val);
	void processJsonPair(const char * key, const char * val, void* pv);
	template<typename T>
	void setProperty(Property<T>& property, const char* strVal);
	template<typename T>
	bool convertCharToVal(const char* strVal, T& value);
	void setMode(char mode);
	void setBeerSetting(const char* val);
	void setFridgeSetting(const char* val);
	void listDevices();
	void printDevice(int id, int type, int chamber, int beer, int function, int hw, int deact, int pin, const char* address, bool first=false);
	int read(int retries = 0);
	bool available();

	NinjaModel& model;
	const Logger& loggerref;
  bool firstPair;
  char printfBuff[PRINTF_BUFFER_SIZE];
#ifndef PILINK_SERIAL
	TCPServer server;
	TCPClient piStream;
#endif
};

#endif
