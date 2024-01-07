## General 

Simulates the [DCF77](https://en.wikipedia.org/wiki/DCF77) time code signal on a GPIO pin. It uses PWM with frequency set to 77.5 kHz to simulate the carrier signal. The time is synchronized using NTP server.

## Installation
Configure your Wi-Fi credentials the `defaults_override.ini` file. 

If missing, the file will be created on the initial build (see `override_copy.py`). 

By default generic ESP32 board (`esp32dev`) is used where only the `DCF77_GPIO` is configured. 

It is also possible to show the time and the transmitted data on an attached display using the `TFT_eSPI` library. 

Select one of the pre-configured environments in the `platformio.ini` or create one for your device.

## Antenna
Best results are achieved if you connect a ferrite antenna over 330 ohm resistor and a capactior to ground.

It also works with analog beeper or even with a led connected to the GPIO pin. 

Normally the clock gets syncrhonized in about two minutes depending on the distance and signal strength.
