/*
  MIDIWave Project

  Author: Aziz Becha
  Credits to the contributors and the creators of the used libraries.
  Copyright 2024
*/


#include <MIDIUSB.h>
#include <MagicPot.h>

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

// General MIDI settings
int channel = 1;
int note = 36;
int CC = 1;

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < NUM_POTS; i++) {
    potentiometers[i].begin();
    potentiometers[i].onChange(onPotentiometerChange);
  }
}

void loop() {
  static unsigned long lastUpdateTime = 0; // Time of the last update
  unsigned long currentTime = millis();

  // Check if the update interval has passed
  if (currentTime - lastUpdateTime >= UPDATE_INTERVAL) {
    for (int i = 0; i < 1; i++) {
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