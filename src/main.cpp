#include <Arduino.h>
#include "config.h"
#if defined(ESP32)
  #include <WiFiMulti.h>
  WiFiMulti wifiMulti;
  #define DEVICE "ESP32"
  #elif defined(ESP8266)
  #include <ESP8266WiFiMulti.h>
  ESP8266WiFiMulti wifiMulti;
  #define DEVICE "ESP8266"
  #endif
  
  #include <InfluxDbClient.h>
  #include <InfluxDbCloud.h>
  

  
  // Time zone info
  #define TZ_INFO "UTC2"
  #define gpio2_digit 2
  #define gpio4_digit 4
  #define gpio2_analog 2
  #define gpio4_analog 4
  #define gpio16_digit 16
  #define gpio17_digit 17
  #define gpio6_digit 6
  #define gpio18_digit 18
  #define gpio19_digit 19
  #define gpio21_digit 21
  #define gpio3_digit 3
  #define gpio1_digit 1
  #define gpio13_digit 13
  #define gpio12_digit 12
  #define gpio14_digit 14
  #define gpio27_digit 27
  #define gpio26_digit 26
  #define gpio25_digit 25
  #define gpio33_digit 33
  #define gpio32_digit 32
  #define gpio35_digit 35
  #define gpio34_digit 34
  #define gpio39_digit 39
  #define gpio36_digit 36

String influxQueryURL = String(INFLUXDB_URL) + "/api/v2/query?org=" + INFLUXDB_ORG;
String influxQuery =
  "from(bucket: \"" + String(INFLUXDB_BUCKET) + "\")"
  " |> range(start: -1h)"
  " |> filter(fn: (r) => r._measurement == \"decision\")"
  " |> filter(fn: (r) => r._field == \"g2_digit_decision\")"
  " |> last()";

String parseDecision(const String& lastLine) {
  int commaCount = 0;
  int startIndex = 0;

  for (int i = 0; i < lastLine.length(); i++) {
    if (lastLine.charAt(i) == ',') {
      commaCount++;
      if (commaCount == 6) {
        startIndex = i + 1;
        int endIndex = lastLine.indexOf(',', startIndex);
        if (endIndex != -1) {
          return lastLine.substring(startIndex, endIndex);
        }
      }
    }
  }

  return "";
}

  // Declare InfluxDB client instance with preconfigured InfluxCloud certificate
  InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
  
  // Declare Data point
  Point sensor("wifi_status");
  Point ai_controls("ai_gpios");
  Point decision("decision");

  int gpios_state;

  
  void setup() {
    Serial.begin(9600);
  
    // Setup wifi
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  
    Serial.print("Connecting to wifi");
    while (wifiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      delay(100);
    }
    Serial.println();
  
    // Accurate time is necessary for certificate validation and writing in batches
    // We use the NTP servers in your area as provided by: https://www.pool.ntp.org/zone/
    // Syncing progress and the time will be printed to Serial.
    timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
  
  
    // Check server connection
    if (client.validateConnection()) {
      Serial.print("Connected to InfluxDB: ");
      Serial.println(client.getServerUrl());
    } else {
      Serial.print("InfluxDB connection failed: ");
      Serial.println(client.getLastErrorMessage());
    }

  
    // ... code in setup() from Initialize Client
   
    // Add tags to the data point
    sensor.addTag("device", DEVICE);
    sensor.addTag("SSID", WiFi.SSID());
   
  }



    void loop() {
    // Clear fields for reusing the point. Tags will remain the same as set above.
    sensor.clearFields();
  
    // Store measured value into point
    // Report RSSI of currently connected network
    sensor.addField("rssi", WiFi.RSSI());
 
    int g2_digit=digitalRead(gpio2_digit);
    int g4_digit=digitalRead(gpio4_digit);
    int g16_digit=digitalRead(gpio16_digit);
    int g17_digit=digitalRead(gpio17_digit);
    int g6_digit=digitalRead(gpio6_digit);
    int g18_digit=digitalRead(gpio18_digit);
    int g19_digit=digitalRead(gpio19_digit);

      


    
     // Add readings as fields to point

  ai_controls.addField("g2_digit", g2_digit); 
  ai_controls.addField("g4_digit", g4_digit); 
  ai_controls.addField("g16_digit", g16_digit); 
   ai_controls.addField("g6_digit", g6_digit); 
  ai_controls.addField("g18_digit", g18_digit); 
   ai_controls.addField("g19_digit", g19_digit); 

/*
    int g2_digit_decision;
    int g4_digit_decision;
    int g16_digit_decision;
    int g17_digit_decision;
    int g6_digit_decision;
    int g18_digit_decision;
    int g19_digit_decision;
    decision.addField("g2_digit_decision",g2_digit_decision);
    decision.addField("g4_digit_decision",g4_digit_decision);
    decision.addField("g16_digit_decision",g16_digit_decision);
    decision.addField("g17_digit_decision",g17_digit_decision);
    decision.addField("g6_digit_decision",g6_digit_decision);
    decision.addField("g18_digit_decision",g18_digit_decision);
    decision.addField("g19_digit_decision",g19_digit_decision);*/
    




  Serial.print("Writing: ");
  Serial.println(client.pointToLineProtocol(ai_controls));
  Serial.println(client.pointToLineProtocol(decision));
  
  // Write point into buffer
  client.writePoint(ai_controls);
   client.writePoint(decision);

  // Clear fields for next usage. Tags remain the same.
  ai_controls.clearFields();
  decision.clearFields();

  
    // Print what are we exactly writing
    Serial.print("Writing: ");
    Serial.println(sensor.toLineProtocol());
  
    // Check WiFi connection and reconnect if needed
    if (wifiMulti.run() != WL_CONNECTED) {
      Serial.println("Wifi connection lost");
    }
  
    // Write point
    if (!client.writePoint(sensor)) {
      Serial.print("InfluxDB write failed: ");
      Serial.println(client.getLastErrorMessage());
    }
  
    /*digitalWrite(gpio2_digit,g2_digit);
     digitalWrite(gpio4_digit,g4_digit);
    digitalWrite(gpio16_digit,g16_digit);
    digitalWrite(gpio17_digit,g17_digit);
    digitalWrite(gpio6_digit,g6_digit);
    digitalWrite(gpio18_digit,g18_digit);
    digitalWrite(gpio19_digit,g19_digit);*/




    Serial.println("Waiting 1 second");

    delay(1000);
   if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(influxQueryURL);
    http.addHeader("Authorization", "Token " + String(INFLUXDB_TOKEN));
    http.addHeader("Content-Type", "application/vnd.flux");

    int httpResponseCode = http.POST(influxQuery);

    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("Odpowiedź InfluxDB:");
      Serial.println(payload);

      int lastIndex = payload.lastIndexOf("\n");
      while (lastIndex > 0) {
        String lastLine = payload.substring(lastIndex + 1);
        lastLine.trim();

        if (lastLine.length() > 0) {
          Serial.println("Ostatnia linia: " + lastLine);
          String decision = parseDecision(lastLine);
          Serial.print("Odczytana decyzja: ");
          Serial.println(decision);


          }

        payload = payload.substring(0, lastIndex);
        lastIndex = payload.lastIndexOf("\n");
      }

      if (lastIndex <= 0) {
        Serial.println("Nie znaleziono danych w odpowiedzi.");
      }
    } else {
      Serial.print("Błąd HTTP: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }

  delay(10000); // odczyt co 10 sekund
}

    
  