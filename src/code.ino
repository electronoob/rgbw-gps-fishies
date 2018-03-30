#include <SPI.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include <Adafruit_GFX.h>
#include <SoftwareSerial.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

char name[32] = "rainbow fish";
char time[32] = "ERROR: unknown time";
char status[32] = "dildowagon";
#define OLED_RESET 7
Adafruit_SSD1306 display(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#ifdef __AVR__
  #include <avr/power.h>
#endif
#define PIN 6
#define NUM_LEDS 9
#define BRIGHTNESS 255
#define SPEED 15

static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRBW + NEO_KHZ800);
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

//for messages to the oled (tmp)
String tmp;

void setup()
{
  Serial.begin(115200);
  ss.begin(GPSBaud);
  //start ssd1306
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  // init done
  display.clearDisplay();
  display.display();

  //end ssd1306
  updateStatus( "INFO: Starting up." );
  //
  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  colorWipe(strip.Color(255, 0, 0), SPEED); // Red
  colorWipe(strip.Color(0, 255, 0), SPEED); // Green
  colorWipe(strip.Color(0, 0, 255), SPEED); // Blue
  colorWipe(strip.Color(0, 0, 0, 255), SPEED); // White
}
// times
// 06-08, red.
// 08-22, white.
// 22-00, blue.
// 00-06, off;

int hourLastSeen = -1;
void loop()
{
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    updateStatus("ERROR: No GPS Module: check wiring.");
    render();
    return;
  }
  render();
  while (ss.available() > 0)
  {
    if (gps.encode(ss.read()))
    {
      if (gps.date.isValid())
      {
        if(!gps.location.isValid())
        {
          updateStatus("No GPS Location.");
          updateTime("ERROR");
          return;
        }
        int i = gps.time.hour();
        if(hourLastSeen != i)
        {
          char tmp2[32] = "*****************";
          sprintf(tmp2, "%02d:%02d:%02d ", gps.time.hour(), gps.time.minute(), gps.time.second());
          updateTime((String)gps.location.isValid());
          
          // 00-06, off;
          //
          
          if( (i>=0)&&(i<6) )
          {
            colorWipe(strip.Color(0, 0, 0, 0), 0); // off I think.
            updateStatus("00-06 lights out.");
          }
          //06-08, red.
          if( (i>=6)&&(i<8) )
          {
            colorWipe(strip.Color(255, 0, 0), SPEED); // Red
            updateStatus("06-08 red.");
          }
          // 08-22, white.
          if( (i>=8)&&(i<22) )
          {
            colorWipe(strip.Color(0, 0, 0, 255), SPEED); // White
            updateStatus("08-22 white.");
          }
          // 22-00, blue.
          if(i>=22)
          {
            colorWipe(strip.Color(0, 0, 255), SPEED); // Blue
            updateStatus("22-00 blue.");
          }
          hourLastSeen = i;
        }
      }
      else
      {
        updateStatus("ERROR: no time and date.");
      }
    }
  }


}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void updateStatus(String message) {
  message.toCharArray(status, message.length()+1);
  Serial.println(message);
  render();
  delay(1000);
}
void updateTime(String message) {
  message.toCharArray(time, message.length()+1);
  Serial.println(message);
  render();
  delay(1000);
}

void render() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(name);
  display.setTextColor(BLACK, WHITE); // 'inverted' text
  display.println(time);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(status);
  display.display();
  delay(2000);
}
