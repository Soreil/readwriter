//Channel generator

const uint8_t in = A0;
const uint8_t out = A1;

const int led[4] = { 2,3,4,5 };
const int dip[4] = { 9,8,7,6 };

const unsigned long syncTime = 15; //ms, ulong is same as Arduino libraries use for time.
const unsigned long bitTime = 9; //ms

const int readStart = 0;
const int writeStart = 4;
const int messageLength = 4;

void pciSetup(uint8_t pin)
{
	*digitalPinToPCMSK(pin) |= bit(digitalPinToPCMSKbit(pin));  // enable pin
	PCIFR |= bit(digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
	PCICR |= bit(digitalPinToPCICRbit(pin)); // enable interrupt for the group 
}

void setup() {
	pinMode(in, INPUT);
	pinMode(out, OUTPUT);
	pinMode(led[0], OUTPUT);
	pinMode(led[1], OUTPUT);
	pinMode(led[2], OUTPUT);
	pinMode(led[3], OUTPUT);

	pinMode(dip[0], INPUT_PULLUP);
	pinMode(dip[1], INPUT_PULLUP);
	pinMode(dip[2], INPUT_PULLUP);
	pinMode(dip[3], INPUT_PULLUP);

	pciSetup(in);
	Serial.begin(115200);
}

volatile unsigned long time = millis();
volatile int edgeCount = 0;

ISR(PCINT1_vect) {
	interrupts();
	Serial.println(edgeCount);

	if (digitalRead(in) == HIGH && millis() - time >= 12) {
		edgeCount = 0;
		time = millis();
		return;
	}

	if (edgeCount/2 >= readStart && edgeCount/2 < readStart + messageLength) {
		if (digitalRead(in) == LOW) {
			digitalWrite(led[edgeCount/2 - readStart], (millis() - time) > 4);
			edgeCount++;
			time = millis();
			return;
		}
	}
	if (edgeCount/2 >= writeStart && edgeCount/2 < writeStart + messageLength) {
		if (digitalRead(in) == HIGH) {
			if (digitalRead(dip[edgeCount - writeStart]) == HIGH) {
				unsigned long writeDuration = millis();
				//while (millis() - writeDuration < (bitTime / 4) * 3) {
				//	Serial.print("Line start value:");
				//	Serial.println(digitalRead(out));
				//	digitalWrite(out, digitalRead(dip[edgeCount / 2 - writeStart]));
				//	Serial.print("Line end value:");
				//	Serial.println(digitalRead(out));
				//}
				digitalWrite(out, HIGH);
				delay(6);
			}
			digitalWrite(out, LOW);
			edgeCount++;
			time = millis();
			return;
			}
	}

	edgeCount++;
	time = millis();
}

void loop() {
}