# kibblegraph

Monitors long-term changes in weight (of a cat food bowl) and sends out HTTP requests to log changes.

First implementation simply implements a timer with millis(). Later version may go wireless with more hardware (below).

## Hardware

Prototype implementation uses hardware selections from Adafruit: [QT Py ESP32-S2](https://www.adafruit.com/product/5325), [NAU7802](https://www.adafruit.com/product/4538), [20Kg strain gauge](https://www.adafruit.com/product/4543).

You will need to install the libraries for this board and the Adafruit NAU7802. For the latter, in `Adafruit_NAU7802.cpp`, you may need to comment out the six lines beginning with `Check for NAU7802 revision register` (per [this bug](https://github.com/adafruit/Adafruit_NAU7802/issues/5)).

To go wireless, a later version may incorporate a [DS3231 RTC](https://learn.adafruit.com/adafruit-ds3231-precision-rtc-breakout) (to support wake from deep sleep, as in [flipflop-clock-advancer](https://github.com/clockspot/flipflop-clock-advancer)), [BFF power supply/charger](https://www.adafruit.com/product/5397) and [battery](https://www.adafruit.com/product/1781). The QT Py connections (besides STEMMA QT I2C) will be:

* A2 (GPIO 9) to BFF (and enable `BATTERY_MONITOR_PIN` in config)
* A3 (GPIO 8) to RTC SQW (this is the interrupt pin the RTC uses to wake up the ESP32)

## Google Form/Sheet setup

You can have this submit anywhere (e.g. to an HTTP server of your own design), but I've set it up to submit to a Google sheet thus:

* Set up a Google form that doesn't require login to participate and writes results to a Google sheet (optionally messaging you when results come in)
* Add a single field for weight, and inspect the field on the public form to get its ID
* In `configs`, edit (or create a new) `.h` file, fill in properties, and specify that file in `kibblegraph.h`