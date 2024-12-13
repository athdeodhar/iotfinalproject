#include <Arduino.h>
#include <WiFi.h>
#include <inttypes.h>
#include <stdio.h>
#include <ArduinoHttpClient.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <TinyGPS++.h>
#define RXD2 9  // 13
#define TXD2 10 // 15

#define GPS_BAUD 9600
#define loopDelay 50
#define BUTTON_PIN 22
#define BUTTON_REDUNDANT_PIN 34
int gpsCycle = 40;
int currCycle = 1;

int pressed = 0;
int counter = 0;
TinyGPSPlus gps;
int lastState = LOW;    // the previous state from the input pin
int currentState = LOW; // the current reading from the input pin

HardwareSerial gpsSerial(2);

char ssid[50]; // your network SSID (name)
char pass[50]; // your network password (use for WPA, or use
// as key for WEP)
// Name of the server we want to connect to
const char kHostname[] = "18.116.74.254";
// Path to download (this is the bit after the hostname in the URL
// that you want to download
// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;
/*
// *************************SETUP FOR INITIAL WIFI USERNAME / PASSWORD *******************************
void setup()
{
  Serial.begin(9600);
  delay(1000);
  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
  Serial.printf("\n");
  Serial.printf("Opening Non-Volatile Storage (NVS) handle... ");
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {
    Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  }
  else
  {
    Serial.printf("Done\n");
    // Write
    Serial.printf("Updating ssid/pass in NVS ... ");
    char ssid[] = "";    /////////////////////////////////////////////////////////////////////////
    char pass[] = ""; ///////////////////////////////////////////////////////////////////////////
    err = nvs_set_str(my_handle, "ssid", ssid);
    err |= nvs_set_str(my_handle, "pass", pass);
    Serial.printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

    Serial.printf("Committing updates in NVS ... ");
    err = nvs_commit(my_handle);
    Serial.printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
    // Close
    nvs_close(my_handle);
  }
}
*/

void nvs_access()
{
  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {

    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
  // Open
  Serial.printf("\n");
  Serial.printf("Opening Non-Volatile Storage (NVS) handle... ");
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {
    Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  }
  else
  {
    Serial.printf("Done\n");
    Serial.printf("Retrieving SSID/PASSWD\n");
    size_t ssid_len;
    size_t pass_len;
    err = nvs_get_str(my_handle, "ssid", ssid, &ssid_len);
    err |= nvs_get_str(my_handle, "pass", pass, &pass_len);
    switch (err)
    {
    case ESP_OK:
      Serial.printf("Done\n");
      break;
    case ESP_ERR_NVS_NOT_FOUND:
      Serial.printf("The value is not initialized yet!\n");
      break;
    default:
      Serial.printf("Error (%s) reading!\n", esp_err_to_name(err));
    }
  }
  // Close
  nvs_close(my_handle);
}

void setup()
{
  Serial.begin(9600);
  delay(300);
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial 2 started at 9600 baud rate");

  nvs_access();

  delay(1000);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());
}

void loop()
{

  delay(loopDelay);
  int err = 0;
  WiFiClient wifi;
  HttpClient client = HttpClient(wifi, kHostname, 5000);

  char gpsData = gpsSerial.read();
  // Serial.print(gpsData);
  if (currCycle % gpsCycle == 0)
  {
    currCycle = 1;
    while (gpsSerial.available() > 0)
    {
      gps.encode(gpsSerial.read());
    }
    if (gps.location.isUpdated())
    {
      Serial.print("LAT: ");
      Serial.println(gps.location.lat(), 6);
      Serial.print("LONG: ");
      Serial.println(gps.location.lng(), 6);
      Serial.print("SPEED (km/h) = ");
      Serial.println(gps.speed.kmph());
      Serial.print("HDOP = ");
      Serial.println(gps.hdop.value() / 100.0);
      Serial.print("Satellites = ");
      Serial.println(gps.satellites.value());
      Serial.println();
      /*   SEND GPS INFORMATION*/

      char buffer[50];
      dtostrf(gps.location.lat(), 0, 20, buffer);

      char buffer2[50];
      dtostrf(gps.location.lng(), 0, 20, buffer2);

      Serial.println("Sending gps data.................................");
      String inputString = "{\"lat\":\"" + String(buffer) + "\",\"lng\":\"" + String(buffer2) + "\",\"speed\":\"" + String(gps.speed.kmph()) + "\"}";
      Serial.println(inputString);

      std::string kPath = "/hider";
      String contentType = "application/json";
      String postData = inputString;
      client.post(kPath.c_str(), contentType, postData);

      // read the status code and body of the response
      int statusCode = client.responseStatusCode();
      String response = client.responseBody();
    }
  }
  else
  {
    currCycle++;
  }

  Serial.println(currentState);

  lastState = currentState;
  currentState = digitalRead(BUTTON_PIN);

  if ((lastState == LOW && currentState == HIGH))
  {
    Serial.println("HIDER IS CAUGHT. GAME OVER!!");

    std::string kPath = "/set_game_state";
    String contentType = "application/x-www-form-urlencoded";
    String postData = "state=False";
    client.post(kPath.c_str(), contentType, postData);

    // read the status code and body of the response
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();

    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);
    Serial.println("Wait five seconds");
    delay(5000);
  }
}
