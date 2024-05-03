/*
 * Project trafficLogic
 * Author: Christian Chavez
 * Date: April 29th, 2024
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"

#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

#include "neopixel.h"

// Define parameters for OLED and create 'display' object
#define OLED_RESET D4
Adafruit_SSD1306 display(OLED_RESET);

// Define number of pixels and create 'pixel' object/'

const int PIXELCOUNT = 64;
Adafruit_NeoPixel pixel(PIXELCOUNT, SPI1, WS2812B);
int pixelPattern = 0; // variable to track pixel sequences

int pixelColor = 0x0000FF; // variable to store color variable starting with default blue

int pixelBrightness = 100; // variable for pixel brightness 0-255

int myTimeZone = -7; // variable to adjust time zone from UTC

unsigned int lastTime = -60000; // variable for interval timer to track millis()

unsigned int lastShowTime = -60000; // variable to track last pixel update

String travelLocations = "34.1808,-118.3089:33.9416,-118.4085"; // Burbank to LAX

String routeDescription = "Burbank to LAX";

int travelTimeInSeconds = -999;
int trafficDelayInSeconds = -999;

int targetHour = 10; // variable to track target arrival time
int targetMinute = 00;

int minutesToLeave = -999;

char data[particle::protocol::MAX_EVENT_DATA_LENGTH + 1];

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
// SerialLogHandler logHandler(LOG_LEVEL_INFO);

// create prototypes of functions
void handleResponse(const char *event, const char *data);

void lightPixels(int patternNumber);

// setup() runs once, when the device is first turned on
void setup()
{
  // Put initialization like pinMode and begin functions here

  Serial.begin(9600);
  delay(1000);                           // wait for the serial monitor to initialize
  Serial.printf("PLEASE STAND BY...\n"); // print waiting message on serial monitor

  // display setup
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // initialize with the I2C address of the display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(BLACK, WHITE);     // text will print in black with white background
  display.printf("PLEASE STAND BY...\n"); // print waiting message on display
  display.display();                      // update display to show text

  // neopixel setup
  pixel.begin();
  pixel.setBrightness(pixelBrightness);
  pixel.clear();
  pixel.show();

  for (int i = 0; i < PIXELCOUNT; i++)
  { // light up all pixels random colors on startup
    pixel.setPixelColor(i, random(0x000000, 0xFFFFFF));
    pixel.show();
  }

  delay(1000);

  // time setup
  Particle.syncTime();
  Time.zone(myTimeZone);

  // Particle.subscribe("TomTomResponse", handleResponse, ALL_DEVICES); // assign handleResponse function to run on response from Particle subscription
  Particle.subscribe("calculateRouteResponse", handleResponse, ALL_DEVICES); // assign handleResponse function to run on response from Particle subscription
  lastTime = -120000;                                                        // intitialize our timer variable
}

void loop()
{
  lightPixels(pixelPattern);

  if (Particle.connected() && millis() - lastTime > 60000)
  {
    Serial.printf("\n\nLocal Time Now: %02i:%02i\n\n", Time.hour(), Time.minute()); // print current time

    JSONBufferWriter writer(data, sizeof(data) - 1); //build json data

    writer.beginObject();

    writer.name("coordinates").value(travelLocations);
    writer.name("h").value(Time.hour());
    writer.name("m").value(Time.minute());
    writer.name("s").value(Time.second());
    writer.name("yr").value(Time.year());
    writer.name("mo").value(Time.month());
    writer.name("d").value(Time.weekday());
    writer.name("tz").value(Time.zone());

    writer.endObject();

    Serial.printf("json: %s\n", data);

    Particle.publish("trafficLogic", data); // call Particle Logic function

    lastTime = millis(); // reset our timer
  }
}

void lightPixels(int patternNumber)
{
  static int currentPixel = 0;
  if (millis() - lastShowTime > 222)
  {

    if (patternNumber == 0) // default ambient random pattern
    {
      pixel.setPixelColor(currentPixel, pixelColor);
      pixel.show();
      currentPixel++;
      if (currentPixel > PIXELCOUNT)
      {
        currentPixel = 0;
        pixelColor = random(0x000000, 0xFFFFFF);
      }
    }

    if (patternNumber == 1) // low traffic
    {
      pixelColor = random(0x00FF00, 0x00FF00);
      for (int i = 0; i < PIXELCOUNT; i++)
      {
        pixel.setPixelColor(currentPixel, pixelColor); // green range
        pixel.show();
      }
      currentPixel++;
      if (currentPixel > PIXELCOUNT)
      {
        currentPixel = 0;
      }
    }
    if (patternNumber == 2) // heavy traffic
    {
      pixelColor = random(0xFF0000, 0xFF0000);
      for (int i = 0; i < PIXELCOUNT; i++)
      {
        pixel.setPixelColor(currentPixel, pixelColor); // range
        pixel.show();
      }
      currentPixel++;
      if (currentPixel > PIXELCOUNT)
      {
        currentPixel = 0;
      }
    }
    if (patternNumber == 3) // optimal departure window
    {
      pixelColor = random(0xFFA500, 0xFFA500); // orange range
      for (int i = 0; i < PIXELCOUNT; i++)
      {
        pixel.setPixelColor(currentPixel, pixelColor);
        pixel.show();
      }
      currentPixel++;
      if (currentPixel > PIXELCOUNT)
      {
        currentPixel = 0;
      }
    }
    if (patternNumber == 4) // beyond optimal departure window
    {
      pixelColor = random(0x0000FF, 0x0000FF); // blue range
      for (int i = 0; i < PIXELCOUNT; i++)
      {
        pixel.setPixelColor(currentPixel, pixelColor);
        pixel.show();
      }
      currentPixel++;
      if (currentPixel > PIXELCOUNT)
      {
        currentPixel = 0;
      }
    }
    lastShowTime = millis();
  }
}

void handleResponse(const char *event, const char *data)
{
  Serial.printf("\n\nWebhook response...\n\n");

  Serial.printf("\ndata: %s\n", data);

  String nestedEvent = "";

  if (data)
  {

    JSONValue tomtomData = JSONValue::parseCopy(data);
    if (tomtomData.isValid())
    {
      JSONObjectIterator iterator(tomtomData);
      while (iterator.next())
      {
        if (iterator.name() == "travelTimeInSeconds")
        {
          travelTimeInSeconds = iterator.value().toInt();
        }
        else if (iterator.name() == "trafficDelayInSeconds")
        {
          trafficDelayInSeconds = iterator.value().toInt();
        }
        else if (iterator.name() == "routeDescription")
        {
          routeDescription = (const char *)iterator.value().toString();
        }
        else if (iterator.name() == "pixelBrightness")
        {
          pixelBrightness = iterator.value().toInt();
        }
        else if (iterator.name() == "targetHour")
        {
          targetHour = iterator.value().toInt();
        }
        else if (iterator.name() == "targetMinute")
        {
          targetMinute = iterator.value().toInt();
        }
      }

      int currentTimeinMinutes = Time.hour() * 60 + Time.minute(); // current time converted to minutes from midnight for calculating minutes to leave
      // int currentTimeinMinutes = 21 * 60 + 23; // current time converted to minutes from midnight for calculating minutes to leave

      int arriveByTimeinMinutes = targetHour * 60 + targetMinute; // calculate the arrive by time in minutes from midnight

      minutesToLeave = arriveByTimeinMinutes - currentTimeinMinutes - travelTimeInSeconds / 60;

      int currentArrivalTimeinMinutes = currentTimeinMinutes + travelTimeInSeconds / 60;

      int currentArrivalHour = (currentArrivalTimeinMinutes / 60) % 24;
      int currentArrivalMinute = currentArrivalTimeinMinutes % 60;

      if (targetHour == -2) //-2 specifies nighttime sequence from logic
      {
        pixelPattern = 0;
      }
      else
      {
        if (minutesToLeave < 60)
        {
          if (trafficDelayInSeconds / 60 < 10)
          {
            pixelPattern = 1; // low traffic pattern
          }
          else if (trafficDelayInSeconds / 60 > 10)
          {
            pixelPattern = 2; // high traffic pattern
          }

          if (minutesToLeave <= 15)
          {
            if (minutesToLeave > 0)
            {
              pixelPattern = 3; // optimal departure window
            }
            else
            { 
              if (targetHour == -1) //no specified arrival time
              {
                pixelPattern = 0;
              }
              else
              {
                pixelPattern = 4; //beyond optimal departure window
              }
            }
          }
        }
        else
        {
          pixelPattern = 0; // default ambient random colors
          Serial.printf("pixelPattern = 0\n");
        }
      }

      pixel.setBrightness(pixelBrightness); // set pixel brightness if received from logic

      display.clearDisplay();
      display.setCursor(0, 0);
      if (targetHour == -1) // no specified arrival time
      {
        display.setTextSize(1);
        display.printf("Time Now: %02i:%02i\n%s\nTravel time: %im %is\nTraffic: %im %is\nCurrent ETA: %02i:%02i\nArrive anytime...", Time.hour(), Time.minute(), routeDescription.c_str(), travelTimeInSeconds / 60, travelTimeInSeconds % 60, trafficDelayInSeconds / 60, trafficDelayInSeconds % 60, currentArrivalHour, currentArrivalMinute);
      }
      else if (targetHour == -2) // nighttime
      {
        display.setTextSize(2);
        display.printf("\n %02i:%02i \n Nighttime", Time.hour(), Time.minute());
      }
      else if (targetHour >= 0)
      {
        display.setTextSize(1);
        display.printf("Time Now: %02i:%02i\n%s\nLeave in: %im\nTravel time: %im %is\nTraffic: %im %is\nCurrent ETA: %02i:%02i\nTarget TA: %02i:%02i", Time.hour(), Time.minute(), routeDescription.c_str(), minutesToLeave, travelTimeInSeconds / 60, travelTimeInSeconds % 60, trafficDelayInSeconds / 60, trafficDelayInSeconds % 60, currentArrivalHour, currentArrivalMinute, targetHour, targetMinute);
      }
      display.display();
    }
  }
}