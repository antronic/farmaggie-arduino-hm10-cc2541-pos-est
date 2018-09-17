#include "FarmPosEst.h"

String str_token(String data, char separator, int index);
EVENT deviceNotFound();

FarmPosEst::FarmPosEst(String name, SoftwareSerial *softwareSerial, EventCallback eventHandler)
{
  module_name = name;
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
  static String response;
  static int devices_found;
  static String device_address = "";
  static String mac = "";
  static int rssi = 0;

  if (MODULE_STATUS == REST) {
    // call command for SEARCHING
    Serial.println("Resting...");
    ss->print("AT+DISI?");
    // sets to SEARCHING status
    MODULE_STATUS = SEARCHING;
    // reset values
    device_address = "";
    response = "";
    devices_found = 0;
    rssi = 0;

    return;
  }

  if (MODULE_STATUS == SEARCHING) {

    while (ss->available()) {
      String res = ss->readString();
      // If response starts with OK+DISIS
      // that means module works
      // then we remove OK+DISIS from response
      // for next identify

      if (res.startsWith("OK+DISIS")) {
        res.replace("OK+DISIS", "");
      }

      response += res;
      Serial.println("res -> ");
      Serial.println(res);
      // If response has "OK+DISC"
      // that means module found some devices
      // and the response per devices will at least 78 characters
      while (response.indexOf("OK+DISC") >= 0 && response.length() >= 78)
      {
        Serial.println("in loop");
        String dd = response.substring(0, 78);
        // Deqeue current device from response
        response = response.substring(78);

        device_address = str_token(dd, ':', 2);
        // String hilo = str_token(device, ':', 3);
        mac = str_token(dd, ':', 4);
        String rssi_str = str_token(dd, ':', 5);
        rssi = rssi_str.toInt();

        // Beacon will not has Mac Address as "0000000000"
        if (mac != "0000000000")
        {
          devices_found++;
        }

        if (devices_found > 0) {
          DEVICE device;
          device.address = device_address;
          device.mac = mac;
          device.rssi = rssi;

          EVENT event;
          event.event_name = DEVICE_FOUND;
          event.device = device;
          eventCallback(event);
        }
        else {
          DEVICE device;
          device.address = "";
          device.mac = "";
          device.rssi = -10000;

          EVENT event;
          event.event_name = DEVICE_NONE;
          event.device = device;
          eventCallback(event);
        }
      }

      // OK+DISCE will appear when meet at the end of response
      if (response.indexOf("OK+DISCE") >= 0) {
        MODULE_STATUS = DONE_CHECK;
        return;
      }
    }
  }

  if (MODULE_STATUS == DONE_CHECK) {
    MODULE_STATUS = REST;
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