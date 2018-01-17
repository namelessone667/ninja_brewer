// This #include statement was automatically added by the Particle IDE.
#include "theApp.h"
#include "globals.h"
#include "secrets.h"

#include "myUbidots.h"
#include <blynk.h>
#include "PID_v1.h"
#include "probe.h"
#include "fridge.h"
//#include "myLCD.h"

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

const int EEPROM_VER = 1;  // eeprom data tracking

//#define PROBES_HERMS

#ifdef PROBES_HERMS
byte probe1addr[] = {0x28, 0x78, 0x28, 0x85, 0x8, 0x0, 0x0, 0x9c}; //thermowell white
byte probe2addr[] = {0x28, 0x78, 0x28, 0x85, 0x8, 0x0, 0x0, 0x9c}; //thermowell white
#endif

#ifndef PROBES_HERMS
////byte probe2addr[] = {0x28, 0xd6, 0xd5, 0x84, 0x08, 0x00, 0x00, 0x16}; //???
byte probe1addr[] = {0x28, 0x31, 0x9b, 0xec, 0x07, 0x00, 0x00, 0x22}; //probe fridge
//byte probe2addr[] = {0x28, 0xff, 0x53, 0x7d, 0x83, 0x16, 0x03, 0x84}; //probe beer
byte probe2addr[] = {0x28, 0x9d, 0x87, 0xe8, 0x8, 0x0, 0x0, 0xf}; //thermowell (pink)
//byte probe2addr[] = {0x28, 0x78, 0x28, 0x85, 0x8, 0x0, 0x0, 0x9c}; //thermowell white

// thermowell (pink) 0x28, 0xd6, 0xd5, 0x84, 0x8, 0x0, 0x0, 0x16
//byte probe1addr[] = {0x28, 0xd6, 0xd5, 0x84, 0x8, 0x0, 0x0, 0x16}; //thermowell (pink)

#endif

byte oneWireDevices1[][8] =   {
                                {0x28, 0x31, 0x9b, 0xec, 0x07, 0x00, 0x00, 0x22} //probe fridge

                            };

byte oneWireDevices2[][8] =   {
                                {0x28, 0x9d, 0x87, 0xe8, 0x8, 0x0, 0x0, 0xf}, //thermowell (pink)
                                {0x28, 0x78, 0x28, 0x85, 0x8, 0x0, 0x0, 0x9c} //thermowell white
                            };


char auth[] = BLYNK_AUTH;

OneWire oneWire2(ONE_WIRE_BUS_PIN);
//probe fridge(&oneWire2, probe1addr), beer(&oneWire2, probe2addr);
//probe fridge(&oneWire2), beer(&oneWire2);
probe* tempProbe = NULL;
probe* fridge = NULL;
probe* beer = NULL;

double Input, Setpoint, Output, Kp, Ki, Kd, ITerm, fridgeTemp;  // SP, PV, CO, tuning params for main PID
double heatInput, heatOutput, heatSetpoint, heatKp, heatKi, heatKd;
PID mainPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, PID_DIRECT);  // main PID instance for beer temp control (DIRECT: beer temperature ~ fridge(air) temperature)
PID heatPID(&heatInput, &heatOutput, &heatSetpoint, heatKp, heatKi, heatKd, PID_DIRECT);
const byte relay1 = A1; //cooling
const byte relay2 = A2; //heating
unsigned long lastTime = 0UL;
unsigned long blynkConnectTimestamp = 0UL;
bool inHermsMode = false;
bool firstrun = true;
bool publish = true;
bool controledFermentation = false;
bool disableControl = false;
int pidMode = PID_MANUAL, heatPIDMode = PID_MANUAL;
bool debug = true;
bool setup_failed = false;
bool sensor_failure = false;
bool discoverDevicesFlg = false;
int probeFailureCounter = 0;


