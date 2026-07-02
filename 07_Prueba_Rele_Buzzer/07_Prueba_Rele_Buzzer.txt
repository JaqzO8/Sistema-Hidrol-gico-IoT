/*
  PRUEBA DE RELE CONTROLANDO BUZZER - ESP32

  Lado de control del relé:
  Relé VCC -> 5V
  Relé GND -> GND
  Relé IN  -> P26

  Lado de potencia del relé:
  5V -> COM del relé
  NO del relé -> + del buzzer
  - del buzzer -> GND

  No uses NC para esta prueba.

  Nota:
  Muchos módulos relé se activan con LOW.
  Si funciona al revés, cambia RELE_ACTIVO_EN_LOW a false.
*/

#define PIN_RELE 26

const bool RELE_ACTIVO_EN_LOW = true;

void encenderRele() {
  digitalWrite(PIN_RELE, RELE_ACTIVO_EN_LOW ? LOW : HIGH);
}

void apagarRele() {
  digitalWrite(PIN_RELE, RELE_ACTIVO_EN_LOW ? HIGH : LOW);
}

void setup() {
  pinMode(PIN_RELE, OUTPUT);
  apagarRele();

  Serial.begin(115200);
  Serial.println("Prueba de rele + buzzer iniciada.");
}

void loop() {
  Serial.println("Rele encendido: buzzer debe sonar.");
  encenderRele();
  delay(1000);

  Serial.println("Rele apagado: buzzer debe apagarse.");
  apagarRele();
  delay(2000);
}
