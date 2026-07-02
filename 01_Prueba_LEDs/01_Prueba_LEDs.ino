/*
  PRUEBA DE LEDS - ESP32

  Conexiones:
  LED verde:
    P13 -> resistencia 220 ohm -> patita larga LED verde
    patita corta LED verde -> GND

  LED amarillo:
    P14 -> resistencia 220 ohm -> patita larga LED amarillo
    patita corta LED amarillo -> GND

  LED rojo:
    P27 -> resistencia 220 ohm -> patita larga LED rojo
    patita corta LED rojo -> GND
*/

#define LED_VERDE 13
#define LED_AMARILLO 14
#define LED_ROJO 27

void setup() {
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
}

void loop() {
  digitalWrite(LED_VERDE, HIGH);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(LED_ROJO, LOW);
  delay(1000);

  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARILLO, HIGH);
  digitalWrite(LED_ROJO, LOW);
  delay(1000);

  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(LED_ROJO, HIGH);
  delay(1000);
}
