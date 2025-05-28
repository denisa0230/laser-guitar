#include <Arduino.h>

#define NUM_CHORDS 6

const int sensorPins[NUM_CHORDS] = {0, 1, 2, 3, 4, 5};
const int notes[NUM_CHORDS] = {262, 294, 330, 349, 392, 440}; // C4–A4
const int threshold = 30;
const int buzzerPin = 11;
const int encoderCLK[NUM_CHORDS] = {21, 2, 20, 3, 19, 18}; // top, middle, down, right-left
const int encoderDT[NUM_CHORDS]  = {29, 34, 22, 35, 23, 28};

int noteOffsets[NUM_CHORDS] = {0};
volatile int lastState[NUM_CHORDS];

void adc_init() {
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);
}

uint16_t analog_read(uint8_t channel) {
  ADMUX  = (1 << REFS0) | (channel & 0x0f);
  ADCSRB &= ~(1 << MUX5);          

  ADCSRA |= (1 << ADSC);                   
  while (ADCSRA & (1 << ADSC));           

  return ADC;
}

int digital_read_manual(uint8_t pin) {
  switch (pin) {
    // Buzzer
    case 11:  return (PINB & (1 << PB5)) ? HIGH : LOW;

    // Encoder CLK
    case 21: return (PIND & (1 << PD0)) ? HIGH : LOW;
    case 2:  return (PINE & (1 << PE4)) ? HIGH : LOW;
    case 20: return (PIND & (1 << PD1)) ? HIGH : LOW;
    case 3:  return (PINE & (1 << PE5)) ? HIGH : LOW;
    case 19: return (PIND & (1 << PD2)) ? HIGH : LOW;
    case 18: return (PIND & (1 << PD3)) ? HIGH : LOW;

    // Encoder DT
    case 29: return (PINA & (1 << PA7)) ? HIGH : LOW;
    case 34: return (PINC & (1 << PC3)) ? HIGH : LOW;
    case 22: return (PINA & (1 << PA0)) ? HIGH : LOW;
    case 35: return (PINC & (1 << PC2)) ? HIGH : LOW;
    case 23: return (PINA & (1 << PA1)) ? HIGH : LOW;
    case 28: return (PINA & (1 << PA6)) ? HIGH : LOW;

    default:
      return LOW;
  }
}

void GPIO_init() {
  // Buzzer (OUTPUT)
  DDRB |= (1 << PB5);

  // Encoder CLK (INPUT)
  DDRD &= ~(1 << PD0); // pin 21
  DDRE &= ~(1 << PE4); // pin 2
  DDRD &= ~(1 << PD1); // pin 20
  DDRE &= ~(1 << PE5); // pin 3
  DDRD &= ~(1 << PD2); // pin 19
  DDRD &= ~(1 << PD3); // pin 18

  // Encoder DT (INPUT)
  DDRA &= ~(1 << PA7); // pin 29
  DDRC &= ~(1 << PC3); // pin 34
  DDRA &= ~(1 << PA0); // pin 22
  DDRC &= ~(1 << PC2); // pin 35
  DDRA &= ~(1 << PA1); // pin 23
  DDRA &= ~(1 << PA6); // pin 28
}

void setup() {
  Serial.begin(9600);
  GPIO_init();
  adc_init();
}

void loop() {
  bool notePlayed = false;

  for (int i = 0; i < NUM_CHORDS; i++) {
    int val = analog_read(sensorPins[i]);

    if (val < threshold) {
      Serial.print("Sensor "); Serial.print(i); Serial.print(" has value: "); Serial.println(val);
      tone(buzzerPin, notes[i] + noteOffsets[i] - 20);
      notePlayed = true;
      break;
    }
  }

  if (!notePlayed) {
    noTone(buzzerPin);
  }

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