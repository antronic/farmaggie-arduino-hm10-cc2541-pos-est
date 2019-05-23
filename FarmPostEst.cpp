#include "FarmPosEst.h"

String str_token(String data, char separator, int index);
EVENT deviceNotFound();

FarmPosEst::FarmPosEst(String moduleName, String moduleType, SoftwareSerial *softwareSerial, EventCallback eventHandler)
{
  module_name = moduleName;
  module_type = moduleType;

  ss = softwareSerial;
  eventCallback = eventHandler;
}

void FarmPosEst::begin() {
  ss->begin(9600);

  ss->print("AT");
  delay(100);

  while (ss->available())
  {
    String res = ss->readString();
    Serial.println(res);
  }

  // sets to Master/Central mode
  ss->print("AT+ROLE1");
  delay(100);
  while (ss->available())
  {
    String res = ss->readString();
    Serial.println(res);
  }

  // sets notify mode
  ss->print("AT+NOTI1");
  delay(100);
  while (ss->available())
  {
    String res = ss->readString();
    Serial.println(res);
  }

  // sets max power
  ss->print("AT+POWE3");
  delay(100);
  while (ss->available())
  {
    String res = ss->readString();
    Serial.println(res);
  }

  // sets manual connection mode
  ss->print("AT+IMME1");
  delay(100);
  while (ss->available())
  {
    String res = ss->readString();
    Serial.println(res);
  }

  // Reset(some kind like restart) module
  ss->print("AT+RESET");
  delay(2000);
  while (ss->available())
  {
    String res = ss->readString();
    Serial.println(res);
  }
}

// update function using for scan
void FarmPosEst::update() {
  static int MODULE_STATUS = REST;
  static String res_tmp = "";

  // String device_address = "";
  // String mac = "";
  // int rssi = -10000;

  if (MODULE_STATUS == REST) {
    // call command for SEARCHING
    delay(500);
    ss->print("AT+DISC?");
    MODULE_STATUS = SEARCHING;
    return;
  }

  /*
    EXAMPLE RESPONSE
OK+DISCSOK+DIS0:F4844C4DC2A9OK+RSSI:-042
OK+NAME:HMSensor
OK+DISCE
  */

  if (MODULE_STATUS == SEARCHING) {
    static int SCAN_STATUS = SCAN_END;
    while (ss->available() > 0) {
      String res = ss->readString();
      // Serial.println(res);

      // New incoming is starting
      if (res.startsWith("OK+DISCS")) {
        SCAN_STATUS = SCAN_IN_PROGRESS;
        res.replace("OK+DISCS", "");
        /* SCAN STARTS */
      }
      if (res.endsWith("OK+DISCE")) {
        SCAN_STATUS = SCAN_ENDING;
        res.replace("OK+DISCE", "");
        /* SCAN IS ENDING */
      }

      if (SCAN_STATUS == SCAN_IN_PROGRESS || SCAN_STATUS == SCAN_ENDING) {
        res_tmp += res;
      }

      if (SCAN_STATUS == SCAN_ENDING) {
        SCAN_STATUS = SCAN_END;

        // Serial.println(res_tmp);
        res_tmp.replace("\r", "#NEWLINE#");
        res_tmp.replace("\n", "");

        Serial.print("{");
        Serial.print("\"device\":\"");
        Serial.print(module_name);
        Serial.print("\",");

        Serial.print("\"type\":\"");
        Serial.print(module_type);
        Serial.print("\",");

        Serial.print("\"msg\":\"");
        Serial.print(res_tmp);
        Serial.print("\"}");

        Serial.println();

        MODULE_STATUS = REST;
        res_tmp = "";
        /* SCAN END */
      }

      // Serial.println("RES END");
      return;
    }
  }
}

EVENT FarmPosEst::deviceNotFound() {
  EVENT not_found_event;
  DEVICE empty_device;

  empty_device.address = "";
  empty_device.mac = "";
  empty_device.rssi = -100000;

  not_found_event.event_name = DEVICE_NONE;
  not_found_event.device = empty_device;

  return not_found_event;
}

String FarmPosEst::getModuleName() {
  return module_name;
}

String FarmPosEst::getModuleType() {
  return module_type;
}

/**
 * Ref. https://github.com/dinosd/BLE_PROXIMITY
 */
String str_token(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}