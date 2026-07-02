/*
  PRUEBA DE BUZZER DIRECTO - ESP32

  Esta prueba es opcional.
  Para la presentación usarás el buzzer controlado por relé.

  Conexiones:
  Buzzer + -> P25
  Buzzer - -> GND

  Si no suena, invierte los pines del buzzer.
*/

#define PIN_BUZZER 25

void setup() {
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
}

void loop() {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(500);

  digitalWrite(PIN_BUZZER, LOW);
  delay(1500);
}
