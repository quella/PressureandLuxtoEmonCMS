/*
The sketch reads in LUX and Pressure and adds that to feeds in EmonCMS for data collection.  Built on an
ESP8266 NodeMCU (12F) using the Arduino IDE.

*/

#include <Arduino.h>
#include <Wire.h>
#include <BH1750FVI.h>
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;
#include <ESP8266WiFi.h>

uint8_t ADDRESSPIN = 13;
BH1750FVI::eDeviceAddress_t DEVICEADDRESS = BH1750FVI::k_DevAddress_H;
BH1750FVI::eDeviceMode_t DEVICEMODE = BH1750FVI::k_DevModeContHighRes;

// Create the Lightsensor instance
BH1750FVI LightSensor(ADDRESSPIN, DEVICEADDRESS, DEVICEMODE);


const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

const char* host = "EMONCMS_IP_ADDRESS";

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

delay(5000);

  LightSensor.begin();  
  Serial.println("Light sensor Running...");

  if (!bmp.begin()) {
  Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  while (1) {}

  }
}

void loop() {
  
  float barometer;
  long ConvertedSolarRadiation;
  float SolarPercent;
   
  //
  //  Read and print the pressure info
  //
  barometer = bmp.readSealevelPressure()*0.0002952998751;
    Serial.print("Pressure Local: ");
    Serial.println(barometer);
    Serial.print("Pressure Sealevel = ");
    Serial.print(bmp.readSealevelPressure(432)*0.0002952998751);
    Serial.println(" Hg");

    uint16_t lux = LightSensor.GetLightIntensity();
    SolarPercent = ((lux * 100)/ 54612);
    ConvertedSolarRadiation = lux * 0.01674;
    Serial.printf("Light: %d lux", lux);
    Serial.println();
    
    Serial.print("Converted Solar Radiation: ");
    Serial.println(ConvertedSolarRadiation);
    Serial.println();
    
    Serial.print("Solar Percent: ");
    Serial.println(SolarPercent);
    Serial.println();

    int value = 0;

   ++value;

  Serial.print("connecting to ");
  Serial.println(host);

//
// Use WiFiClient class to create TCP connections
//
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

delay(5000);

//
// Build a URI for the request to the EmonCMS server
//
  
  String fronturl = "/emoncms/input/post";
  String midurl = "?node=Enviroment&fulljson={%22Pressure%22:";
  String afterurl = ",%20%22Light_Lux%22:";
  String endmidurl = ",%20%22Solar_Radiation%22:";
  String nexttoendurl = ",%20%22Solar_Percent%22:";
  String endurl = "}&apikey=APIKEY";
  String url = fronturl + midurl + barometer + afterurl + lux + endmidurl + ConvertedSolarRadiation + nexttoendurl + SolarPercent + endurl;


  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server

//   client.print(String("GET ") + url + "Connection: close\r\n\r\n");
               
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
              "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
}
