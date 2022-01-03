
#include <SPI.h>
#include <NimBLEDevice.h>
//#include <Lpf2HubEmulation.h>
//#include <Lpf2HubConst.h>
//#include <LegoinoCommon.h>
//#include <BLEDevice.h>


// ----- input
#define PIN_JOY1_X 39
#define PIN_JOY1_Y 34
#define PIN_TRIGGER_R 36
#define PIN_TRIGGER_L 35


// ----- TFT
#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>

#define TFT_DC 22 //A0
#define TFT_CS 5 //CS
#define TFT_RST 17 
#define TFT_MOSI 23 //SDA
#define TFT_CLK 18 //SCK

//Adafruit_ST77xx tft = Adafruit_ST77xx(128, 128, TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK);
//Adafruit_ST77xx tft = Adafruit_ST77xx(128, 128, new SPIClass(VSPI), TFT_CS, TFT_DC);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);


// ---- PF
#include <PowerFunctions.h>
#define PIN_IRLED 32
PowerFunctions powerFunctions(PIN_IRLED, 0); //Pin 12, Channel 0


// ----- PU hub
#include <Lpf2Hub.h>
Lpf2Hub myHub;
byte portA = (byte)ControlPlusHubPort::A;
byte portB = (byte)ControlPlusHubPort::B;
byte portC = (byte)ControlPlusHubPort::C;
byte portD = (byte)ControlPlusHubPort::D;


// ----- ble/buwizz 
static BLEUUID serviceUUID("500592d1-74fb-4481-88b3-9919b1676e93"); //Service UUID of BuWizz 3
static BLEUUID charUUID("50052901-74fb-4481-88b3-9919b1676e93"); //Characteristic  UUID of BuWizz control
static BLERemoteCharacteristic* pRemoteCharacteristic;
BLEScan* pBLEScan; //Name the scanning device as pBLEScan
BLEScanResults foundDevices;
static BLEAddress* Server_BLE_Address;
String Scaned_BLE_Address;
uint8_t valueselect[] = { 0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }; //BuWizz "payload" to control the outputs.
boolean buwizz_paired = false; //boolean variable to toggel pairing
boolean buwizz_notfound = true;


// ---- application
int counter;


void setup()
{
	Serial.begin(115200);

    ui_init();

    //pu_init();
    bw_init();
}

void loop()
{
    //pu_check_connection();

    // read input
    int joy_x_raw = analogRead(PIN_JOY1_Y);
    int joy_x_value = map(joy_x_raw, 0, 4096, -110, 110) + 9;
    int trigger_r_raw = analogRead(PIN_TRIGGER_R);
    int trigger_r_value = min(max(map(trigger_r_raw, 300, 2500, 100, 0), 0l), 100l);
	int trigger_l_raw = analogRead(PIN_TRIGGER_L);
	int trigger_l_value = min(max(map(trigger_l_raw, 300, 2500, 0, 100), 0l), 100l);
    int steer = min(100, max(-100, joy_x_value)) * -1;
	int speed = (trigger_r_value - trigger_l_value);

    
    //pf_update(speed, steer);
    //pu_update(speed, steer);
    bw_update(speed, steer);

    ui_render(steer, speed);

    delay(100);
}


void pu_init()
{
    myHub.init();
}

void pu_check_connection()
{
    if (myHub.isConnecting()) 
    {
        myHub.connectHub();

        if (myHub.isConnected()) 
        {
            Serial.println("We are now connected to the HUB");
        }
        else 
        {
            Serial.println("We have failed to connect to the HUB");
        }
    }
}

void pu_update(int speed, int steer)
{
    if (myHub.isConnected())
    {
        myHub.setBasicMotorSpeed(portC, speed);
        myHub.setBasicMotorSpeed(portD, speed);
        myHub.setAbsoluteMotorPosition(portB, 100, steer * 0.9);
    }
}


void pf_update(int speed, int steer)
{
    if (steer < -5 || steer > 5)
    {
        PowerFunctionsPwm pwm = powerFunctions.speedToPwm(steer);
        powerFunctions.single_pwm(PowerFunctionsPort::BLUE, pwm);
    }
    else
    {
        powerFunctions.single_pwm(PowerFunctionsPort::BLUE, PowerFunctionsPwm::BRAKE);
    }

    powerFunctions.single_pwm(PowerFunctionsPort::RED, powerFunctions.speedToPwm(speed));
}


void ui_init()
{
    tft.initR(INITR_144GREENTAB);

    tft.fillScreen(ST7735_BLACK);
    tft.setRotation(3);

    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5, 5);
    //tft.println("Hello");

    //tft.drawRGBBitmap(0, 0, *epd_bitmap_allArray, 128, 128);
}

void ui_render(int steer, int power)
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

    ui_renderBarBox(3, 3, 100, 9, steer, ST7735_GREEN, true);
    ui_renderBarBox(115, 3, 9, 120, power, ST7735_BLUE, false);
}

void ui_renderBarBox(int x, int y, int w, int h, int value, uint16_t color, bool horizontal)
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


bool connectToserver(BLEAddress pAddress) 
{
    BLEClient* pClient = BLEDevice::createClient();
    
    boolean connection_ok = false;
    
    while (!connection_ok) 
    {
        Serial.println("Connecting to BuWizz...");
        
        if (pClient->connect(BLEAddress(pAddress)))
        {
            connection_ok = true;
            
            Serial.println("Connected to BuWizz");
        }
        else
            return false;
    }

    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);     // Obtain a reference to the service we are after in the remote BLE server.
    
    if (pRemoteService != nullptr)
    {
        Serial.println(" - Found the service");
    }
    else
    {
        pClient->disconnect();
        return false;
    }

    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);     // Obtain a reference to the characteristic in the service of the remote BLE server.
    
    if (pRemoteCharacteristic != nullptr)
    {
        Serial.println(" - Found the characteristic");
        buwizz_paired = true;
        return true;
    }
    else
    {
        pClient->disconnect();
        return false;
    }
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(NimBLEAdvertisedDevice* advertisedDevice)
    {
        Serial.printf("Scan Result: %s \n", advertisedDevice->toString().c_str());

        if (advertisedDevice->getName() == "BuWizz3") 
        {
            Serial.println("BuWizz found! MAC address:");
            advertisedDevice->getScan()->stop();
            buwizz_notfound = false;
            Server_BLE_Address = new BLEAddress(advertisedDevice->getAddress());
            Scaned_BLE_Address = Server_BLE_Address->toString().c_str();
            Serial.println(Scaned_BLE_Address);
        }
    }
};

void bw_init()
{
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan(); //create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks()); //Call the class that is defined above 
    pBLEScan->setActiveScan(false); //active scan uses more power, but get results faster

    while (buwizz_notfound == true) 
    {
        pBLEScan->start(3);
    }

    while (connectToserver(*Server_BLE_Address) != true) 
    {
        Serial.println("Tikk-Takk.. waiting..");
        delay(5000);
    }

    //pRemoteCharacteristic->writeValue((uint8_t*)modeselect, 2, true);
}

void bw_update(int speed, int steer)
{
    if (buwizz_paired == false)
    {
        return;
    }

    valueselect[1] = speed;
    valueselect[2] = speed;
    valueselect[3] = speed;
    valueselect[4] = speed;
    valueselect[5] = speed;
    valueselect[6] = speed;
    //valueselect[7] = speed;


    pRemoteCharacteristic->writeValue((uint8_t*)valueselect, 9, true);
}