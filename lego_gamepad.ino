//#include <Adafruit_ST7789.h>
#include <Adafruit_ST7735.h>
//#include <Adafruit_ST77xx.h>
//#include <gfxfont.h>
//#include <Adafruit_SPITFT_Macros.h>
//#include <Adafruit_SPITFT.h>
//#include <Adafruit_GrayOLED.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

#include <PowerFunctions.h>

//#include <NimBLEDevice.h>
//#include <Lpf2HubEmulation.h>
//#include <Lpf2HubConst.h>
#include <Lpf2Hub.h>
//#include <LegoinoCommon.h>
//#include <Boost.h>

#define TFT_DC 22 //A0
#define TFT_CS 5 //CS
#define TFT_RST 17 
#define TFT_MOSI 23 //SDA
#define TFT_CLK 18 //SCK

#define PIN_IRLED 32
#define PIN_JOY1_X 39
#define PIN_JOY1_Y 34
#define PIN_TRIGGER_R 36
#define PIN_TRIGGER_L 35

//Adafruit_ST77xx tft = Adafruit_ST77xx(128, 128, TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK);
//Adafruit_ST77xx tft = Adafruit_ST77xx(128, 128, new SPIClass(VSPI), TFT_CS, TFT_DC);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
PowerFunctions powerFunctions(PIN_IRLED, 0); //Pin 12, Channel 0
Lpf2Hub myHub;

byte portA = (byte)ControlPlusHubPort::A;
byte portB = (byte)ControlPlusHubPort::B;
byte portC = (byte)ControlPlusHubPort::C;
byte portD = (byte)ControlPlusHubPort::D;

int counter;


void setup()
{
	Serial.begin(115200);

    tft.initR(INITR_144GREENTAB);
    
    tft.fillScreen(ST7735_BLACK);
    tft.setRotation(3);
    
	tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5, 5);
    //tft.println("Hello");

	//tft.drawRGBBitmap(0, 0, *epd_bitmap_allArray, 128, 128);

    myHub.init();
}

void loop()
{
	if (myHub.isConnecting()) {
		myHub.connectHub();
		if (myHub.isConnected()) {
			Serial.println("We are now connected to the HUB");
		}
		else {
			Serial.println("We have failed to connect to the HUB");
		}
	}


    int joy_x_raw = analogRead(PIN_JOY1_Y);
    int joy_x_value = map(joy_x_raw, 0, 4096, -110, 110) + 9;
    int trigger_r_raw = analogRead(PIN_TRIGGER_R);
    int trigger_r_value = min(max(map(trigger_r_raw, 300, 2500, 100, 0), 0l), 100l);
	int trigger_l_raw = analogRead(PIN_TRIGGER_L);
	int trigger_l_value = min(max(map(trigger_l_raw, 300, 2500, 0, 100), 0l), 100l);
	
    int steer = min(100, max(-100, joy_x_value)) * -1;
	int speed = (trigger_r_value - trigger_l_value);

    /*if (steer < -5 || steer > 5)
    {
        PowerFunctionsPwm pwm = powerFunctions.speedToPwm(steer);
        powerFunctions.single_pwm(PowerFunctionsPort::BLUE, pwm);
    }
    else
    {
        powerFunctions.single_pwm(PowerFunctionsPort::BLUE, PowerFunctionsPwm::BRAKE);
    }

    powerFunctions.single_pwm(PowerFunctionsPort::RED, powerFunctions.speedToPwm(speed));*/

    if (myHub.isConnected())
    {
        myHub.setBasicMotorSpeed(portC, speed);
        myHub.setBasicMotorSpeed(portD, speed);
        myHub.setAbsoluteMotorPosition(portB, 100, steer * 0.9);
    }

    renderState(steer, speed);

    delay(100);
}


void renderState(int steer, int power)
{
    /*tft.setCursor(5, 5);
    tft.print(joy_x_value);
    tft.print(" ");
    tft.print(steer);
    tft.print("     ");

    tft.setCursor(5, 15);
    tft.print(trigger_l_value);
    tft.print(" ");
    tft.print(trigger_r_value);
    tft.print(" ");
    tft.print(speed);
    tft.print("      ");*/

    renderBarBox(3, 3, 100, 9, steer, ST7735_GREEN, true);
    renderBarBox(115, 3, 9, 120, power, ST7735_BLUE, false);
}

void renderBarBox(int x, int y, int w, int h, int value, uint16_t color, bool horizontal)
{
    int half = horizontal ? w / 2 : h / 2;
    tft.fillRect(x + 1, y + 1, w - 2, h - 2, ST7735_BLACK);
    tft.fillRect(
        horizontal ? x + half : x + 1, 
        horizontal ? y + 1 : y + half, 
        horizontal ? value * half * -0.01 : w - 2, 
        horizontal ? h - 2 : value * half * -0.01,
        color);
    
    tft.drawRect(x, y, w, h, ST7735_WHITE);
}