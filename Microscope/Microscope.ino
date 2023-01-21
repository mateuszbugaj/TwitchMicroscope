#include <FastLED.h>

#define NUM_BED_LEDS 10
#define BED_LED_PIN     3

#define NUM_TOP_LEDS 3
#define TOP_LED_PIN     4

#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB bedLeds[NUM_BED_LEDS];
CRGB topLeds[NUM_TOP_LEDS];
int brightness = 255;
String command = "";

bool isTopOn = false;
bool isBedOn = false;

void bedOn(){
  for( int i = 0; i < NUM_BED_LEDS; ++i) {
    bedLeds[i] = CRGB::Gray;
  }
  FastLED.show();
  isBedOn = true;
}

void bedOff(){
  for( int i = 0; i < NUM_BED_LEDS; ++i) {
    bedLeds[i] = CRGB::Black;
  }
  FastLED.show();
  isBedOn = false;
}


void topOn(){
  for( int i = 0; i < NUM_TOP_LEDS; ++i) {
    topLeds[i] = CRGB::Gray;
  }
  FastLED.show();
  isTopOn = true;
}

void topOff(){
  for( int i = 0; i < NUM_TOP_LEDS; ++i) {
    topLeds[i] = CRGB::Black;
  }
  FastLED.show();
  isTopOn = false;
}

void setup() {
  Serial.begin(9600);
  Serial.print("Microscope is running.\n\r");

  FastLED.addLeds<LED_TYPE, BED_LED_PIN, COLOR_ORDER>(bedLeds, NUM_BED_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(brightness);

  FastLED.addLeds<LED_TYPE, TOP_LED_PIN, COLOR_ORDER>(topLeds, NUM_TOP_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(brightness);

  bedOff();
  topOff();
}

void interpret(String command){
    if(command == "lgt bed on"){
      Serial.print("Bed lights on\n");
      Serial.print("Brightness: ");
      Serial.print(brightness);
      Serial.print("\n\r");
      bedOn();
      return;
    }

    if(command == "lgt bed off"){
      Serial.print("Bed lights off\n\r");
      bedOff();
      return;
    }

    if(command == "lgt top on"){
      Serial.print("Top lights on\n");
      Serial.print("Brightness: ");
      Serial.print(brightness);
      Serial.print("\n\r");
      topOn();
      return;
    }

    if(command == "lgt top off"){
      Serial.print("Bed lights off\n\r");
      topOff();
      return;
    }

    
    if(command.indexOf("lgt force") == 0){
      int newBrightness = command.substring(10, command.length()).toInt();
      Serial.println(newBrightness);

      brightness = newBrightness;
      FastLED.setBrightness(brightness);

      if(isBedOn){
        bedOn();
      }

      if(isTopOn){
        topOn();
      }
      
      Serial.print("Light brightness set to " + String(newBrightness) + "\n\r");
      return;
    }

    Serial.print("Got ");
    Serial.print(command);
    Serial.print("\n\r");
}

void loop()
{   
  if (Serial.available() > 0) {
    command = Serial.readString();
    command.trim();


     interpret(command);
    delay(500);

    Serial.print("done\n\r");
  }
}
