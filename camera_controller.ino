/**
 * @brief Program for arduino Leonardo using 2 2-Axis joysticks and 2 potenciometers
 * as a MIDI device to be used as a controller for VFX editing.
 * @author Kristyna Kumpanova (kumpakri@gmail.com)
 * @date 18.05.2021
 */

#include "MIDIUSB.h"
#define CHANNEL 0

// pins
const byte pinJoy1X = 0;  //A0
const byte pinJoy1Y = 1;  //A1
const byte pinJoy2X = 2;  //A2
const byte pinJoy2Y = 3;  //A3
const byte pinKnob1 = 4;  //A4
const byte pinKnob2 = 5;  //A5

// controls
const byte ctrlJoy1Left = 0;
const byte ctrlJoy1Right = 1;
const byte ctrlJoy1Down = 2;
const byte ctrlJoy1Up = 3;
const byte ctrlJoy2Left = 4;
const byte ctrlJoy2Right = 5;
const byte ctrlJoy2Down = 6;
const byte ctrlJoy2Up = 7;
const byte ctrlKnob1 = 8;
const byte ctrlKnob2 = 9;

// other global parameters
int knob1LastValue;
int knob2LastValue;

// viz https://github.com/arduino-libraries/MIDIUSB/
void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

/**
 * Converts raw analog joystick readings to MIDI range (0x00 to 0x7F).
 * @param analogVal raw joystick reading
  */
int analogJoy2MIDIrange(double analogVal)
{
  return (abs(analogVal - 512) / 512) * 0x7F;
}

/**
 * Converts raw analog knob readings to MIDI range (0x00 to 0x7F).
 * @param analogVal raw knob reading
  */
int analogKnob2MIDIrange(double analogVal)
{
  return (analogVal / 1023) * 0x7F;
}

/**
 * @brief checks analog value on joystick X and Y axis, sends out the value converted to MIDI range 
 * as a MIDI command if the value is not in the dead zone (510 to 520). The dead zone is located 
 * around the joystick stable position at around 515 (theoretically should be 512 but experience 
 * proved otherwise).
 * @param pinX pin for X-axis joystick data
 * @param pinY pin for Y-axis joystick data
 * @param controlLeft controller number for X-axis movemet in one direction 
 * @param controlRight controller number for X-axis movemet in the other direction
 * @param controlDown controller number for Y-axis movemet in one direction 
 * @param controlUp controller number for Y-axis movemet in the other direction
 */
void updateJoystick2Axis(byte pinX, byte pinY, byte controlLeft, byte controlRight, byte controlDown, byte controlUp)
{
  int valueX = analogRead(pinX);
  int valueY = analogRead(pinY);
  
  if(valueX < 510) //left
  {
    controlChange(CHANNEL, controlLeft, analogJoy2MIDIrange(valueX));
    MidiUSB.flush();
  }
  else if (valueX > 520) //right
  {
    controlChange(CHANNEL, controlRight, analogJoy2MIDIrange(valueX)); 
    MidiUSB.flush(); 
  }
  if(valueY < 400) //down
  {
    controlChange(CHANNEL, controlDown, analogJoy2MIDIrange(valueY));
    MidiUSB.flush();
  }
  else if (valueY > 600) //up
  {
    controlChange(CHANNEL, controlUp, analogJoy2MIDIrange(valueY));
    MidiUSB.flush();
  }
}

/**
 * Checks analog value on the knob and sends its converted value as MIDI
 * command if the value differs from the last sent value.
 */
void updateKnob(byte pinKnob, byte control, int & lastValue)
{
  int value = analogRead(pinKnob);
  if (abs(value - lastValue) > 2)
  {
    controlChange(CHANNEL, control, analogKnob2MIDIrange(value));
    MidiUSB.flush();
    lastValue = value;
  }
}

void setup() {
  Serial.begin(9600);
  knob1LastValue = 0;
  knob2LastValue = 0;
}

void loop() {
  updateJoystick2Axis(pinJoy1X, pinJoy1Y, ctrlJoy1Left, ctrlJoy1Right, ctrlJoy1Down, ctrlJoy1Up);
  updateJoystick2Axis(pinJoy2X, pinJoy2Y, ctrlJoy2Left, ctrlJoy2Right, ctrlJoy2Down, ctrlJoy2Up);
  updateKnob(pinKnob1, ctrlKnob1, knob1LastValue);
  updateKnob(pinKnob2, ctrlKnob2, knob2LastValue);
  // TODO: is delay needed?
}
