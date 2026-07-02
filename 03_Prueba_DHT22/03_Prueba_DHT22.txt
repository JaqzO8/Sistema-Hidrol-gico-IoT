/*
  PRUEBA DE DHT22 - ESP32

  Librerías necesarias:
  - DHT sensor library
  - Adafruit Unified Sensor

  Conexiones:
  DHT22 UCC  -> 3V3
  DHT22 DATA -> P4
  DHT22 GND  -> GND
*/

#include <DHT.h>

#define PIN_DHT 4
#define DHTTYPE DHT22

DHT dht(PIN_DHT, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  Serial.println("Prueba DHT22 iniciada.");
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  Serial.println("========== DHT22 ==========");

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Error leyendo DHT22.");
    Serial.println("Revisa UCC -> 3V3, DATA -> P4, GND -> GND.");
  } else {
    Serial.print("Temperatura: ");
    Serial.print(temp);
    Serial.println(" C");

    Serial.print("Humedad: ");
    Serial.print(hum);
    Serial.println(" %");
  }

  delay(2000);
}
