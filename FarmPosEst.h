/**
 * Inspire pattern and logic from https://github.com/dinosd/BLE_PROXIMITY
 */

#include "Arduino.h"
#include "SoftwareSerial.h"

#ifndef FarmPosEst_h
#define FarmPosEst_h
// Define event for devices
#define DEVICE_NONE 1
#define DEVICE_FOUND 2
#define DEVICE_MOVED 3

#define REST 100
#define SEARCHING 11

#define SCAN_IN_PROGRESS 12
#define SCAN_ENDING 13
#define SCAN_END 14

#define CHECK_RESULT 22
#define DONE_CHECK 33

struct DEVICE {
  String address;
  String mac;
  String res;
  int rssi;

  // uint8_t hi;
  // uint8_t lo;
  // String hilo;
};

struct EVENT {
  uint8_t event_name;
  DEVICE device;
};

struct MSG {
  String msg;
};

// typedef void (*EventCallback)(EVENT event);
typedef void (*EventCallback)(MSG msg);

class FarmPosEst{
public:
  FarmPosEst(String moduleName, String moduleType, SoftwareSerial *ss, EventCallback eventCallback);

  void begin();
  void update();

  String getModuleName();
  String getModuleType();

  String status;

private:
  SoftwareSerial *ss;
  EventCallback eventCallback;

  String module_name;
  String module_type;
  int moduleId;

  EVENT deviceNotFound();
};
#endif