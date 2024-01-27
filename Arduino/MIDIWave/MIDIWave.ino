/*
  MIDIWave Project

  Author: Aziz Becha
  Credits to the contributors and the creators of the used libraries.
  Copyright 2024
*/


#include <MIDIUSB.h>
#include <MagicPot.h>

#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>

// Potentiometers properties
#define NUM_POTS 4
#define POTENTIOMETER_PINS {A0, A1, A2, A3}
#define POTENTIOMETER_MIN_READ 0
#define POTENTIOMETER_MAX_READ 127
#define POT_THRESHOLD 0 // Threshold for value change
#define UPDATE_INTERVAL 15 // Update interval in milliseconds

int potsValues[NUM_POTS] = {0};
int prevPotsValues[NUM_POTS] = {0};

MagicPot potentiometers[NUM_POTS] = {
  MagicPot(A0, POTENTIOMETER_MIN_READ, POTENTIOMETER_MAX_READ),
  MagicPot(A1, POTENTIOMETER_MIN_READ, POTENTIOMETER_MAX_READ),
  MagicPot(A2, POTENTIOMETER_MIN_READ, POTENTIOMETER_MAX_READ),
  MagicPot(A3, POTENTIOMETER_MIN_READ, POTENTIOMETER_MAX_READ)
};

// Buttons properties
const int numButtons = 8;

// Variables to track button states
int buttonStates[numButtons] = {0};
int previousButtonValues[numButtons] = {0};

// Multiplexer Pinout
#define CHAN 10
#define pinS0 15
#define pinS1 14
#define pinS2 16

// MIDI note and velocity values
const int notes[] = {60, 61, 62};       // MIDI note numbers (C4, C#4)
const int velocity = 127;  // Velocity (max)

// General MIDI settings
int channel = 1;
int note = 36;
int CC = 1;

Thread potsThread = Thread();
Thread buttonsThread = Thread();

void setup() {
  Serial.begin(115200);

  // Initialize the potentiometers
  for (int i = 0; i < NUM_POTS; i++) {
    potentiometers[i].begin();
    potentiometers[i].onChange(onPotentiometerChange);
  }

  // Initialize the button pins as inputs with pull-up resistors
  for (int i = 0; i < numButtons; i++) {
    // pinMode(buttonPins[i], INPUT_PULLUP);
    previousButtonValues[i] = HIGH;
  }

  // Set Multiplexer pins A, B and C as digital outs
  pinMode(pinS0, OUTPUT);
  pinMode(pinS1, OUTPUT);
  pinMode(pinS2, OUTPUT);

  // Initialize Threads
  potsThread.onRun(potentiometersJob);
  buttonsThread.onRun(buttonsJob);
}

void loop() {
  potsThread.run();
  buttonsThread.run();
}

void potentiometersJob(){
  static unsigned long lastUpdateTime = 0; // Time of the last update
  unsigned long currentTime = millis();

  // Check if the update interval has passed
  if (currentTime - lastUpdateTime >= UPDATE_INTERVAL) {
    for (int i = 0; i < NUM_POTS; i++) {
      potentiometers[i].read(POT_THRESHOLD);
      potsValues[i] = potentiometers[i].getValue();

      // Check if the value has changed
      if (potsValues[i] != prevPotsValues[i]) {
        Serial.println(String("Value Pot ") + (i + 1) + ": " + potsValues[i]);
        controlChange(channel, i + 1, potsValues[i]);
        prevPotsValues[i] = potsValues[i]; // Update the previous value
      }
    }

    lastUpdateTime = currentTime; // Update the last update time
  }
}

void buttonsJob() {
  // Read the button states
  for (int i = 0; i < numButtons; i++)
  {
    readChannel(i);

    // Check if the button state has changed
    if (buttonStates[i] != previousButtonValues[i]) {
      if (buttonStates[i] == LOW) {
        // Button pressed, send MIDI Note On
        noteOn(1, note+i, velocity);
      } else {
        // Button released, send MIDI Note Off
        noteOff(1, note+i, 0);
      }

      // Update previousButtonValues
      previousButtonValues[i] = buttonStates[i];

      // Add a small delay for button debouncing
      delay(1);
    }
  }
}

void readChannel(int channel) {
  // Set the multiplexer to the specified channel
  digitalWrite(pinS2, (channel & 0x04) ? HIGH : LOW);
  digitalWrite(pinS1, (channel & 0x02) ? HIGH : LOW);
  digitalWrite(pinS0, (channel & 0x01) ? HIGH : LOW);

  // Return the value from the selected channel
  buttonStates[channel] = analogRead(CHAN) == 255;
}

void onPotentiometerChange() {
  // Potentiometer value changed! You can add code here if needed.
}

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush();
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}