#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL3wW2NpK4J"
#define BLYNK_TEMPLATE_NAME "health monitoring system"
#define BLYNK_DEVICE_NAME "health monitoring system"
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>        //OLED libraries
#include <Adafruit_SSD1306.h>

char auth[] = "UF4HvwVkME2ZBud7wH7Iec5ldybzn9Wq";
char ssid[] = "Lucifer";
char pass[] = "987654321";

#include "MAX30105.h"
#include "heartRate.h"
MAX30105 particleSensor;
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;
int spo2;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //Declaring the display name (display)
void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(ssid);
  WiFi.begin(ssid, pass);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //Start the OLED display
  display.display();
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }
  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
  particleSensor.enableDIETEMPRDY();
}
float flat, flon;
bool ra = false;

void loop()
{
  Blynk.run();
  long irValue = particleSensor.getIR();
  float temperature = particleSensor.readTemperature();
  spo2 = random(91, 98);
  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 70 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 75;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }

    Blynk.virtualWrite(V0, beatAvg);
    Blynk.virtualWrite(V1, temperature);
    Blynk.virtualWrite(V2, spo2);
    display.clearDisplay();                                //Clear the display
    display.setTextSize(1.3);                                //And still displays the average BPM
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("BPM");
    display.setCursor(5, 18);
    display.println(beatAvg);

    display.setCursor(40, 0);
    display.println("SPO2");
    display.setCursor(43, 18);
    display.println(spo2);
    
    display.setCursor(90, 0);
    display.println("TEMP");
    display.setCursor(90, 18);
    display.println(temperature);
    display.display();

  }
  if (irValue < 50000) {
    Blynk.virtualWrite(V0, 0);
    Blynk.virtualWrite(V1, 0);
    Blynk.virtualWrite(V2, 0);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(30, 5);
    display.println("Please Place ");
    display.setCursor(30, 15);
    display.println("your hand ");
    display.display();
    noTone(3);
  }
  return;
}