#include "PiLink.h"

bool PiLink::firstPair;
char PiLink::printfBuff[PRINTF_BUFFER_SIZE];
TCPServer PiLink::server(23);
TCPClient PiLink::piStream;
