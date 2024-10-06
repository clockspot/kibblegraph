// ESP32 solution to monitor changes in mass over time and make HTTP requests to log changes
// Future version to add low-power support
// https://github.com/clockspot/kibblegraph
// Sketch by Luke McKenzie (luke@theclockspot.com)

#include <arduino.h>
#include <Wire.h>
#include "kibblegraph.h" //specifies config
//#include "esp_sleep.h" //for future low-power support - see flipflop-clock-advancer

//TODO which of these are needed for NTP sync
// #include <WiFi.h>
// #include <WiFiClientSecure.h>
// #include <HTTPClient.h> // Needs to be from the ESP32 platform version 3.2.0 or later, as the previous has problems with http-redirect

#include <Adafruit_NAU7802.h>
Adafruit_NAU7802 nau;
bool nauOK = false;

#ifdef ENABLE_NEOPIXEL
  #include <Adafruit_NeoPixel.h>
  #define NUMPIXELS 1
  Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
#endif

unsigned long sampleLast;
int32_t weightLast;
int32_t weightLastStable;

void setup() {

  Serial.begin(115200);

  #ifdef ENABLE_NEOPIXEL
    #if defined(NEOPIXEL_POWER)
      // If this board has a power control pin, we must set it to output and high
      // in order to enable the NeoPixels. We put this in an #if defined so it can
      // be reused for other boards without compilation errors
      pinMode(NEOPIXEL_POWER, OUTPUT);
      digitalWrite(NEOPIXEL_POWER, HIGH);
    #endif
    
    pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
    pixels.setBrightness(20); // not so bright
    pixels.fill(0xFFFFFF); //white - indicate startup
    pixels.show();
  #endif

  Wire.begin();

  //remove when adding future low-power support
  delay(10000); //gives a chance to upload new sketch before it sleeps

  if (! nau.begin()) {
    Serial.println("Failed to find NAU7802");
    #ifdef ENABLE_NEOPIXEL
      pixels.fill(0xFF0000);
      pixels.show();
    #endif
  } else {
    Serial.println("Found NAU7802");
    nauOK = true;
    //https://github.com/adafruit/Adafruit_NAU7802/blob/master/examples/nau7802_test/nau7802_test.ino
    nau.setLDO(NAU7802_3V0);
    nau.setGain(NAU7802_GAIN_128);
    nau.setRate(NAU7802_RATE_10SPS);
    // Take 10 readings to flush out readings
    for (uint8_t i=0; i<10; i++) {
      while (! nau.available()) delay(1);
      nau.read();
    }

    while (! nau.calibrate(NAU7802_CALMOD_INTERNAL)) {
      Serial.println("Failed to calibrate internal offset, retrying!");
      delay(1000);
    }
    Serial.println("Calibrated internal offset");

    while (! nau.calibrate(NAU7802_CALMOD_OFFSET)) {
      Serial.println("Failed to calibrate system offset, retrying!");
      delay(1000);
    }
    Serial.println("Calibrated system offset");

  } //end nau OK

} //end setup()

void loop() {
  if(!nauOK) return;
  
  unsigned long sampleNow = millis();

  if(sampleNow-sampleLast > (weightLast==weightLastStable? CHECK_INTERVAL_MS: CONFIRM_INTERVAL_MS)) { //time to take a sample

      sampleLast = sampleNow;

      while (! nau.available()) delay(1);
      int32_t weightNow = nau.read();
      // int32_t weightNow = millis()/10000;

      Serial.print(weightNow,DEC);
      Serial.print(F(" -> "));
      Serial.println(weightNow/WEIGHT_DIVIDE,DEC);
      
      int32_t weightDiff = weightNow-weightLast;

      if(weightLast==weightLastStable && abs(weightDiff) >= WEIGHT_TOLERANCE) { //check for instability
        weightLast = weightNow;
        Serial.println(F("Unstable!"));
        #ifdef ENABLE_NEOPIXEL
          pixels.fill(0xFFCC00); //yellow - unstable
          pixels.show();
        #endif
      } else if(weightLast!=weightLastStable && abs(weightDiff) <= WEIGHT_TOLERANCE) { //confirm stability
        weightDiff = weightNow-weightLastStable; //todo what about x to y to x
        weightLast = weightNow;
        weightLastStable = weightNow;
        Serial.print(F("Stable! Weight change: "));
        Serial.println(weightDiff/WEIGHT_DIVIDE,DEC);
        #ifdef ENABLE_NEOPIXEL
          pixels.fill(0x000000);
          pixels.show();
        #endif
      } else weightDiff = 0;

      #ifdef NETWORK_SSID
      if(weightDiff!=0) { //if the weight differed enough to matter, send info
        //Start wifi
        for(int attempts=0; attempts<3; attempts++) {
          Serial.print(F("\nConnecting to WiFi SSID "));
          Serial.println(NETWORK_SSID);
          WiFi.begin(NETWORK_SSID, NETWORK_PASS);
          int timeout = 0;
          while(WiFi.status()!=WL_CONNECTED && timeout<15) {
            timeout++; delay(1000);
          }
          if(WiFi.status()==WL_CONNECTED){ //did it work?
            #ifdef ENABLE_NEOPIXEL
              pixels.fill(0x0000FF); //blue - wifi success
              pixels.show();
            #endif
            Serial.println(F("Connected!"));
            //Serial.print(F("SSID: ")); Serial.println(WiFi.SSID());
            Serial.print(F("Signal strength (RSSI): ")); Serial.print(WiFi.RSSI()); Serial.println(F(" dBm"));
            Serial.print(F("Local IP: ")); Serial.println(WiFi.localIP());
            break; //leave attempts loop
          }
        }
        if(WiFi.status()!=WL_CONNECTED) {
          Serial.println(F("Wasn't able to connect."));
          #ifdef ENABLE_NEOPIXEL
            pixels.fill(0xFF0000); //red - no wifi success
            pixels.show();
            delay(1000);
          #endif

          WiFi.disconnect(true);
          WiFi.mode(WIFI_OFF);
          return;
        }

        //If we reach this point, wifi is OK
        HTTPClient http;
        int httpReturnCode;
        for(int attempts=0; attempts<3; attempts++) {
          Serial.print(F("\nSending to log, attempt "));
          Serial.println(attempts,DEC);
          unsigned long offset = millis()-millisStart;
          http.begin(String(LOG_URL)+"&offset="+String(offset));
          httpReturnCode = http.GET();
          if(httpReturnCode==200) {
            Serial.println(F("Successful!"));
            #ifdef ENABLE_NEOPIXEL
              pixels.fill(0x00FF00); //green - log success
              pixels.show();
              delay(1000);
            #endif
            break; //leave attempts loop
          }
        }
        if(httpReturnCode!=200) {
          Serial.print(F("Not successful. Last HTTP code: "));
          Serial.println(httpReturnCode,DEC);
          #ifdef ENABLE_NEOPIXEL
            pixels.fill(0xFF0000); //red - log failure
            pixels.show();
            delay(1000);
          #endif
        }

        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
      } //if the weight differed
      #endif

  } //end time to take a sample

} //end loop