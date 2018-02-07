/*
CLOCK TO TAP
Uses Arduino Leonardo bootloader and MIDIUSB library, oh and MIDI library (Francois Best)
*/

#include <MIDIUSB.h>
#include <MIDI.h>

const byte littleButtPins[4] = { 2,4,19,16 };
const byte bigButtPins[4] = { 20,9,8,7 };

MIDI_CREATE_DEFAULT_INSTANCE();

void noteOn(byte channel, byte pitch, byte velocity) {
	midiEventPacket_t noteOn = { 0x09, 0x90 | channel, pitch, velocity };
	MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
	midiEventPacket_t noteOff = { 0x08, 0x80 | channel, pitch, velocity };
	MidiUSB.sendMIDI(noteOff);
}

#define outA 18 //A0
#define outB 15 //15
#define outC 14 //14
#define outD 21 //A3
byte outs[4] = { outA, outB, outC, outD };

#define ledA 3 //D3
#define ledB 5 //D3
#define ledC 6 //D3
#define ledD 10 //D3
byte LEDs[4] = { ledA, ledB, ledC, ledD };

bool inversion[4] = { false,false,false,false };

void setup() {
	MIDI.setHandleClock(clockTick);
	MIDI.setHandleStart(handleStart);
	MIDI.setHandleStop(handleStop);
	MIDI.begin(MIDI_CHANNEL_OMNI); // Initiate MIDI communications, listen to all channels
								   //Serial.begin(115200);
	for (int i = 0; i < 4; i++) {
		pinMode(outs[i], OUTPUT);
		pinMode(LEDs[i], OUTPUT);
		pinMode(littleButtPins[i], INPUT_PULLUP);
		pinMode(bigButtPins[i], INPUT_PULLUP);
		digitalWrite(LEDs[i], HIGH);
	}
	delay(500);
	allLedsOff();

}

void allLedsOn() {
	for (int i = 0; i < 4; i++) {
		digitalWrite(LEDs[i], HIGH);
	}
}

void allLedsOff() {
	for (int i = 0; i < 4; i++) {
		digitalWrite(LEDs[i], LOW);
	}
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).






byte clockLengths[4] = { 48, 24, 24, 24 }; //24 = 4/4   16 = triplets
unsigned long clockIncrement = 0;
byte clockDivisors[4] = { 1, 1, 1, 1 };
bool taps[4] = { false, false, false, false };
bool isRunning = false;
bool clockUpdated = false;

void clockTick() {
	clockIncrement++;
	//clockUpdated = true;
	Serial.println(clockIncrement);
	handleTaps();
}

void handleStop() {
	Serial.println("STOP RESET");
	isRunning = false;
	allLedsOff();
}

void handleStart() {
	Serial.println("START");
	isRunning = true;
	clockIncrement = 0;
	clockTick();
}

void handleTaps() {

	for (int i = 0; i < 4; i++) {
		int localTapTimer = clockIncrement % clockLengths[i];
		Serial.println(taps[i]);


		if (localTapTimer == 0) {
			taps[i] = true;
			digitalWrite(outs[i], !inversion[i]);
			digitalWrite(LEDs[i], inversion[i]);

		}
		else if (localTapTimer == 1) {
			taps[i] = false;
			digitalWrite(outs[i], LOW);
			digitalWrite(LEDs[i], LOW);
		}
	}
}



void loop() {
	MIDI.read();
	midiEventPacket_t rx;
	do {
		rx = MidiUSB.read();
		if (rx.header != 0) {
			if (rx.header == 15) {
				if (rx.byte1 == 248) {
					clockTick();
				}
				else if (rx.byte1 == 252) {

					handleStop();
				}
			}
			else if (rx.header == 3 && rx.byte1 == 242) {

			}
			
			Serial.print("Received: ");
			Serial.print(rx.header);
			Serial.print("-");
			Serial.print(rx.byte1);
			Serial.print("-");
			Serial.print(rx.byte2);
			Serial.print("-");
			Serial.println(rx.byte3);
			
		}
	} while (rx.header != 0);
	handleButts();

}

bool littleButtStates[4] = { true,true,true,true };
bool oldLittleButtStates[4] = { false,false,false,false };
bool bigButtStates[4] = { true,true,true,true, };
bool oldBigButtStates[4] = { false,false,false,false };


int temp = 0;
void handleButts() {
	//Serial.println("poop");
	for (int i = 0; i < 4; i++) {
		oldLittleButtStates[i] = littleButtStates[i];
		littleButtStates[i] = digitalRead(littleButtPins[i]);
		if (!littleButtStates[i] && oldLittleButtStates[i]) {
			inversion[i] = !inversion[i];
			Serial.print("inversion ");
			Serial.print(i);
			Serial.print(" = ");
			Serial.println(inversion[i]);
		}
		else if (littleButtStates[i] && !oldLittleButtStates[i]) {
			
		}
	}
	for (int i = 0; i < 4; i++) {
		oldBigButtStates[i] = bigButtStates[i];
		bigButtStates[i] = digitalRead(bigButtPins[i]);
		if (!bigButtStates[i] && oldBigButtStates[i]) {
			
		}
		else if (bigButtStates[i] && !oldBigButtStates[i]) {
			
		}
	}
}