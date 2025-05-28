#include <Arduino.h>

#define NUM_ENCODERS 6

const int sensorPins[NUM_ENCODERS] = {A0, A1, A2, A3, A4, A5};
const int baseNotes [NUM_ENCODERS] = {262, 294, 330, 349, 392, 440};
volatile int noteOffsets[NUM_ENCODERS] = {0};
const int buzzerPin = 9;

const int encoderCLK[NUM_ENCODERS] = {21, 2, 20, 3, 19, 18}; // top, middle, down, right-left
const int encoderDT[NUM_ENCODERS]  = {29, 34, 22, 35, 23, 28};

/*
INT0 - PIN 2
INT1 - PIN 3
INT2 - PIN 21
INT3 - PIN 20
INT4 - PIN 19
INT5 - PIN 18
*/

void handleEncoder(int idx);

// Prototip ISR-uri
ISR(INT0_vect){ handleEncoder(0); }
ISR(INT1_vect){ handleEncoder(1); }
ISR(INT2_vect){ handleEncoder(3); }
ISR(INT3_vect){ handleEncoder(2); }
ISR(INT4_vect){ handleEncoder(5); }
ISR(INT5_vect){ handleEncoder(4); }

void setup() {
  Serial.begin(9600);
  pinMode(buzzerPin, OUTPUT);

  for (int i = 0; i < NUM_ENCODERS; i++) {
    pinMode(encoderCLK[i], INPUT_PULLUP);
    pinMode(encoderDT[i], INPUT_PULLUP);
  }

  EICRA |= (1<<ISC01)|(1<<ISC00)|(1<<ISC11)|(1<<ISC10);
  EICRB |= (1<<ISC21)|(1<<ISC20)
         | (1<<ISC31)|(1<<ISC30)
         | (1<<ISC41)|(1<<ISC40)
         | (1<<ISC51)|(1<<ISC50);
  EIFR = 0x3F; // Clear all INT0–INT5 flags
  EIMSK |= (1<<INT0)|(1<<INT1)|(1<<INT2)
         | (1<<INT3)|(1<<INT4)|(1<<INT5);
}

void loop() {
}

volatile static unsigned long lastTime[NUM_ENCODERS] = {0};

void handleEncoder(int idx) {
  // La fiecare front crescător pe CLK verificăm DT pentru direcție
  if (digitalRead(encoderDT[idx]) == LOW) {
    noteOffsets[idx] += 10;  // rotație în sens orar
    Serial.print("Encoder "); Serial.print(idx); Serial.println(" rotated clockwise");
  } else {
    noteOffsets[idx] -= 10;  // rotație inversă
    Serial.print("Encoder "); Serial.print(idx); Serial.println(" rotated counterclockwise");
  }
  noteOffsets[idx] = constrain(noteOffsets[idx], -200, 200);
}
