#include <Arduino.h>

#include "adc.h"

#define NUM_CHORDS 6

const int sensorPins[NUM_CHORDS] = {0, 1, 2, 3, 4, 5};
const int notes[NUM_CHORDS] = {262, 294, 330, 349, 392, 440}; // C4–A4
const int threshold = 40;
const int buzzerPin = 11;

const int encoderCLK[NUM_CHORDS] = {21, 2, 20, 3, 19, 18}; // top, middle, down, right-left
const int encoderDT[NUM_CHORDS]  = {29, 34, 22, 35, 23, 28};

int noteOffsets[NUM_CHORDS] = {0};

volatile int lastState[NUM_CHORDS];

volatile uint16_t toggle_count = 0;
volatile uint16_t toggle_limit = 0;
volatile float fade_factor = 1.0;

void tone_init(uint16_t frequency, float dutyCycle = 0.5) {
  pinMode(11, OUTPUT);

  TCCR1A = 0;
  TCCR1B = 0;

  uint32_t top = (F_CPU / (2UL * frequency)) - 1;
  if (top > 65535) top = 65535;

  OCR1A = top;

  TCCR1A |= (1 << COM1A0);              
  TCCR1B |= (1 << WGM12) | (1 << CS10);
}

void tone_stop() {
  TCCR1A = 0;
  TCCR1B = 0;
  digitalWrite(11, LOW);
}

void playGuitarChord(const uint16_t notes[], uint8_t count) {
  for (uint8_t i = 0; i < count; i++) {
    tone_init(notes[i]);

    // Simulate pluck fading out
    for (uint8_t j = 0; j < 12; j++) {
      float fadeDuty = 0.5 + (j * 0.03);
      if (fadeDuty < 0.05) fadeDuty = 0.05;
      OCR2B = OCR2A * fadeDuty;
      delay(5);
    }

    tone_stop();
    delay(100);
  }
}


int digital_read_manual(uint8_t pin) {
  switch (pin) {
    // Buzzer
    case 11:  return (PINB & (1 << PB5)) ? HIGH : LOW;

    // Encoder CLK
    case 21: return (PIND & (1 << 0)) ? HIGH : LOW;
    case 2:  return (PINE & (1 << 4)) ? HIGH : LOW;
    case 20: return (PIND & (1 << 1)) ? HIGH : LOW;
    case 3:  return (PINE & (1 << 5)) ? HIGH : LOW;
    case 19: return (PIND & (1 << 2)) ? HIGH : LOW;
    case 18: return (PIND & (1 << 3)) ? HIGH : LOW;

    // Encoder DT
    case 29: return (PINA & (1 << 7)) ? HIGH : LOW;
    case 34: return (PINC & (1 << 3)) ? HIGH : LOW;
    case 22: return (PINA & (1 << 0)) ? HIGH : LOW;
    case 35: return (PINC & (1 << 2)) ? HIGH : LOW;
    case 23: return (PINA & (1 << 1)) ? HIGH : LOW;
    case 28: return (PINA & (1 << 6)) ? HIGH : LOW;

    default:
      return LOW;
  }
}

void GPIO_init() {
  // Buzzer (OUTPUT)
  DDRB |= (1 << PB5); // pin 11

  // Encoder CLK (INPUT)
  DDRD &= ~(1 << 0); // pin 21
  DDRE &= ~(1 << 4); // pin 2
  DDRD &= ~(1 << 1); // pin 20
  DDRE &= ~(1 << 5); // pin 3
  DDRD &= ~(1 << 2); // pin 19
  DDRD &= ~(1 << 3); // pin 18

  // Encoder DT (INPUT)
  DDRA &= ~(1 << 7); // pin 29
  DDRC &= ~(1 << 3); // pin 34
  DDRA &= ~(1 << 0); // pin 22
  DDRC &= ~(1 << 2); // pin 35
  DDRA &= ~(1 << 1); // pin 23
  DDRA &= ~(1 << 6); // pin 28
}

void setup() {
  Serial.begin(9600);
  GPIO_init();
  adc_init();
}

void loop() {
  bool notePlayed = false;

  for (int i = 0; i < NUM_CHORDS; i++) {
    int valoare = analog_read(sensorPins[i]);

    if (valoare < threshold && i == 0) {
      tone_init(notes[i] + noteOffsets[i]);
      notePlayed = true;
      break;
    }
  }

  if (!notePlayed)
    tone_stop();

  for (int i = 0; i < NUM_CHORDS; i++) {
    int currentStateCLK = digital_read_manual(encoderCLK[i]);

    if (currentStateCLK != lastState[i]) {
      if (digital_read_manual(encoderDT[i]) != currentStateCLK) {
        Serial.print("Encoder "); Serial.print(i); Serial.println(" rotated clockwise");
        noteOffsets[i] += 10;
      } else {
        Serial.print("Encoder "); Serial.print(i); Serial.println(" rotated counterclockwise");
        noteOffsets[i] -= 10;
      }
    }

    lastState[i] = currentStateCLK;
  }

  delay(10);
}