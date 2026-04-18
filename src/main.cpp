/*
==================== CatFeederPro Wiring ====================

--- LCD (I2C) ---
VCC  -> 5V (Pin 2 or 4)
GND  -> GND (Pin 6)
SDA  -> GPIO2 (Pin 3)
SCL  -> GPIO3 (Pin 5)

--- RFID (RC522 - SPI) ---
VCC      -> 3.3V (Pin 1)
GND      -> GND (Pin 9)
SDA / SS -> GPIO8  (Pin 24)
SCK      -> GPIO11 (Pin 23)
MOSI     -> GPIO10 (Pin 19)
MISO     -> GPIO9  (Pin 21)
RST      -> GPIO25 (Pin 22)
IRQ      -> not used

--- Ultrasonic (HC-SR04) ---
VCC  -> 5V
GND  -> GND
TRIG -> GPIO17 (Pin 11)
ECHO -> GPIO27 (Pin 13) through voltage divider:
    ECHO ----[1kΩ]----+----> GPIO27
                      |
                    [2kΩ]
                      |
                     GND

--- Servo (SG90 / 9G) ---
Brown  -> GND (Pin 6)
Red    -> 5V  (Pin 2 or 4)
Orange -> GPIO18 (Pin 12)

=============================================================
*/

#include "display/lcd.hpp"
#include "distance/hcsr04.hpp"
#include "rfid/rc522_reader.hpp"
#include "servo/sg90.hpp"

#include <chrono>
#include <string>
#include <thread>

int main() {
    display::Lcd lcd;
    lcd.init();

    distance::Hcsr04 sensor;
    servo::Sg90 motor;
    rfid::Rc522Reader reader;

    if (!sensor.begin()) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Dist init fail");
        return 1;
    }

    if (!motor.begin()) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Servo init fail");
        return 1;
    }

    if (!reader.begin()) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("RFID init fail");
        return 1;
    }

    bool servoForward = false;

    while (true) {
        double cm = sensor.readCm();
        std::string id;

        bool sawCard = reader.readId(id);

        lcd.clear();

        if (sawCard) {
            lcd.setCursor(0, 0);
            lcd.print("RFID:");
            lcd.setCursor(0, 1);
            lcd.print(id.substr(0, 16));

            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            continue;
        }

        lcd.setCursor(0, 0);

        if (cm < 0) {
            lcd.print("Dist: timeout");
            lcd.setCursor(0, 1);
            lcd.print("Servo: hold");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }

        lcd.print("Dist: " + std::to_string((int)cm) + "cm");

        // Hysteresis so it doesn't flap constantly
        if (!servoForward && cm < 10.0) {
            motor.setAngle(120);   // "forward"
            servoForward = true;

            lcd.setCursor(0, 1);
            lcd.print("Servo: forward");

            // hold longer
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            continue;
        }

        if (servoForward && cm > 14.0) {
            motor.setAngle(60);    // "backward"
            servoForward = false;

            lcd.setCursor(0, 1);
            lcd.print("Servo: back");

            // hold longer
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            continue;
        }

        lcd.setCursor(0, 1);
        lcd.print(servoForward ? "Servo: forward" : "Servo: back");

        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }

    return 0;
}