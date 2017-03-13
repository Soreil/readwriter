//Channel generator

//Input pin on arduino
const uint8_t in = A0;
//Output pin on Arduino
const uint8_t out = A1;

//LED output pins
const int led[4] = { 2,3,4,5 };

//Dipswitches for what to write on the wire
const int dip[4] = { 12,8,7,6 };

const unsigned long syncTime = 15; //ms, ulong is same as Arduino libraries use for time.
const unsigned long bitTime = 9; //ms

//Bit on which we start reading
const int readStart = 92;
//Bit on which we start writing
const int writeStart = 4;
//Amount of bits to read and write
const int messageLength = 4;
//Wheter or not we are working in tandem with another group, @INCOMPLETE
const bool groupversion = false;

//Character read in from serial console.
char ourChar = '\0';

//Makes a non hardware interrupt pin behave as interrupt
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
	//Read character for lab 3
	Serial.readBytes(&ourChar, 1);
}

//Time of last fired ISR.
volatile unsigned long time = millis();

//Edgecount is how many times we had an edge trigger fire the ISR. From this value we can derive the current bit number and position in the bit. provided it is accurate.
volatile int edgeCount = 0;

//How to deal with variable width pulse train:
//	Always read and safe last 4 bits
//	When we detect a sync pulse display the last 4 bits(counter)
ISR(PCINT1_vect) {

	//Enabling interrupts while already in an interrupt can give issues, however I want to be sure to catch all state changes on the pin we monitor.
	interrupts();
	auto NewTime = millis();

	//We have a sync pulse! 12 should be more than plenty time.
	if (digitalRead(in) == HIGH && NewTime - time >= 12) {
		edgeCount = 0;
		time = NewTime;
		//return;
	}

	//We are in our range of bits to read luckily
	if (edgeCount/2 >= readStart && edgeCount/2 < readStart + messageLength) {
		//Serial.print("Reading wire");
		//Serial.println(edgeCount);
		if (digitalRead(in) == LOW) {
			//Write to the subindex of the read bit where the LED is
			digitalWrite(led[edgeCount/2 - readStart], !((NewTime - time) > 4));
			//Serial.print("Current edge:");
			//Serial.println(edgeCount / 2 - readStart);
			//Serial.print("State:");
			//Serial.println((NewTime - time) > 4);
			edgeCount++;
			time = NewTime;
			return;
		}
	}

	//We are in our write range
	if (edgeCount/2 >= writeStart && edgeCount/2 < writeStart + messageLength) {
		//Serial.print("Writing wire");
		//Serial.println(edgeCount);
		if (digitalRead(in) == HIGH) {
			//If the dip isn't high we really don't care for writing on the wire.
			Serial.print("Interested in state of digital");
			Serial.println(edgeCount / 2 - writeStart);
			if (digitalRead(dip[edgeCount/2 - writeStart]) == LOW) {
				unsigned long writeDuration = NewTime;
				//while (millis() - writeDuration < (bitTime / 4) * 3) {
				//	Serial.print("Line start value:");
				//	Serial.println(digitalRead(out));
				//	digitalWrite(out, digitalRead(dip[edgeCount / 2 - writeStart]));
				//	Serial.print("Line end value:");
				//	Serial.println(digitalRead(out));
				//}
				digitalWrite(out, HIGH);
				//This delay is very much not the tool we want but interrupts can fire during a delay so it's less awful than it looks.
				//We aren't very interested in general in interrupts that fire during our write moment.
				delay(6);
			}
			digitalWrite(out, LOW);
			edgeCount++;
			time = NewTime;
			return;
			}
	}

	edgeCount++;
	time = NewTime;
}

void loop() {
}