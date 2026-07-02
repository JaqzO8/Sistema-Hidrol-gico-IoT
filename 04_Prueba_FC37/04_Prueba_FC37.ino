/*
  PRUEBA DE SENSOR DE LLUVIA FC-37 - ESP32

  Conexiones:
  FC-37 VCC -> 3V3
  FC-37 GND -> GND
  FC-37 DO  -> P34
  FC-37 AO  -> P33

  Nota:
  En muchos módulos FC-37, cuando detecta agua, DO pasa a LOW.
  Si la lectura sale al revés, cambia LLUVIA_ACTIVA_EN_LOW a false.
*/

#define PIN_FC37_DO 34
#define PIN_FC37_AO 33

const bool LLUVIA_ACTIVA_EN_LOW = true;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_FC37_DO, INPUT);
  Serial.println("Prueba FC-37 iniciada.");
}

void loop() {
  int lecturaDO = digitalRead(PIN_FC37_DO);
  int lecturaAO = analogRead(PIN_FC37_AO);

  bool lluviaDetectada;
  if (LLUVIA_ACTIVA_EN_LOW) {
    lluviaDetectada = lecturaDO == LOW;
  } else {
    lluviaDetectada = lecturaDO == HIGH;
  }

  Serial.println("========== FC-37 ==========");
  Serial.print("DO digital: ");
  Serial.println(lecturaDO);

  Serial.print("AO analogico: ");
  Serial.println(lecturaAO);

  Serial.print("Lluvia detectada: ");
  Serial.println(lluviaDetectada ? "SI" : "NO");

  delay(1000);
}
