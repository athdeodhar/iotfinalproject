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
int gpsCycle = 7;
int currCycle = 1;
#define BUTTON_PIN 39
int pressed = 0;
int counter = 0;
TinyGPSPlus gps;
int lastState = LOW;    // the previous state from the input pin
int currentState = LOW; // the current reading from the input pin
// Create an instance of the HardwareSerial class for Serial 2
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
 *************************SETUP FOR INITIAL WIFI USERNAME / PASSWORD *******************************
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
    char ssid[] = "SSIDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"; /////////////////////////////////////////////////////////////////////////
    char pass[] = " PASSWORDDDDDDDDDDDDDDDDDDDDDDDDDDD";///////////////////////////////////////////////////////////////////////////
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
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
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
      // Serial.printf("SSID = %s\n", ssid);
      // Serial.printf("PASSWD = %s\n", pass);
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

  // Retrieve SSID/PASSWD from flash before anything else
  nvs_access();
  // We start by connecting to a WiFi network
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

  int err = 0;
  WiFiClient wifi;
  HttpClient client = HttpClient(wifi, kHostname, 5000);

  // char gpsData = gpsSerial.read();
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
    }
  }
  else
  {
    currCycle++;
  }
  // Serial.println(currCycle);

  Serial.println(currentState);

  lastState = currentState;
  currentState = digitalRead(BUTTON_PIN);
  if (lastState == LOW && currentState == HIGH)
  {
    Serial.println("HIDER IS CAUGHT. GAME OVER!!!!!!!!!!!!!!!!!!!!!!");

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

  /*
   String inputString = "{\"lat\":\"" + String(gps.location.lat()) + "\",\"lng\":\"" + String(gps.location.lng()) + "\",\"speed\":\"" + String(gps.speed.value()) + "\"}";
   // String inputStringTest = "{\"lat\":\"" + String(33.646062) + "\",\"lng\":\"" + String(-117.846545) + "\",\"speed\":\"" + String(15) + "\"}";
   Serial.println(inputString);
   // err = http.post("18.116.74.254", 5000, "hider", inputString.c_str());

   if (err == 0)
   {
     Serial.println("startedRequest ok");
     err = http.responseStatusCode();
     if (err >= 0)
     {
       Serial.print("Got status code: ");
       Serial.println(err);
       // Usually you'd check that the response code is 200 or a
       // similar "success" code (200-299) before carrying on,
       // but we'll print out whatever response we get
       err = http.skipResponseHeaders();
       if (err >= 0)
       {
         int bodyLen = http.contentLength();
         Serial.print("Content length is: ");
         Serial.println(bodyLen);
         Serial.println();
         Serial.println("Body returned follows:");
         // Now we've got to the body, so we can print it out
         unsigned long timeoutStart = millis();
         char c;
         // Whilst we haven't timed out & haven't reached the end of the body
         while ((http.connected() || http.available()) &&
                ((millis() - timeoutStart) < kNetworkTimeout))
         {
           if (http.available())
           {
             c = http.read();
             // Print out this character
             Serial.print(c);
             bodyLen--;
             // We read something, reset the timeout counter
             timeoutStart = millis();
           }
           else
           {
             // We haven't got any data, so let's pause to allow some to
             // arrive
             delay(kNetworkDelay);
           }
         }
       }
       else
       {
         Serial.print("Failed to skip response headers: ");
         Serial.println(err);
       }
     }
     else
     {
       Serial.print("Getting response failed: ");
       Serial.println(err);
     }
   }
   else
   {
     Serial.print("Connect failed: ");
     Serial.println(err);
   }
   http.stop();

   */
}