void publishStatus()
{
    if(!publish || !WiFi.ready())
        return;
    //Every 60 seconds publish uptime
    //beer, fridge, setPoint, pidoutput
    double now = millis();
    if (now-lastTime>60000UL) {
        lastTime = now;

        if(inHermsMode == false)
        {
            //VO - beer temp
            //V1 - fridge temp
            //V2 - target fermentation temperature
            //V3 - pid output temperature
            //V4 - pid mode auto/manual
            //V5 - heat pid mode auto / manual
            //V6 - controlled fermentation on/off
            //V7 - status LED cooling / idle/ heating
            //V8 -
            //V9 -
            //Blynk.virtualWrite(V0, beer.getFilter());
            //Blynk.virtualWrite(V1, fridge.getFilter());
            //.virtualWrite(V3, Output);

            ubidots.add("beertemperature", beer->getFilter());
            ubidots.add("temperature", fridge->getFilter());
            ubidots.add("pidintegralterm", mainPID.GetITerm());
            //ubidots.add("heatpidintegralterm", heatPID.GetITerm());
            ubidots.add("targetbeertemperature", Setpoint);
            ubidots.add("pidoutputtemperature", Output);
            ubidots.add("heatpidoutput", heatOutput);

            ubidots.sendAll();

            Blynk.virtualWrite(PIN_FRIDGE_TEMP, fridge->getFilter());
            Blynk.virtualWrite(PIN_BEER_TEMP, beer->getFilter());
            Blynk.virtualWrite(PIN_PID_OUTPUT, Output);
            Blynk.virtualWrite(PIN_PID_SETPOINT, Setpoint);
            Blynk.virtualWrite(PIN_HEATPID_OUTPUT, heatOutput);

        }
        else
        {
            ubidots.add("herms1", beer->getTemp());
            ubidots.add("herms2", fridge->getTemp());
            ubidots.sendAll();

        }
    }
}

void EEPROMWritePresets() {      // save defaults to eeprom

    byte temp = EEPROM_VER;
    EEPROM.put(0, temp);

    EEPROM.put(10, (double)18.00);       // default main Setpoint
    EEPROM.put(20, (double)18.00);       // default main Output for manual operation
    EEPROM.put(30, (double)3.00);      // default main Kp
    EEPROM.put(40, (double)5E-4);       // default main Ki,
    EEPROM.put(50, (double)0.0);     // default main Kd
    EEPROM.put(60, (double)0.00);      // default HEAT Output for manual operation
    EEPROM.put(70, (double)5.00);      // default HEAT Kp original value 5.0
    EEPROM.put(80, (double)0.025);      // default HEAT Ki original value 00.25
    EEPROM.put(90, (double)0.0);      // default HEAT Kd original value 1.15
    EEPROM.put(100, (double)05.00);      // default peakEstimator
    EEPROM.put(110, (double)18.00); // ITerm
    EEPROM.put(120, (bool)false); //controledFermentation
    EEPROM.put(130, (int)PID_MANUAL); //mainPID mode
    EEPROM.put(140, (int)PID_MANUAL); //heatPID mode

}

void EEPROMWriteSettings() {  // write current settings to EEPROM
  byte temp = EEPROM_VER;
  EEPROM.put(0, temp);
  EEPROM.put(10, Setpoint);
  EEPROM.put(20, Output);
  EEPROM.put(30, Kp);
  EEPROM.put(40, Ki);
  EEPROM.put(50, Kd);
  EEPROM.put(60, heatOutput);
  EEPROM.put(70, heatKp);
  EEPROM.put(80, heatKi);
  EEPROM.put(90, heatKd);
  double* estimator = getPeakEstimatorAddr();
  EEPROM.put(100, *estimator);
  EEPROM.put(110, mainPID.GetITerm());
  EEPROM.put(120, controledFermentation); //isControledFermentation
  EEPROM.put(130, mainPID.GetMode()); //mainPID mode
  EEPROM.put(140, heatPID.GetMode()); //heatPID mode
}

void EEPROMReadSettings() {  // read settings from EEPROM

  EEPROM.get(10, Setpoint);
  EEPROM.get(20, Output);
  EEPROM.get(30, Kp);
  EEPROM.get(40, Ki);
  EEPROM.get(50, Kd);
  EEPROM.get(60, heatOutput);
  EEPROM.get(70, heatKp);
  EEPROM.get(80, heatKi);
  EEPROM.get(90, heatKd);
  double* estimator = getPeakEstimatorAddr();
  EEPROM.get(100, *estimator);
  EEPROM.get(110, ITerm);
  EEPROM.get(120, controledFermentation);
  EEPROM.get(130, pidMode);
  EEPROM.get(140, heatPIDMode);
}

int setPIDMode(String arg, int mode)
{
    double newSetPoint = strtod(arg, NULL);
    if(newSetPoint >= MIN_FRIDGE_TEMP && newSetPoint <= MAX_FRIDE_TEMP)
    {
      Setpoint = newSetPoint;
      EEPROM.put(10, Setpoint);
      mainPID.SetMode(mode);
      if(mode == PID_MANUAL)
        Output = newSetPoint;
      return 1;
    }
    return -1;

}

