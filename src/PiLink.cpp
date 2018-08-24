#include "PiLink.h"

bool PiLink::firstPair;
char PiLink::printfBuff[PRINTF_BUFFER_SIZE];
TCPServer PiLink::server(23);
TCPClient PiLink::piStream;

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
