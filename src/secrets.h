#ifndef secrets_h
#define secrets_h

#ifdef HERMS_MODE
#define BLYNK_AUTH "48d82fb0e71047cda347285a2408369c" // Blynk authentication token
#else
#define BLYNK_AUTH "752f7de9ef0a417d80caa5aa0b87d5d8" // Blynk authentication token
//#define BLYNK_AUTH "5ca8835174204b3b9836c380b0ce48ed" // Blynk authentication token
#endif
#define UBIDOTS_TOKEN "A1E-jPfWoFnDeB4O12NA5RoZVRRoS4EDS0"  // Add here your Ubidots TOKEN

#define PAPERTAIL_PORT 10532//10532 //19915
#endif
