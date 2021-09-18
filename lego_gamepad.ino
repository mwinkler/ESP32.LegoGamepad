//#include <Adafruit_ST77xx.h>
//#include <Adafruit_ST7789.h>
#include <Adafruit_ST7735.h>
//#include <gfxfont.h>
//#include <Adafruit_SPITFT_Macros.h>
//#include <Adafruit_SPITFT.h>
//#include <Adafruit_GrayOLED.h>
#include <Adafruit_GFX.h>

#include <PowerFunctions.h>

//#include <NimBLEDevice.h>
//#include <Lpf2HubEmulation.h>
//#include <Lpf2HubConst.h>
//#include <Lpf2Hub.h>
//#include <LegoinoCommon.h>
//#include <Boost.h>

#define TFT_DC 22 //A0
#define TFT_CS 5 //CS
#define TFT_MOSI 23 //SDA
#define TFT_CLK 18 //SCK
//#define TFT_RST 0 

#define PIN_IRLED 32
#define PIN_JOY1_X 39
#define PIN_JOY1_Y 34
#define PIN_TRIGGER_R 36
#define PIN_TRIGGER_L 35

Adafruit_ST77xx tft = Adafruit_ST77xx(128, 128, TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK);
//Adafruit_ST77xx tft = Adafruit_ST77xx(128, 128, new SPIClass(VSPI), TFT_CS, TFT_DC);
PowerFunctions powerFunctions(PIN_IRLED, 0); //Pin 12, Channel 0

int counter;

void setup()
{
	//Serial.begin(115200);

    tft.initSPI();
    
    
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    
    tft.setRotation(3);
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(5, 5);
    tft.println("Hello");
}

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
}
