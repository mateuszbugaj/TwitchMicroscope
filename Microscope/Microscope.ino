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
#define SLEEP_PIN 9

// AccelStepper motor initialization
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// Edge detection switch configuration
#define SWITCH_PIN 12
bool switchState = false;
bool lastSwitchState = false;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 20;

// Sample configuration
#define SAMPLE_DISTANCE 1680 // Number of steps between each sample, adjust for fine-tuning
#define BACK_ROTATIONS 0.2 // Number of rotations to move back after homing
int currentSample = 0;

#define SAMPLE_SWITCH_PIN 8
#define SAMPLE_SWITCH_STATE_INDICATOR 7
bool sampleButtonPressed = LOW;
unsigned long lastPressed = 0; // Timestamp
bool sampleButtonClickSeen = false;
bool sampleSwitchState = true;

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
  pinMode(SAMPLE_SWITCH_PIN, INPUT_PULLUP);
  pinMode(SAMPLE_SWITCH_STATE_INDICATOR, OUTPUT);

  // A4988 stepper motor initialization
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(SLEEP_PIN, OUTPUT);
  
  // AccelStepper configuration
  stepper.setMaxSpeed(800);
  stepper.setAcceleration(600); // Set desired acceleration

  home();
}

void moveSteps(int steps) {
  digitalWrite(SLEEP_PIN, HIGH);
  stepper.move(steps);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
  digitalWrite(SLEEP_PIN, LOW);
}

void checkSampleSwitch(){
  int reading = digitalRead(SAMPLE_SWITCH_PIN);

  if(sampleButtonPressed == false && reading == 0){
    sampleButtonPressed = true;
    lastPressed = millis();
  }

  if(reading == 1){
    sampleButtonPressed = false;
    sampleButtonClickSeen = false;
  }

  if(millis() - lastPressed > debounceDelay && sampleButtonPressed == true){
    if(sampleButtonClickSeen == false){
      sampleSwitchState = !sampleSwitchState;
      sampleButtonClickSeen = true;
      Serial.println(sampleSwitchState);
    }
  }
}


void moveToSample(int targetSample) {
  if(sampleSwitchState == true) { 
    int stepsToMove = (targetSample - currentSample) * SAMPLE_DISTANCE;
    moveSteps(-stepsToMove);
    currentSample = targetSample;
  } else {
    Serial.println("Sample switch not activated!");
  }
}

void home() {
  digitalWrite(DIR_PIN, LOW); // Set the direction to home
  digitalWrite(SLEEP_PIN, HIGH);
  while (digitalRead(SWITCH_PIN) == HIGH) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(1000);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(1000);
  }
  digitalWrite(SLEEP_PIN, LOW);
  delay(500);
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

  checkSampleSwitch();

  if(sampleSwitchState){
    digitalWrite(SAMPLE_SWITCH_STATE_INDICATOR, LOW);
  } else {
    digitalWrite(SAMPLE_SWITCH_STATE_INDICATOR, HIGH);
  }

  // Process serial commands
  if (Serial.available() > 0) {
    command = Serial.readString();
    command.trim();

    interpret(command);
    delay(500);

    Serial.print("done\n\r");
  }
}
