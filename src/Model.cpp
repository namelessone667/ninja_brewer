#include "Model.h"
#include "EEPROM_MAP.h"

const int EEPROM_VER = 2;

Model::Model()
{
  _appState = defaultAppState;
  _appConfig = defaultAppConfig;
}

void Model::loadAppConfigFromEEPROM()
{
  byte eeprom_ver = -1;
  EEPROM.get(APP_VERSION_ADDR, eeprom_ver);

  if(eeprom_ver == EEPROM_VER)
    EEPROM.get(APP_CONFIG_ADDR, _appConfig);
  else
    _appConfig = defaultAppConfig;
}

void Model::saveAppConfigToEEPROM()
{
  byte eeprom_ver = EEPROM_VER;
  EEPROM.put(APP_VERSION_ADDR, eeprom_ver);
  EEPROM.put(APP_CONFIG_ADDR, _appConfig);
}

AppState Model::getApplicationState()
{
  return _appState;
}

AppConfig Model::getApplicationConfig()
{
  return _appConfig;
}

void Model::setApplicationState(ApplicationState newAppState)
{
  _appState.app_state = newAppState;
}

void Model::setApplicationConfig(AppConfig newAppConfig)
{
  _appConfig = newAppConfig;
}
