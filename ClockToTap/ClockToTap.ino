/*
  CLOCK TO TAP
  Uses Arduino Leonardo bootloader and MIDIUSB library, oh and MIDI library (Francois Best)
*/

//#define proMicro

//#include <ButtonDebounce.h>
//#include <MIDIUSB.h>
#include <MIDI.h>

bool intClock = true;
bool notReceivedClockSinceBoot = true;
bool littleButtStates[4] = { true, true, true, true };
bool oldLittleButtStates[4] = { false, false, false, false };
bool bigButtStates[4] = { true, true, true, true, };
bool oldBigButtStates[4] = { false, false, false, false };
unsigned long lastTimeOfTap = 0;
unsigned long timeOfTap = 0;
int clockStepTimer = 0;
bool flippedTrips[4] = { false, false, false, false };
unsigned long tripTimer[4] = { 0, 0, 0, 0 };
bool triplets[4] = { false, false, false, false };
//unsigned int bigButtTimer[4] = { 0,0,0,0 };
const byte littleButtPins[4] = { PB0, PA6, PA2, PB9 };
const byte bigButtPins[4] = { PA10, PB6, PB7, PB8 };
byte waitLedSelect = 0;
unsigned long waitTimer = 0;
//int tapTimer = 0;
unsigned long tapTimer = 0;
unsigned long intClockTimer = 0;
int tock = 0;


MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, MIDI);


#ifdef proMicro
void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = { 0x09, 0x90 | channel, pitch, velocity };
  MidiUSB.sendMIDI(noteOn);
}


void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = { 0x08, 0x80 | channel, pitch, velocity };
  MidiUSB.sendMIDI(noteOff);
}

#endif


//these need fixing
#define outA PA1 //A0
#define outB PA4 //15
#define outC PA5 //14
#define outD PB1 //A3
byte outs[4] = { outA, outB, outC, outD };

#define ledA PA1 //D3
#define ledB PA4 //D3
#define ledC PA5 //D3
#define ledD PB1 //D3
byte LEDs[4] = { ledA, ledB, ledC, ledD };


unsigned long bigDebounceTimers[4] = { 0, 0, 0, 0 };
unsigned long smallDebounceTimers[4] = { 0, 0, 0, 0 };
bool bigDebounceReady[4] = { true, true, true, true };
bool smallDebounceReady[4] = { true, true, true, true };


const int debounceThresh = 10;


bool inversion[4] = { false, false, false, false };

void setup() {
  //
  MIDI.setHandleClock(MIDIClockTick);
  MIDI.setHandleStart(MIDIStart);
  MIDI.setHandleStop(handleStop);
  MIDI.begin(MIDI_CHANNEL_OMNI); // Initiate MIDI communications, listen to all channels
  //Serial.begin(115200);
  for (int i = 0; i < 4; i++) {
    //pinMode(outs[i], OUTPUT);
    //pinMode(LEDs[i], OUTPUT);
    pinMode(littleButtPins[i], INPUT_PULLUP);
    pinMode(bigButtPins[i], INPUT_PULLUP);
    ////digitalWrite(LEDs[i], HIGH);
    delay(200);
  }
  delay(500);
  // ////digitalWrite(LEdD, HIGH);
  // ////digitalWrite(LEdC, HIGH);
  // ////digitalWrite(LEdB, HIGH);
  // ////digitalWrite(LEdA, HIGH);
  // pinMode(PC13, OUTPUT);
  // digitalWrite(PC13, HIGH);
  // delay(1000);
  allLedsOff();
}

void allLedsOn() {
  for (int i = 0; i < 4; i++) {
    ////digitalWrite(LEDs[i], HIGH);
  }
}

void allLedsOff() {
  for (int i = 0; i < 4; i++) {
    ////digitalWrite(LEDs[i], LOW);
  }
}


void waiting4clock() {
  if (tapTimer > 0) {
    //intClock = true;
    handleIntClock();
  }
}


void handleIntClock() {
  //Serial.println("TapTimer = ");
  //Serial.println(tapTimer);
  //Serial.print("millis = ");
  //Serial.println(millis());

  if (tapTimer > 0) {
    //if (millis() - intClockTimer > tapTimer>>3) {
    if (millis() - intClockTimer > clockStepTimer) {
      //tock++;
      ////Serial.println(tock);
      clockTick();
      intClockTimer = millis();
    }
  }
}

void lightScroll() {
  if (millis() - waitTimer > 60) {
    waitTimer = millis();
    ////digitalWrite(LEDs[waitLedSelect], LOW);
    waitLedSelect++;
    waitLedSelect = waitLedSelect % 4;
    ////digitalWrite(LEDs[waitLedSelect], HIGH);
  }
}







byte clockLengths[4] = { 24, 24, 24, 24 }; //24 = 4/4   16 = triplets
unsigned long clockIncrement = 0;
byte clockDivisors[4] = { 1, 1, 1, 1 };
bool taps[4] = { false, false, false, false };
bool isRunning = false;
bool clockUpdated = false;

