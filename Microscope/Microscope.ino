#include <FastLED.h>
#include <Stepper.h>

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

// Stepper motor configuration
#define STEPS_PER_REV 2048
#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11
Stepper stepper(STEPS_PER_REV, IN1, IN3, IN2, IN4);

// Edge detection switch configuration
#define SWITCH_PIN 12
bool switchState = false;
bool lastSwitchState = false;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// Sample configuration
#define SAMPLE_DISTANCE 4300 // Number of steps between each sample, adjust for fine-tuning
#define BACK_ROTATIONS 0.2 // Number of rotations to move back after homing
int currentSample = 0;

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

void moveToSample(int targetSample) {
  int stepsToMove = (targetSample - currentSample) * SAMPLE_DISTANCE;
  stepper.step(stepsToMove);
  currentSample = targetSample;
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

  stepper.setSpeed(10); // Set motor speed (RPM)
  pinMode(SWITCH_PIN, INPUT_PULLUP);
}

void home() {
  while (digitalRead(SWITCH_PIN) == HIGH) {
    stepper.step(1);
    delay(1);
  }

  // Move back a few rotations after homing
  int stepsToMoveBack = STEPS_PER_REV * BACK_ROTATIONS;
  stepper.step(-stepsToMoveBack);
  
  currentSample = 5;
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
      Serial.print("Top lights off\n\r");
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

    
    // Stepper motor commands
    if (command == "motor cw") {
      Serial.print("Stepper motor clockwise\n\r");
      stepper.step(STEPS_PER_REV);
      return;
    }

    if (command == "motor ccw") {
      Serial.print("Stepper motor counterclockwise\n\r");
      stepper.step(-STEPS_PER_REV);
      return;
    }

    // Stepper motor homing command
    if (command == "home") {
      Serial.print("Homing stepper motor\n\r");
      home();
      return;
    }

    if (command.startsWith("sample ")) {
      int targetSample = command.substring(7).toInt();
      Serial.print("Moving to sample ");
      Serial.print(targetSample);
      Serial.print("\n\r");
      moveToSample(targetSample);
      return;
    }

    Serial.print("Got ");
    Serial.print(command);
    Serial.print("\n\r");
}

void loop()
{   
  // Read and debounce switch state
  bool reading = digitalRead(SWITCH_PIN);

  if (reading != lastSwitchState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != switchState) {
      switchState = reading;
    }
  }
  
  lastSwitchState = reading;

  // Process serial commands
  if (Serial.available() > 0) {
    command = Serial.readString();
    command.trim();

    interpret(command);
    delay(500);

    Serial.print("done\n\r");
  }
}
