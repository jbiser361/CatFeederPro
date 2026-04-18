#include "display/lcd.hpp"
#include "rfid/rc522_reader.hpp"
#include "distance/hcsr04.hpp"

#include <chrono>
#include <string>
#include <thread>

/*
==================== CatFeederPro Wiring ====================

Raspberry Pi (BCM numbering)

--- LCD (I2C) ---
VCC  -> 5V (Pin 2 or 4)
GND  -> GND (Pin 6)
SDA  -> GPIO2 (Pin 3)
SCL  -> GPIO3 (Pin 5)
I2C Address: 0x27

--- RFID (RC522 - SPI) ---
VCC  -> 3.3V (Pin 1)
GND  -> GND (Pin 9)
SDA (SS) -> GPIO8  (Pin 24)
SCK      -> GPIO11 (Pin 23)
MOSI     -> GPIO10 (Pin 19)
MISO     -> GPIO9  (Pin 21)
RST      -> GPIO25 (Pin 22)
IRQ      -> (not used)

--- Ultrasonic (HC-SR04) ---
VCC  -> 5V (Pin 2 or 4)
GND  -> GND (Pin 6)

TRIG -> GPIO17 (Pin 11)

ECHO -> GPIO27 (Pin 13) THROUGH VOLTAGE DIVIDER:
    ECHO ----[1k]----+----> GPIO27
                     |
                   [2k]
                     |
                    GND

NOTE:
- DO NOT connect ECHO directly to Pi (5V signal)
- Common ground required across ALL components

=============================================================
*/

int main() {
    display::Lcd lcd;
    lcd.init();

    rfid::Rc522Reader reader;
    distance::Hcsr04 sensor;

    if (!reader.begin()) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("RFID init fail");
        return 1;
    }

    if (!sensor.begin()) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Dist init fail");
        return 1;
    }

    while (true) {
        double cm = sensor.readCm();

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("RFID ready");

        lcd.setCursor(0, 1);
        if (cm < 0) {
            lcd.print("Dist: timeout");
        } else {
            std::string dist = "Dist: " + std::to_string((int)cm) + "cm";
            lcd.print(dist.substr(0, 16));
        }

        std::string id;
        if (reader.readId(id)) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("RFID card:");
            lcd.setCursor(0, 1);
            lcd.print(id.substr(0, 16));
            std::this_thread::sleep_for(std::chrono::milliseconds(1200));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    return 0;
}
