#ifndef CONFIG_H
#define CONFIG_H

#define SHOW_SERIAL
#define ENABLE_NEOPIXEL
#define Wire Wire1 //If using Adafruit QT Py ESP32, with all the I2C stuff connected to the QT port (Wire1) rather than the pins (Wire) - TODO will this mess up your use of the regular Wire pins for other purposes?

//pins expressed in gpio_num_t format
// #define BATTERY_MONITOR_PIN GPIO_NUM_9 //A2, when QT Py is equipped with LiPo BFF
// #define WAKEUP_PIN GPIO_NUM_8 //A3 per https://learn.adafruit.com/adafruit-qt-py-esp32-s2/pinouts

//#define ENABLE_NTP_SYNC

//Uncomment to support logging via HTTP requests
// #define NETWORK_SSID "{{your network}}"
// #define NETWORK_PASS "{{your password}}"

// https://stackoverflow.com/a/72548850
#define LOG_URL "https://docs.google.com/forms/d/e/{{formId}}/formResponse"
#define LOG_HOST "docs.google.com"
#define LOG_CONTENT_TYPE "application/x-www-form-urlencoded"
#define LOG_KEY "{{entry.id}}"

#define CHECK_INTERVAL_MS 60000 //When stable, resample at this rate to check for instability (cat has started eating). E.g. 60000
#define CONFIRM_INTERVAL_MS 30000 //When unstable, resample at this rate to confirm stability (cat has stopped eating). E.g. 30000
//If doing calibration via serial, change these to e.g. 1000

#define WEIGHT_TOLERANCE 2 //The weight difference (in strain gauge units) required to trigger instability/determine stability
#define WEIGHT_SCALING 1.25 //To convert from strain gauge units to grams

// #define NTP_HOST "pool.ntp.org"
//#define TZ_OFFSET_SEC -21600 //will go at midnight in this time zone
// #define TZ_OFFSET_SEC -18000 //will go at midnight in this time zone - 1am Central
// #define DST_OFFSET_SEC 3600

#endif //CONFIG_H