void MIDIClockTick() {
  intClock = false;
  notReceivedClockSinceBoot = false;
  clockTick();
}

void clockTick() {

  clockIncrement++;
  handleTaps();
}

void handleStop() {
  notReceivedClockSinceBoot = false;
  //Serial.println("STOP RESET");
  isRunning = false;
  allLedsOff();
}

void MIDIStart() {
  intClock = false;
  notReceivedClockSinceBoot = false;
  handleStart();
}

void handleStart() {
  notReceivedClockSinceBoot = false;
  //Serial.println("START");
  isRunning = true;
  clockIncrement = 0;
  handleTaps();
  //clockTick();
}
byte tapBlinkLength = 3;
int localTapTimer[4] = { 0, 0, 0, 0 };
void handleTaps() {
  ////Serial.println("hello");
  for (int i = 0; i < 4; i++) {

    localTapTimer[i] = clockIncrement % clockLengths[i];
    //  //Serial.println(taps[i]);


    if (localTapTimer[i] == 0) {
      taps[i] = true;
      digitalWrite(outs[i], !inversion[i]);
      if (!bigButtStates[i]) {
        ////digitalWrite(LEDs[i], !inversion[i]);
      }
    }

    else if (localTapTimer[i] == tapBlinkLength) {
      taps[i] = false;
      digitalWrite(outs[i], inversion[i]);
      if (!bigButtStates[i]) {
        ////digitalWrite(LEDs[i], inversion[i]);
      }
    }
  }
}


int aliveCounter = 0;
void loop() {
  aliveCounter++;
  if (notReceivedClockSinceBoot) {
    //waiting4clock();
  }

  if (intClock) {
    handleIntClock();
  }

  MIDI.read();


  handleButts();
  handleBlinks();
}




int temp = 0;
void setClockLengths(byte tap) {
  if (!triplets[tap]) {
    clockLengths[tap] = 48 >> clockDivisors[tap];
  }
  else {
    clockLengths[tap] = 32 >> clockDivisors[tap];
  }
}

void debounce(bool bigSmall, byte number) {
  /*//Serial.print("deBouncing ");
    //Serial.print(bigSmall);
    //Serial.print(" - ");
    //Serial.print(number);
    //Serial.print(" - ");
    //Serial.print(smallDebounceTimers[number]);
    //Serial.print(" - ");
    //Serial.println(millis());
  */

  if (bigSmall) {  //if its a big button

    if (millis() - bigDebounceTimers[number] > debounceThresh) {
      bigDebounceReady[number] = true;
      ////Serial.println("big debounce DONE");
    }
  }
  else {
    if (millis() - smallDebounceTimers[number] > debounceThresh) {
      smallDebounceReady[number] = true;
      ////Serial.println("small debounce DONE");
    }
  }
}

unsigned long blinkTimers[4];
byte blinkCounter[4] = { 0, 0, 0, 0 };

void blink(byte led, byte times) {
  blinkCounter[led] = times + 1;
  blinkTimers[led] = millis();
}

int desiredBlinkPeriod = 100;
//bool onTimerSetButNotReached[4] = { false, false, false, false };
void handleBlinks() {
  for (int i = 0; i < 4; i++) {
    if (blinkCounter[i] > 0) {
      if (millis() - blinkTimers[i] < desiredBlinkPeriod >> 1) { //if we are b4 halfway point
        ////digitalWrite(LEDs[i], HIGH);
      }
      else if (millis() - blinkTimers[i] < desiredBlinkPeriod) {
        ////digitalWrite(LEDs[i], LOW);
        //Serial.println(blinkCounter[i]);
      }
      else {
        blinkCounter[i]--;
        blinkTimers[i] = millis();
      }
    }
  }
}




void proMicroUSB() {
#ifdef proMicro

  // First parameter is the event type (0x0B = control change).
  // Second parameter is the event type, combined with the channel.
  // Third parameter is the control number number (0-119).
  // Fourth parameter is the control value (0-127).
  midiEventPacket_t rx;
  do {
    rx = MidiUSB.read();
    if (rx.header != 0) {
      if (rx.header == 15) {
        if (rx.byte1 == 248) {

          MIDIClockTick();
        }
        else if (rx.byte1 == 252) {

          handleStop();
        }
      }
      else if (rx.header == 3 && rx.byte1 == 242) { // a start
        intClock = false;
        notReceivedClockSinceBoot = false;
        handleStart();

      }
      /*
        //Serial.print("Received: ");
        //Serial.print(rx.header);
        //Serial.print("-");
        //Serial.print(rx.byte1);
        //Serial.print("-");
        //Serial.print(rx.byte2);
        //Serial.print("-");
        //Serial.println(rx.byte3);
      */
    }
  } while (rx.header != 0);
#endif
}

