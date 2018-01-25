#ifndef globals_h
#define globals_h

#define ENABLE_PID

#define MAX_FRIDE_TEMP 35
#define MIN_FRIDGE_TEMP 2

#define COOLING_PID_MODE PID_AUTOMATIC
#define HEATINH_PID_MODE PID_AUTOMATIC

#define HEAT_MIN_PERCENT    0
#define HEAT_MAX_PERCENT    25

#define PIN_FRIDGE_TEMP V5
#define PIN_BEER_TEMP V6
#define PIN_PID_OUTPUT V7
#define PIN_PID_SETPOINT V8
#define PIN_HEATPID_OUTPUT V9

#define BLYNK_CONNECT_RETRY 60000

#define LEFT_BUTTON_PIN A4
#define RIGHT_BUTTON_PIN A3
#define ENCODER_PIN_A D3
#define ENCODER_PIN_B D4

#define ONE_WIRE_BUS_PIN D6

#define PAPERTAIL_SERVER "logs.papertrailapp.com"
#define LOG_LEVEL LOG_LEVEL_ALL

#endif
