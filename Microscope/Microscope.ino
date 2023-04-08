#include <FastLED.h>
#include <AccelStepper.h>

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

// A4988 stepper motor configuration
#define DIR_PIN 11
#define STEP_PIN 10
#define STEPS_PER_REV 2048

// AccelStepper motor initialization
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// Edge detection switch configuration
#define SWITCH_PIN 12
bool switchState = false;
bool lastSwitchState = false;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// Sample configuration
#define SAMPLE_DISTANCE 420 // Number of steps between each sample, adjust for fine-tuning
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

void setup() {
  Serial.begin(9600);
  Serial.print("Microscope is running.\n\r");

  FastLED.addLeds<LED_TYPE, BED_LED_PIN, COLOR_ORDER>(bedLeds, NUM_BED_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(brightness);

  FastLED.addLeds<LED_TYPE, TOP_LED_PIN, COLOR_ORDER>(topLeds, NUM_TOP_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(brightness);

  bedOff();
  topOff();

  pinMode(SWITCH_PIN, INPUT_PULLUP);

  // A4988 stepper motor initialization
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  
  // AccelStepper configuration
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500); // Set desired acceleration

  home();
}

void moveSteps(int steps) {
  stepper.move(steps);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
}

void moveToSample(int targetSample) {
  int stepsToMove = (targetSample - currentSample) * SAMPLE_DISTANCE;
  moveSteps(-stepsToMove);

  currentSample = targetSample;
}

void home() {
  digitalWrite(DIR_PIN, LOW); // Set the direction to home
  while (digitalRead(SWITCH_PIN) == HIGH) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(5000);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(5000);
  }
  delay(1000);
  moveSteps(100);
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
