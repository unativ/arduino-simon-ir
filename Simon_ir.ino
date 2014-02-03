/*
  A 3 LED Simon Says Game
*/

#include <IRremote.h>
#include "pitches.h"
#include <NewTone.h>

int RECV_PIN = 2;

IRrecv irrecv(RECV_PIN);
decode_results results;


const int NUM_LEDS = 3; // 3 leds
const int speakerPin = 6; // speaker connected at pin 10

//int buttonPin[] = {3, 2, 4}; // buttons connected at pins 3, 2, 4
int ledPin[] = {12, 8, 4};   // LEDS connected at pins 13, 8, 9
int waitForInputPin = 7;     // light up a led when waiting for input from the user
// set the tone for each LED
int tones[] = {NOTE_C4, NOTE_A4, NOTE_G3};  

//int buttonState[] = {HIGH, HIGH, HIGH};
int lastButtonState[] = {LOW, LOW, LOW};
boolean buttonOn[] = {false, false};

// win when completing level 7
int sequence[] = {0, 0, 0, 0, 0, 0, 0};
int max_level = 7;
int level = 1;

void setup() {
  randomSeed(analogRead(0));
  Serial.begin(9600);

  irrecv.enableIRIn(); // Start the receiver
  
  // initialize the LED pin as an output:
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(ledPin[i], OUTPUT);         
  }
  pinMode(waitForInputPin, OUTPUT);         
}

// generate a random seuqence
void generateSequence() {
  for (int i = 0; i < max_level; i++) {
    int randLed = random(0, 3);
    sequence[i] = randLed;
  }
}

void loop(){

  boolean wrong = false;
  level = 1;

  generateSequence();

  // play until win or loose
  while (!wrong && level <= max_level) {
    playSequence(level);
    wrong = inputFromUser(level);
    level++;
  }
  
  if (wrong) {
    playLoosingSequence();
  }
  else {
    playWinningSequence();
  }
  delay(1000);
 
}

// play the sequence till the level the user has reached 
void playSequence(int level) {
  for (volatile int i = 0; i < level; i++) {
    volatile int ledNum = sequence[i];
    NewTone(speakerPin, tones[ledNum]);
    digitalWrite(ledPin[ledNum], HIGH);
    delay(500 - 30 * level);
    digitalWrite(ledPin[ledNum], LOW);
    noNewTone(speakerPin);
    delay(200  - 10 * level); 
  }
}

// listen to the buttons pressed by the user
boolean inputFromUser(int level) {
  // turn on a led to indicate that we are waiting for input from the user
  digitalWrite(waitForInputPin, HIGH);
  boolean wrong = false;
  int seqIndex = 0;
  
  while (!wrong && seqIndex < level) {
    volatile int value = -1;
    while (value == -1) {
      if (irrecv.decode(&results)) {
        Serial.print("results.value=");
        Serial.println(results.value, HEX);
        value = dump(&results);
        Serial.print("value=");
        Serial.println(value);
        irrecv.resume(); // Receive the next value
      }
    }
    if (value == sequence[seqIndex]) {
      play(ledPin[value], tones[value]);
      seqIndex++;
    }
    else {
      wrong = true;
      Serial.println("WRONG");
    }
  }
  digitalWrite(waitForInputPin, LOW);
  
  return wrong;
}

int dump(volatile decode_results *results) {
    volatile int count = results->rawlen;

    if (results->decode_type == UNKNOWN) {
    Serial.println("Could not decode message");
  } 
  else {
    if (results->decode_type == NEC) {
      Serial.print("Decoded NEC: ");
    } 
    else if (results->decode_type == SONY) {
      Serial.print("Decoded SONY: ");
    } 
    else if (results->decode_type == RC5) {
      Serial.print("Decoded RC5: ");
    } 
    else if (results->decode_type == RC6) {
      Serial.print("Decoded RC6: ");
    }
    Serial.print(results->value, HEX);
    Serial.print(" (");
    Serial.print(results->bits, DEC);
    Serial.println(" bits)");
  }
  Serial.print("Raw (");
  Serial.print(count, DEC);
  Serial.print("): ");

  for (int i = 0; i < count; i++) {
    if ((i % 2) == 1) {
      Serial.print(results->rawbuf[i]*USECPERTICK, DEC);
    } 
    else {
      Serial.print(-(int)results->rawbuf[i]*USECPERTICK, DEC);
    }
    Serial.print(" ");
  }
  Serial.println("");

  

  // 0
  if (results->value == 0xFF30CF) {
    return 0;
  }

  // 1
  if (results->value == 0xFF7A85) {
    return 1;
  }

  // 2
  if (results->value == 0xFF18E7) {
    return 2;
  }

  return -1;
}

// play a tone and light a LED
void play(int ledNum, int toneValue) {
  NewTone(speakerPin, toneValue);
  digitalWrite(ledNum, HIGH);
  delay(500);
  digitalWrite(ledNum, LOW);
  noNewTone(speakerPin);
  delay(500);
}

void playWinningSequence() {
  int mc_melody[] = {           NOTE_C4, NOTE_C4, NOTE_C4, NOTE_G3, NOTE_A3, NOTE_A3, NOTE_G3, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_D4, NOTE_C4, NOTE_G3, NOTE_C4, NOTE_C4, NOTE_C4, NOTE_G3, NOTE_A3, NOTE_A3, NOTE_G3, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_D4, NOTE_C4};
  int mc_noteDurations[] = {    4,       4,       4,       4,       4,       4,       2,       4,       4,       4,       4,       2,       4,       4,       4,       4,       4,       4,       4,       2,       4,       4,       4,       4,       2  };

  for (int thisNote = 0; thisNote < 25; thisNote++) {

    // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000/mc_noteDurations[thisNote];
    NewTone(speakerPin, mc_melody[thisNote],noteDuration);

    int randLed = random(0, 3);
    digitalWrite(ledPin[randLed], HIGH);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    digitalWrite(ledPin[randLed], LOW);
    // stop the tone playing:
    noNewTone(speakerPin);
  }
}

void playLoosingSequence() {
  // C do
  // D re
  // E mi 
  // F fa
  // G sol
  // A la
  // B ci
  int mc_melody[] =        {NOTE_F2, NOTE_E2, NOTE_D2, NOTE_C2};
  int mc_noteDurations[] = {      4,       4,       4,       2};

  for (int thisNote = 0; thisNote < 4; thisNote++) {

    // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000/mc_noteDurations[thisNote];
    NewTone(speakerPin, mc_melody[thisNote],noteDuration);

    int randLed = random(0, 3);
    digitalWrite(ledPin[randLed], HIGH);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    digitalWrite(ledPin[randLed], LOW);
    // stop the tone playing:
    noNewTone(speakerPin);
  }
}