int setPIDModeManual(String arg)
{
    return setPIDMode(arg, PID_MANUAL);
}

int setPIDModeAuto(String arg)
{
    return setPIDMode(arg, PID_AUTOMATIC);
}

int setHeatPIDManual(String arg)
{
    double heatPercent = strtod(arg, NULL);
    if(heatPercent >= HEAT_MIN_PERCENT && heatPercent <= HEAT_MAX_PERCENT)
    {
        heatPID.SetMode(PID_MANUAL);
        heatOutput = heatPercent;
        return 1;
    }
    return -1;
}

int setHeatPIDAuto(String arg)
{
    heatPID.SetMode(PID_AUTOMATIC);
    return 1;
}

void initPIDs()
{
    mainPID.SetTunings(Kp, Ki, Kd);    // set tuning params
    mainPID.SetSampleTime(1000);       // (ms) matches sample rate (1 hz)
    mainPID.SetOutputLimits(1, 28);  // deg C (~32.5 - ~100 deg F)
    mainPID.setOutputType(PID_FILTERED);
    mainPID.setFilterConstant(1000);
    mainPID.initHistory();

    mainPID.SetMode(pidMode);  // set man/auto
    mainPID.SetITerm(ITerm);

    heatPID.SetTunings(heatKp, heatKi, heatKd);
    heatPID.SetSampleTime(1000);       // sampletime = time proportioning window length
    heatPID.SetOutputLimits(HEAT_MIN_PERCENT, HEAT_MAX_PERCENT);  // heatPID output = duty time per window
    heatPID.SetMode(heatPIDMode);

    pinMode(relay1, OUTPUT);  // configure relay pins and write default HIGH (relay open)
    pinMode(relay2, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
  	digitalWrite(relay1, LOW);
    digitalWrite(relay2, LOW);
    digitalWrite(LED_BUILTIN, LOW);
}

int reboot(String arg)
{
    EEPROMWriteSettings();
    System.reset();
    return 1;
}

void wd_reboot()
{
    controledFermentation = false;
    reboot("");
}

ApplicationWatchdog wd(60000, wd_reboot, 1024);

int setPIDKp(String arg)
{
    double newKp = strtod(arg, NULL);
    Kp = newKp;
    EEPROMWriteSettings();
    mainPID.SetTunings(Kp, Ki, Kd);
    return 1;
}

int setPIDKi(String arg)
{
    double newKi = strtod(arg, NULL);
    Ki = newKi;
    EEPROMWriteSettings();
    mainPID.SetTunings(Kp, Ki, Kd);
    return 1;
}

int setPIDKd(String arg)
{
    double newKd = strtod(arg, NULL);
    Kd = newKd;
    EEPROMWriteSettings();
    mainPID.SetTunings(Kp, Ki, Kd);
    return 1;
}

int setHeatPIDKp(String arg)
{
    double newKp = strtod(arg, NULL);
    heatKp = newKp;
    EEPROMWriteSettings();
    heatPID.SetTunings(heatKp, heatKi, heatKd);
    return 1;
}

int setHeatPIDKi(String arg)
{
    double newKi = strtod(arg, NULL);
    heatKi = newKi;
    EEPROMWriteSettings();
    heatPID.SetTunings(heatKp, heatKi, heatKd);
    return 1;
}

int setHeatPIDKd(String arg)
{
    double newKd = strtod(arg, NULL);
    heatKd = newKd;
    EEPROMWriteSettings();
    heatPID.SetTunings(heatKp, heatKi, heatKd);
    return 1;
}

int startControl(String arg)
{
    controledFermentation = true;
    EEPROMWriteSettings();
    return 1;
}

int stopControl(String arg)
{
    controledFermentation = false;
    EEPROMWriteSettings();
    disableControl = true;
    return 1;
}

int saveState(String arg)
{
    EEPROMWriteSettings();
    return 1;
}

int reinitLCD(String arg)
{
    //initLCD();
    return 1;
}

int getOneWireDevices(String ars)
{
    discoverDevicesFlg = true;
    return 1;
}

void discoverDevices()
{
    oneWire2.reset_search();
    byte address[8];
    String addressString;
    while(oneWire2.search(address) == 1)
    {
       addressString = "";
       for(int i = 0; i < 8; i++)
       {
           addressString += "0x" + String(address[i], HEX);
           if(i < 7)
            addressString += ", ";
       }
       Particle.publish("DeviceOnBus", addressString);
    }
    discoverDevicesFlg = false;
}

STARTUP(WiFi.selectAntenna(ANT_AUTO));

void registerToCloud()
{
    Particle.function("setPIDAuto", setPIDModeAuto);
    Particle.function("setPIDManual", setPIDModeManual);
    Particle.function("setHPIDMan", setHeatPIDManual);
    Particle.function("setHPIDAuto", setHeatPIDAuto);
    Particle.function("reboot", reboot);
    Particle.function("setPIDKi", setPIDKi);
    Particle.function("setPIDKp", setPIDKp);
    Particle.function("setPIDKd", setPIDKd);
    Particle.function("setHeatPIDKi", setHeatPIDKi);
    Particle.function("setHeatPIDKp", setHeatPIDKp);
    Particle.function("setHeatPIDKd", setHeatPIDKd);
    Particle.function("startControl", startControl);
    Particle.function("stopControl", stopControl);
    Particle.function("saveState", saveState);
    Particle.function("getDevices", getOneWireDevices);
    Particle.function("reinitLCD", reinitLCD);
    double* peakEstimator = getPeakEstimatorAddr();
    Particle.variable("peakEstim", *peakEstimator);
    Particle.variable("beerTemp", Input);
    Particle.variable("fridgeTemp", fridgeTemp);
    Particle.variable("pidKp", Kp);
    Particle.variable("pidKi", Ki);
    Particle.variable("pidKd", Kd);
    Particle.variable("heatPidKp", heatKp);
    Particle.variable("heatPidKi", heatKi);
    Particle.variable("heatPidKd", heatKd);
}

bool matchAddress(byte addressList[][8], byte address[8], size_t listSize)
{
    for(int i = 0; i < listSize; i++)
    {
        if( addressList[i][0] == address[0] &&
            addressList[i][1] == address[1] &&
            addressList[i][2] == address[2] &&
            addressList[i][3] == address[3] &&
            addressList[i][4] == address[4] &&
            addressList[i][5] == address[5] &&
            addressList[i][6] == address[6] &&
            addressList[i][7] == address[7])
            return true;
    }

    return false;
}

void setup()
{
#ifdef PROBES_HERMS
    inHermsMode = true;
#endif

    Blynk.config(auth);

    //initLCD();
    initUbidots();

    theApp::drawUsrScreen("Initializing...\n");
    delay(1000);
    wd.checkin();

    // calibration - probe1 { 0x28, 0x31, 0x9B, 0xEC, 0x07, 0x00, 0x00, 0x22 } - 30, 30, 70, 69.75
    // calibration - probe 2 { 0x28, 0x49, 0x99, 0xEC, 0x07, 0x00, 0x00, 0x98 } - 30, 29.94, 70, 69.62
    // probe 3 (with adapter) { 0x28, 0xFF, 0x53, 0x7D, 0x83, 0x16, 0x03, 0x84 } - 30, 29.81, 70, 69.81
    //fridge.setCalibration(30, 29.94, 70, 69.75); // OK
    ///////////////////beer.setCalibration(30, 29.94, 70, 69.62); - probe not used
    //beer.setCalibration(30, 29.81, 70, 69.81); // need to determine calibration values
    /*tempProbe = new probe(&oneWire2);
    byte *address;
    String addressString;
    address = tempProbe->getDeviceAddress();

    addressString = "";
    for(int i = 0; i < 8; i++)
    {
        addressString += "0x" + String(address[i], HEX);
        if(i < 7)
        addressString += ",";

    }
    printLineToLCD(0, addressString);

    return;*/

    for(int i = 0; i < 2; i++)
    {
        tempProbe = new probe(&oneWire2);
        //printAddress(i, tempProbe->getDeviceAddress());
        //delay(2000);

        if(matchAddress(oneWireDevices1, tempProbe->getDeviceAddress(), sizeof(oneWireDevices1) / sizeof(*oneWireDevices1)))
        {
            theApp::drawUsrScreen("Initializing...\nFridge matched\n");
            delay(500);
            fridge = tempProbe;
        }
        else if(matchAddress(oneWireDevices2, tempProbe->getDeviceAddress(), sizeof(oneWireDevices2) / sizeof(*oneWireDevices2)))
        {
            theApp::drawUsrScreen("Initializing...\nBeer matched\n");
            delay(500);
            beer = tempProbe;
        }
    }

    if(fridge == NULL || beer == NULL)
    {
        theApp::drawUsrScreen("Initializing...\nOneWire failure\n");
        Particle.function("getDevices", getOneWireDevices);
        setup_failed = true;
        return;
    }

    theApp::drawUsrScreen("Initializing...\nFridge pr. init\n");
    if(fridge->init() == false)
    {
        theApp::drawUsrScreen("Initializing...\nF.sensor failed\n");
        Particle.function("getDevices", getOneWireDevices);
        setup_failed = true;
        return;
    }
    theApp::drawUsrScreen("Initializing...\nBeer pr. init\n");
    if(beer->init() == false)
    {
        theApp::drawUsrScreen("Initializing...\nB.sensor failed\n");
        Particle.function("getDevices", getOneWireDevices);
        setup_failed = true;
        return;
    }

    probe::startConv();
    delay(1000);
    wd.checkin();
    // uncomment the line bellow to get temperature probe adreses printed to serial
    //discoverOneWireDevices();
    theApp::drawUsrScreen("Initializing...\nReading EEPROM\n");
    byte ver;
    EEPROM.get(0, ver);  // first byte of EEPROM stores a version # for tracking stored settings

    if (ver != EEPROM_VER) {  // EEPROM version number != hard coded version number
      EEPROMWritePresets();  // if version # is outdated, write presets
    }
    EEPROMReadSettings();    // load program settings from EEPROM*/
    delay(1000);
    wd.checkin();
    //wdt_enable(WDTO_8S);  // enable watchdog timer with 8 second timeout (max setting)
                        // wdt will reset the arduino if there is an infinite loop or other hangup; this is a failsafe device
    theApp::drawUsrScreen("Initializing...\nReg. functions\n");
    registerToCloud();
    delay(1000);
    wd.checkin();
    //publishFridgeState();
    theApp::drawUsrScreen("Initializing...\nConn. to cloud\n");
    Particle.connect();

    delay(1000);
    wd.checkin();
    theApp::drawUsrScreen("Initializing...\nSetup complete\n");
}

bool connectToBlynk()
{
    unsigned long now = millis();
    if(WiFi.ready() && (blynkConnectTimestamp == 0UL || (now - blynkConnectTimestamp) > BLYNK_CONNECT_RETRY))
    {
        blynkConnectTimestamp = now;
        return Blynk.connect();

    }
    return false;
}

void loop()
{
  if(Blynk.connected() || connectToBlynk())
  {
    Blynk.run();
  }

  if(setup_failed)
  {

      Particle.connect();
      if(discoverDevicesFlg)
      {
        discoverDevices();
      }
      delay(1000);
      return;
  }

  //progressIndicator();
  //printLineToLCD(1, 15, "1");
  //delay(100);
  if(disableControl)
  {
      //printLineToLCD(1, 15, "2");
      //delay(100);
      disableControl = gotoIdle();
  }

  //printLineToLCD(1, 15, "3");
  //delay(100);
  if (probe::isReady() && sensor_failure == false)
  {
    //printLineToLCD(1, 15, "4");
    //delay(100);
    //check for devicesOnBus
    int checkRetries = 0;
    while(oneWire2.reset() == 0)
    {
        if(checkRetries >= 10)
        {
            Blynk.notify("Shaolin_Brewer 1-Wire bus failure!");
            disableControl = true;
            sensor_failure = true;
            return;
        }
        checkRetries++;
        delay(1000);

    }
    // update sensors when conversion complete
    if( fridge->update() == false ||
        beer->update() == false)
    {
        //printLineToLCD(1, 15, "5");
        //delay(100);
        probe::startConv();
        return;
    }

    //printLineToLCD(1, 15, "6");
    delay(100);
    //
    if(discoverDevicesFlg)
    {
        discoverDevices();
    }

    if(inHermsMode == false)
    {
        //printTemperature(fridge->getFilter(), 0);
        //printTemperature(beer->getFilter(), 1);
        Input = beer->getFilter();
        fridgeTemp = fridge->getFilter();
        if(firstrun == true)
        {
            //todo: call only after Input is firt read from sensors
            initPIDs();
            firstrun = false;
        }
    }
    else
    {
        //printTemperature(fridge->getTemp(), 0);
        //printTemperature(beer->getTemp(), 1);
    }

    probe::startConv();
  }

  if(inHermsMode == false)
  {
      if(controledFermentation && firstrun == false)
      {
          mainPID.Compute();    // update main PID
          updateFridge();

          //printPIDOutputPercent(heatOutput);
          //printPIDOutputTemp(Output);

          //printFridgeState(getFridgeState(0));
      }
  }
  publishStatus();

  theApp::draw();

  delay(100);

}
