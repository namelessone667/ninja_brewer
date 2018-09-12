#include "PiLink.h"

  PiLink::PiLink() :
    model(theApp::getInstance().getModel()),
    loggerref(theApp::getInstance().getLogger()),
    server(23)
  {

  }

  PiLink& PiLink::getInstance()
  {
    static PiLink instance; // Guaranteed to be destroyed.
                            // Instantiated on first use.
    return instance;
  }

	void PiLink::init(void)
  {
#ifdef PILINK_SERIAL
    piStream.begin(57600);
#else
		server.begin();
		logger().info("TCPServer running on port 23");
#endif
  }

	void PiLink::receive(void)
  {
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
								sendControlSettings();	// update script with new settings
								sendControlConstants();
								break;
            case 'l': // Display content requested
							printLCDtext();
         			break;
          }
      }
  }

  void PiLink::print(const char *fmt, ... )
  {
  	va_list args;
  	va_start (args, fmt );
  	vsnprintf(printfBuff, PRINTF_BUFFER_SIZE, fmt, args);
  	va_end (args);

  	piStream.print(printfBuff);

    /*if(piStream.status())
      piStream.write((const uint8_t *)printfBuff, strlen(printfBuff), TCP_TIMEOUT);*/
  }

  void PiLink::print(char c)
  {
    piStream.print(c);

    /*if(piStream.status())
      piStream.write(c, TCP_TIMEOUT);*/
  }

  void PiLink::printNewLine()
  {

    /*if(piStream.status())
    {
      piStream.write('\r', TCP_TIMEOUT);
      piStream.write('\n', TCP_TIMEOUT);
    }*/
    piStream.println();
	}

	// Send settings as JSON string
	void PiLink::sendControlSettings(void)
	{
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
	}

	void PiLink::sendControlConstants(void)
	{
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
	}

	void PiLink::sendControlVariables(void)
	{
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
	}

  void PiLink::printTemperatures(void)
  {
	// print all temperatures with empty annotations
	printTemperaturesJSON(0, 0);
  }


	void PiLink::printTemperaturesJSON(const char * beerAnnotation, const char * fridgeAnnotation)
  {
  	printResponse('T');

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
  }

  void PiLink::printResponse(char type)
  {
	   print("%c:", type);
	   firstPair = true;
  }

  void PiLink::openListResponse(char type)
  {
	   printResponse(type);
	   print('[');
  }

  void PiLink::closeListResponse()
  {
	   print(']');
	   printNewLine();
  }

  void PiLink::sendJsonTemp(const char* name, double temp)
  {
  	char tempString[9];
    snprintf(tempString, 9, "%.3f", temp);
  	printJsonName(name);
  	print(tempString);
  }

	void PiLink::sendJsonPair(const char * name, double val)
  {
	   printJsonName(name);
	   print("%f",val);
  }

  void PiLink::sendJsonPair(const char * name, const char * val)
  {
	   printJsonName(name);
	   print(val);
  }

  void PiLink::sendJsonPair(const char * name, char val)
  {
  	printJsonName(name);
  	print('"');
  	print(val);
  	print('"');
  }

  void PiLink::sendJsonPair(const char * name, uint16_t val)
  {
	   printJsonName(name);
	   print("%u", val);
  }

	void PiLink::sendJsonPair(const char * name, int val)
  {
	   printJsonName(name);
	   print("%d", val);
  }

  void PiLink::sendJsonPair(const char * name, uint8_t val)
  {
	   sendJsonPair(name, (uint16_t)val);
  }

  void PiLink::printJsonName(const char * name)
  {
  	printJsonSeparator();
  	print('"');
  	print(name);
  	print("\":");
  }

  void PiLink::printJsonSeparator()
  {
	   print(firstPair ? '{' : ',');
	   firstPair = false;
  }

  void PiLink::sendJsonClose(bool newline)
  {
	   print("}");
		 if(newline)
	   	printNewLine();
  }

  void PiLink::sendJsonAnnotation(const char* name, const char* annotation)
  {
	   printJsonName(name);
	   const char* fmtAnn = annotation ? "\"%s\"" : "null";
	   print(fmtAnn, annotation);
  }

	const Logger& PiLink::logger()
	{
		return loggerref;
	}

	char* PiLink::fixedPointToString(char * s, double value, uint8_t numDecimals, uint8_t maxLength){
		snprintf(s, maxLength, "%.*f", numDecimals, value);
		return s;
	}

	void PiLink::printLCDtext()
	{
		openListResponse('L');

		bool heatingEnabled = (model.ControllerMode.Get() == HEATER_ONLY || model.ControllerMode.Get() == COOLER_HEATER);

		char stringBuffer[21];
	  String::format("F:%4.1fC  SET:%4.1fC", model.FridgeTemp.Get(), model.SetPoint.Get()).toCharArray(stringBuffer, 20);
		print("\"%s\",", stringBuffer);

	  String::format("B:%4.1fC  PID:%4.1fC", model.BeerTemp.Get(), model.Output.Get()).toCharArray(stringBuffer, 20);
	  print("\"%s\",", stringBuffer);

		if(heatingEnabled)
	  	String::format("PID:%s  HPID:%s", model.PIDMode == PID_MANUAL ? "MAN " : "AUTO", model.HeatPIDMode == PID_MANUAL ? "MAN " : "AUTO").toCharArray(stringBuffer, 20);
		else
			String::format("PID:%s", model.PIDMode == PID_MANUAL ? "MAN " : "AUTO").toCharArray(stringBuffer, 20);
	  print("\"%s\",", stringBuffer);

		String text;

		if(model.StandBy.Get())
		{
					String::format("Stand By").toCharArray(stringBuffer, 20);
		}
		else
		{
			switch(model.ControllerState)
		  {
		    case IDLE:
		      String::format("Idle...").toCharArray(stringBuffer, 20);
		      break;
		    case COOL:
		      String::format("Cooling...").toCharArray(stringBuffer, 20);
		      break;
		    case HEAT:
		      String::format("Heating... %4.1f%%", model.HeatOutput.Get()).toCharArray(stringBuffer, 20);
		      break;
		    default:
		      String::format("unknown state").toCharArray(stringBuffer, 20);
		      break;
		  }
		}
	  print("\"%s\"]", stringBuffer);

		printNewLine();
	}

	void PiLink::receiveJson(void)
	{
		parseJson(&PiLink::processJsonPair, NULL);
		return;
	}

	void PiLink::parseJson(ParseJsonCallback fn, void* data)
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
				(this->*fn)(key, val, data);
		} while (next);
	}

	bool PiLink::parseJsonToken(char* val)
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
	}

	void PiLink::processJsonPair(const char * key, const char * val, void* pv)
	{
		logger().info("Json received: %s = %s", key, val);
		theApp& app = theApp::getInstance();

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
	}

	template<typename T>
	void PiLink::setProperty(Property<T>& property, const char* strVal)
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
	}

	void PiLink::setMode(char mode)
	{
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
				//model.PIDMode = PID_AUTOMATIC;
        model.PIDMode = PID_MANUAL; //for now control only the fridge temp when in temp. profile mode
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
	}

	void PiLink::setBeerSetting(const char* val)
	{
		double setpoint = strtod(val, NULL);
		if(setpoint == 0.0)
		{
			logger().error("setBeerSettings: failed to convert input argument %s to double", val);
			return;
		}
		logger().info("setBeerSettings: setting new SetPoint to %f", setpoint);
		theApp::getInstance().setNewTargetTemp(setpoint);
	}

	void PiLink::setFridgeSetting(const char* val)
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
	}

	void PiLink::listDevices()
	{
		int id = 0;

		printDevice(id++, DEVICE_TYPE_SENSOR, 1, 0, DEVICE_CHAMBER_TEMP, DEVICE_HARDWARE_ONEWIRE_TEMP, 0, ONE_WIRE_BUS_PIN, theApp::getInstance().GetSensor1Address(), true);
		print(',');
		printDevice(id++, DEVICE_TYPE_SENSOR, 0, 1, DEVICE_BEER_TEMP, DEVICE_HARDWARE_ONEWIRE_TEMP, 0, ONE_WIRE_BUS_PIN, theApp::getInstance().GetSensor2Address());
		print(',');
		printDevice(id++, DEVICE_TYPE_ACTUATOR, 1, 0, DEVICE_CHAMBER_HEAT, DEVICE_HARDWARE_PIN, 0, HEATER_SSR_PIN, NULL);
		print(',');
		printDevice(id++, DEVICE_TYPE_ACTUATOR, 1, 0, DEVICE_CHAMBER_COOL, DEVICE_HARDWARE_PIN, 0, COOLER_SSR_PIN, NULL);
	}

	void PiLink::printDevice(int id, int type, int chamber, int beer, int function, int hw, int deact, int pin, const char* address, bool first)
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
	}

	int PiLink::read(int retries)
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
  }

	bool PiLink::available()
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
	}

template<>
bool PiLink::convertCharToVal<double>(const char* strVal, double& value)
{
  if(strcmp(strVal, "0") == 0 || strcmp(strVal, "0.0") == 0)
  {
    value = 0;
    return true;
  }

  value = strtod(strVal, NULL);
  if(value == 0.0)
  {
    logger().error("convertCharToVal: failed to convert input argument %s to double", strVal);
    return false;
  }
  return true;
}

template<>
bool PiLink::convertCharToVal<int>(const char* strVal, int& value)
{
  if(strcmp(strVal, "0") == 0)
  {
    value = 0;
    return true;
  }

  value = strtol(strVal, NULL, 10);
  if(value == 0L)
  {
    logger().error("convertCharToVal: failed to convert input argument %s to int", strVal);
    return false;
  }
  return true;
}
