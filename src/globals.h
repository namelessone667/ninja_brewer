#ifndef globals_h
#define globals_h

/******** feaures section *****************/
#define HERMS_MODE
#define DEBUG_HERMS
#define LCD20_4
#define TEMP_PROFILES
//#define LOG_PID
//#define LOG_HEAT_PID
#define USE_WIFI
#define USE_PARTICLE
//#define USE_UBIDOTS
#define USE_BLYNK
#define USE_PAPERTAIL
//#define BREWPI_LINK
//#define SIMULATE_TEMP_SENSORS
/******** feaures section end *****************/

/******** Blynk Virtual PINS definition *******/
#define PIN_BTN_ON_OFF V0
#define PIN_NEW_SETPOINT V1
#define PIN_BTN_SET_NEW_SETPOINT V2
#define PIN_STATUS_LED V3
#define PIN_FRIDGE_TEMP V5
#define PIN_BEER_TEMP V6
#define PIN_PID_OUTPUT V7
#define PIN_PID_SETPOINT V8
#define PIN_HEATPID_OUTPUT V9
#define HPID_PTERM V10
#define HPID_ITERM V11
#define HPID_DTERM V12

#define PIN_TEMP_PROFILE V13
#define PIN_STEP_TEMPERATURE V14
#define PIN_STEP_DURATION V15
#define PIN_STEP_DURATION_UNIT V16
#define PIN_ADD_STEP_BTN V17
#define PIN_CLEAR_STEPS_BTN V18
#define PIN_TEMPROFILE_ON_OFF_BTN V19
#define PIN_TEMPROFILE_TABLE V20
#define PIN_TEMPROFILESTEP_TYPE V21

/******** Blynk Virtual PINS definition end *******/

#define BLYNK_CONNECT_RETRY 60000

/******** Particle GPIO PIN mapping **************/
#define COOLER_SSR_PIN A1
#define HEATER_SSR_PIN A2
#define LEFT_BUTTON_PIN A3
#define RIGHT_BUTTON_PIN A4
#define ENCODER_PIN_A D4
#define ENCODER_PIN_B D3

#define ONE_WIRE_BUS_PIN D6

/******** Particle GPIO PIN mapping end ***********/

#define PAPERTAIL_SERVER "logs.papertrailapp.com"//"logs7.papertrailapp.com:10532"// //logs7.papertrailapp.com:10532
#define PAPERTAIL_APP_NAME "herms_brewer"
#define LOG_LEVEL LOG_LEVEL_ALL

#define TEMP_ERR_INTERVAL 600000

#endif
