#include <Adafruit_ST77xx.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_ST7735.h>
#include <gfxfont.h>
#include <Adafruit_SPITFT_Macros.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_GFX.h>
#include <NimBLEDevice.h>
#include <PowerFunctions.h>
//#include <Lpf2HubEmulation.h>
#include <Lpf2HubConst.h>
#include <Lpf2Hub.h>
#include <LegoinoCommon.h>
#include <Boost.h>

int PIN_IRLED = 32;
int PIN_JOY1_X = 39;
int PIN_JOY1_Y = 34;
int PIN_TRIGGER_R = 36;
int PIN_TRIGGER_L = 35;
PowerFunctions powerFunctions(PIN_IRLED, 0); //Pin 12, Channel 0

// The setup() function runs once each time the micro-controller starts
void setup()
{
	//pinMode(IrLed, OUTPUT);
	Serial.begin(115200);
    //pinMode(PIN_JOY1_X, INPUT);
    //analogReadResolution(4);
}

// Add the main program code into the continuous loop() function
void loop()
{
    int joy_x_value = map(analogRead(PIN_JOY1_Y), 0, 4096, -110, 110) + 9;
    int trigger_r_raw = analogRead(PIN_TRIGGER_R);
    int trigger_r_value = min(max(map(trigger_r_raw, 300, 2500, 100, 0), 0l), 100l);

    //Serial.println(joy_x_value);

    if (joy_x_value < -5 || joy_x_value > 5)
    {
        PowerFunctionsPwm pwm = powerFunctions.speedToPwm(joy_x_value);
        powerFunctions.single_pwm(PowerFunctionsPort::BLUE, pwm);
    }
    else
    {
        powerFunctions.single_pwm(PowerFunctionsPort::BLUE, PowerFunctionsPwm::BRAKE);
    }

    powerFunctions.single_pwm(PowerFunctionsPort::RED, powerFunctions.speedToPwm(trigger_r_value));

    delay(100);

    /*for (int i = 0; i < 110; i += 10)
    {
        PowerFunctionsPwm pwm = powerFunctions.speedToPwm(i);
        powerFunctions.single_pwm(PowerFunctionsPort::BLUE, pwm);
        Serial.print("single_pwm | speed: ");
        Serial.print(i);
        Serial.print(" pwm: ");
        Serial.println((uint8_t)pwm, HEX);
        delay(1000);
    }*/
}
