/*
#include <Arduino.h>
#include <HttpClient.h>
#include <WiFi.h>
#include <inttypes.h>
#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <TinyGPS++.h>
#define RXD2 9
#define TXD2 10

#define GPS_BAUD 9600

#define BUTTON_PIN 39

// The TinyGPS++ object
TinyGPSPlus gpsTemp;

// Create an instance of the HardwareSerial class for Serial 2
HardwareSerial gpsSerialTemp(2);

int lastState = LOW;    // the previous state from the input pin
int currentState = LOW; // the current reading from the input pin

char ssid[50]; // your network SSID (name)
char pass[50]; // your network password (use for WPA, or use
// as key for WEP)
// Name of the server we want to connect to
const char kHostname[] = "worldtimeapi.org";
// Path to download (this is the bit after the hostname in the URL
// that you want to download
const char kPath[] = "/api/timezone/Europe/London.txt";
// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;
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
void setupTemp()
{
    Serial.begin(9600);
    delay(1000);
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

    gpsSerialTemp.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
    Serial.println("Serial 2 started at 9600 baud rate");
    pinMode(BUTTON_PIN, INPUT_PULLDOWN);
}

void loopTemp()
{

    unsigned long start = millis();

    while (millis() - start < 1000)
    {
        // char gpsData = gpsSerial.read();
        // Serial.print(gpsData);
        while (gpsSerialTemp.available() > 0)
        {
            gpsTemp.encode(gpsSerialTemp.read());
        }
        if (gpsTemp.location.isUpdated())
        {
            Serial.print("LAT: ");
            Serial.println(gpsTemp.location.lat(), 6);
            Serial.print("LONG: ");
            Serial.println(gpsTemp.location.lng(), 6);
            Serial.print("SPEED (km/h) = ");
            Serial.println(gpsTemp.speed.kmph());
            Serial.print("ALT (min)= ");
            Serial.println(gpsTemp.altitude.meters());
            Serial.print("HDOP = ");
            Serial.println(gpsTemp.hdop.value() / 100.0);
            Serial.print("Satellites = ");
            Serial.println(gpsTemp.satellites.value());
            Serial.print("Time in UTC: ");
            Serial.println(String(gpsTemp.date.year()) + "/" + String(gpsTemp.date.month()) + "/" + String(gpsTemp.date.day()) + "," + String(gpsTemp.time.hour()) + ":" + String(gpsTemp.time.minute()) + ":" + String(gpsTemp.time.second()));
            Serial.println("");
        }
    }

    lastState = currentState;
    currentState = digitalRead(BUTTON_PIN);
    if (lastState == LOW && currentState == HIGH)
    {
        Serial.println("HIDER IS CAUGHT. GAME OVER!!!!!!!!!!!!!!!!!!!!!!");
        delay(5000);    
    }

    int err = 0;
    WiFiClient c;
    HttpClient http(c);

    String inputString = "{\"lat\":\"" + String(gpsTemp.location.lat()) + "\",\"lng\":\"" + String(gpsTemp.location.lng()) + "\",\"speed\":\"" + String(gpsTemp.speed.value()) + "\"}";
    String inputStringTest = "{\"lat\":\"" + String(33.646062) + "\",\"lng\":\"" + String(-117.846545) + "\",\"speed\":\"" + String(15) + "\"}";

    err = http.post("https://159a-169-234-51-122.ngrok-free.app/", 5000, "hider", inputStringTest.c_str());

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
    // And just stop, now that we've tried a download
}
*/


/*







*/