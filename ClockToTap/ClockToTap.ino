/*
CLOCK TO TAP
Uses Arduino Leonardo bootloader and MIDIUSB library, oh and MIDI library (Francois Best)
*/

#include <ButtonDebounce.h>
#include <MIDIUSB.h>
#include <MIDI.h>

bool littleButtStates[4] = { true,true,true,true };
bool oldLittleButtStates[4] = { false,false,false,false };
bool bigButtStates[4] = { true,true,true,true, };
bool oldBigButtStates[4] = { false,false,false,false };

bool flippedTrips[4] = { false,false,false,false };
unsigned long tripTimer[4] = { 0,0,0,0 };
bool triplets[4] = { false,false,false,false };
//unsigned int bigButtTimer[4] = { 0,0,0,0 };
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

unsigned long bigDebounceTimers[4] = { 0,0,0,0 };
unsigned long smallDebounceTimers[4] = { 0,0,0,0 };
bool bigDebounceReady[4] = { true,true,true,true };
bool smallDebounceReady[4] = { true,true,true,true };


const int debounceThresh = 10;

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






byte clockLengths[4] = { 24, 24, 24, 24 }; //24 = 4/4   16 = triplets
unsigned long clockIncrement = 0;
byte clockDivisors[4] = { 1, 1, 1, 1 };
bool taps[4] = { false, false, false, false };
bool isRunning = false;
bool clockUpdated = false;

void clockTick() {
	clockIncrement++;
	//clockUpdated = true;
	//Serial.println(clockIncrement);
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
	handleTaps();
	//clockTick();
}
byte tapBlinkLength = 3;
int localTapTimer[4] = { 0,0,0,0 };
void handleTaps() {

	for (int i = 0; i < 4; i++) {

		localTapTimer[i] = clockIncrement % clockLengths[i];
		//	Serial.println(taps[i]);


		if (localTapTimer[i] == 0) {
			taps[i] = true;
			digitalWrite(outs[i], !inversion[i]);
			if (!bigButtStates[i]) {
				digitalWrite(LEDs[i], !inversion[i]);
			}
		}

		else if (localTapTimer[i] == tapBlinkLength) {
			taps[i] = false;
			digitalWrite(outs[i], inversion[i]);
			if (!bigButtStates[i]) {
				digitalWrite(LEDs[i], inversion[i]);
			}
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
			else if (rx.header == 3 && rx.byte1 == 242) { // a start 
				handleStart();

			}
			/*
			Serial.print("Received: ");
			Serial.print(rx.header);
			Serial.print("-");
			Serial.print(rx.byte1);
			Serial.print("-");
			Serial.print(rx.byte2);
			Serial.print("-");
			Serial.println(rx.byte3);
			*/
		}
	} while (rx.header != 0);
	handleButts();
	handleBlinks();
}




int temp = 0;
void handleButts() {
	for (int i = 0; i < 4; i++) {
		oldLittleButtStates[i] = littleButtStates[i];

		if (smallDebounceReady[i]) {
			littleButtStates[i] = !digitalRead(littleButtPins[i]);
		}
		else {
			debounce(0, i);
		}

		if (littleButtStates[i] && !oldLittleButtStates[i]) {
			inversion[i] = !inversion[i];
			smallDebounceTimers[i] = millis();
			smallDebounceReady[i] = false;


			if (localTapTimer[i] > tapBlinkLength) { //set LED imediately to show respect

				digitalWrite(LEDs[i], inversion[i]);

			}
			else {
				digitalWrite(LEDs[i], !inversion[i]);
			}






			Serial.print("inversion ");
			Serial.print(i);
			Serial.print(" = ");
			Serial.println(inversion[i]);
		}
		else if (!littleButtStates[i] && oldLittleButtStates[i]) {
			smallDebounceTimers[i] = millis();
			smallDebounceReady[i] = false;
		}
	}

	for (int i = 0; i < 4; i++) {
		oldBigButtStates[i] = bigButtStates[i];
		if (bigDebounceReady[i]) {
			bigButtStates[i] = !digitalRead(bigButtPins[i]);
		}
		else {
			debounce(1, i);
		}

		//Serial.println(flippedTrips[i]);



		if (bigButtStates[i] && !oldBigButtStates[i]) {
			digitalWrite(LEDs[i], HIGH);
			tripTimer[i] = millis();
			flippedTrips[i] = false;
			bigDebounceTimers[i] = millis();
			bigDebounceReady[i] = false;

			clockDivisors[i]++;
			clockDivisors[i] = clockDivisors[i] % 4;
			setClockLengths(i);

		}
		else if (!bigButtStates[i] && oldBigButtStates[i]) {
			bigDebounceTimers[i] = millis();
			bigDebounceReady[i] = false;
			digitalWrite(LEDs[i], LOW);
		}

		if (bigButtStates[i]) {

			if (millis() - tripTimer[i] > 300 && !flippedTrips[i]) {
				Serial.println(millis() - tripTimer[i]);
				triplets[i] = !triplets[i];
				flippedTrips[i] = true;
				clockDivisors[i]--;
				if (clockDivisors[i] < 0) {
					clockDivisors[i] = 3;
				}
				setClockLengths(i);
				if (triplets[i]) {
					blink(i, 3);
				}
				else {
					blink(i, 1);
				}


				for (int pop = 0; pop < 4; pop++) {
					Serial.print("triplets ");
					Serial.print(pop);
					Serial.print(" = ");
					Serial.println(triplets[pop]);
				}
			}
		}
	}
}

void setClockLengths(byte tap) {
	if (!triplets[tap]) {
		clockLengths[tap] = 48 >> clockDivisors[tap];
	}
	else {
		clockLengths[tap] = 32 >> clockDivisors[tap];
	}
}

void debounce(bool bigSmall, byte number) {
	/*Serial.print("deBouncing ");
	Serial.print(bigSmall);
	Serial.print(" - ");
	Serial.print(number);
	Serial.print(" - ");
	Serial.print(smallDebounceTimers[number]);
	Serial.print(" - ");
	Serial.println(millis());
	*/

	if (bigSmall) {  //if its a big button

		if (millis() - bigDebounceTimers[number] > debounceThresh) {
			bigDebounceReady[number] = true;
			//Serial.println("big debounce DONE");
		}
	}
	else {
		if (millis() - smallDebounceTimers[number] > debounceThresh) {
			smallDebounceReady[number] = true;
			//Serial.println("small debounce DONE");
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
				digitalWrite(LEDs[i], HIGH);
			}
			else if (millis() - blinkTimers[i] < desiredBlinkPeriod) {
				digitalWrite(LEDs[i], LOW);
				Serial.println(blinkCounter[i]);
			}
			else {
				blinkCounter[i]--;
				blinkTimers[i] = millis();
			}
		}

	}
}