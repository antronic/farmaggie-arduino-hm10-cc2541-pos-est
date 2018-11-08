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

  // String device_address = "";
  // String mac = "";
  // int rssi = -10000;

  if (MODULE_STATUS == REST) {
    // call command for SEARCHING
    // Serial.println("Resting...");
    ss->print("AT+DISI?");
    // sets to SEARCHING status
    MODULE_STATUS = SEARCHING;
    // reset values
    // device_address = "";
    // rssi = -1000;

    return;
  }

  if (MODULE_STATUS == SEARCHING) {
    while (ss->available() > 0) {
      String res = ss->readString();
      // If response starts with OK+DISIS
      // that means module works
      // then we remove OK+DISIS from response
      // for next identify
      res.replace("OK+DISCE", "");
      res.replace("OK+DISIS", "");
      // res.replace("OK+DIS+", "");

      String response_temp = "";

      // Serial.println(res);
      // if (res.length() >= 78) {
      //   response_temp = res.substring(0, 100);
      // }

      // If response has "OK+DISC"
      // that means module found some devices
      // and the response per devices will at least 78 characters
      // if (response.indexOf("OK+DISC") >= 0) {

      // }

      // Serial.println(response_temp);

      if (res.length() >= 78) {
        MSG msg;
        msg.msg = res;

        Serial.print("{ \"device\": \"");
        Serial.print(module_name);
        Serial.print("\", ");
        Serial.print("\"msg\": \"");
        delay(1000);
        // delay for send the message
        Serial.print(res);
        Serial.println("\"} ");
        // eventCallback(msg);
      }
      MODULE_STATUS = REST;
      return;

      // BLOCK FROM HERE!!!!!!!!!!!!!!!!

      int count = 1;
      while (response_temp.startsWith("OK+DISC") && (response_temp.indexOf("OK+DISC") >= 0 && response_temp.length() >= 78) > 0) {
      // while (response_temp.indexOf("OK+DISC") >= 0 && response_temp.length() >= 78) {
        Serial.println(res);
        String device_address = str_token(response_temp, ':', 2);
        String hilo = str_token(response_temp, ':', 3);
        String mac = str_token(response_temp, ':', 4);
        String rssi_str = str_token(response_temp, ':', 5);
        int rssi = rssi_str.toInt();

        // Serial.println(response_temp);
        // eventCallback(response_temp);
        // Beacon will not has Mac Address as "0000000000"
        if (mac != "0000000000" && device_address != "00000000000000000000000000000000") {
          DEVICE device;
          device.address = device_address;
          device.mac = mac;
          device.rssi = rssi;
          device.res = res;

          EVENT event;
          event.event_name = DEVICE_FOUND;
          event.device = device;
          // eventCallback(event);
          String json = "{";
          json += "\"device\": \"";
          json += module_name;
          json += "\", ";
          json += "\"address\": \"";
          json += event.device.address;
          json += "\", ";
          json += "\"mac\": \"";
          json += event.device.mac;
          json += "\", ";
          json += "\"rssi\": \"";
          json += event.device.rssi;
          json += "\"";
          json += "}";
          Serial.println(json);
        }

        // response_temp.replace("OK+DISC", "");
        // if (res.length() >= (78 * (count + 1))) {
        //   response_temp = res.substring(78 * count, 78 * (count + 1));
        // } else {
        //   response_temp = res.substring(78 * count, res.length() - 1);
        //   MODULE_STATUS = REST;
        // }
        // ++count;
        res = res.substring(78);
        if (res.indexOf("OK+DISCE") >= 0 || res.length() < 78) {
          MODULE_STATUS = REST;
          return;
        }
      }

      // End of loop        // OK+DISCE will appear when meet at the end of response
      // if (response.indexOf("OK+DISCE") >= 0 || response.length() < 78)

      if (response_temp.length() < 78)
      {
        MODULE_STATUS = REST;
        return;
      }
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