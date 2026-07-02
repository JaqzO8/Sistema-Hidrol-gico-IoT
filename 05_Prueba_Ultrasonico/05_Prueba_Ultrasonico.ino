/*
  PRUEBA DE SENSOR ULTRASONICO - ESP32

  Conexiones:
  Ultrasónico Ucc             -> 5V
  Ultrasónico Gnd             -> GND
  Ultrasónico Trig_RX_SCL_I/O -> P17
  Ultrasónico Echo_TX_SDA     -> P16 con divisor de voltaje

  IMPORTANTE:
  No conectes Echo_TX_SDA directo a P16 si tu sensor entrega 5V.
*/

#define PIN_TRIG 17
#define PIN_ECHO 16

float medirDistanciaCM() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(3);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long duracion = pulseIn(PIN_ECHO, HIGH, 30000);
  if (duracion == 0) {
    return -1;
  }

  float distancia = duracion * 0.0343 / 2.0;
  return distancia;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  Serial.println("Prueba de ultrasonico iniciada.");
}

void loop() {
  float distancia = medirDistanciaCM();

  Serial.println("========== ULTRASONICO ==========");
  if (distancia < 0) {
    Serial.println("Sin lectura. Revisa TRIG, ECHO, GND y divisor de voltaje.");
  } else {
    Serial.print("Distancia: ");
    Serial.print(distancia);
    Serial.println(" cm");
  }

  delay(1000);
}
