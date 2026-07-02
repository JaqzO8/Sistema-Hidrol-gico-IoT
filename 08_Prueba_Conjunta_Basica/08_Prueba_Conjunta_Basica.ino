/*
  PRUEBA CONJUNTA BASICA - ESP32

  Incluye:
  - OLED
  - DHT22
  - FC-37 DO/AO
  - Ultrasónico
  - LEDs
  - Relé controlando buzzer

  No incluye ThingSpeak todavía.
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define PIN_DHT 4
#define DHTTYPE DHT22

#define PIN_FC37_DO 34
#define PIN_FC37_AO 33

#define PIN_TRIG 17
#define PIN_ECHO 16

#define LED_VERDE 13
#define LED_AMARILLO 14
#define LED_ROJO 27

#define PIN_RELE 26

const bool LLUVIA_ACTIVA_EN_LOW = true;
const bool RELE_ACTIVO_EN_LOW = true;

const float DISTANCIA_SENSOR_FONDO_CM = 20.0;

const float NIVEL_MEDIO_CM = 8.0;
const float NIVEL_ALTO_CM = 13.0;
const float NIVEL_CRITICO_CM = 17.0;

#define ANCHO_OLED 128
#define ALTO_OLED 64

DHT dht(PIN_DHT, DHTTYPE);
Adafruit_SSD1306 display(ANCHO_OLED, ALTO_OLED, &Wire, -1);

void encenderRele() {
  digitalWrite(PIN_RELE, RELE_ACTIVO_EN_LOW ? LOW : HIGH);
}

void apagarRele() {
  digitalWrite(PIN_RELE, RELE_ACTIVO_EN_LOW ? HIGH : LOW);
}

bool leerLluviaDigital() {
  int lectura = digitalRead(PIN_FC37_DO);
  return LLUVIA_ACTIVA_EN_LOW ? (lectura == LOW) : (lectura == HIGH);
}

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
  return duracion * 0.0343 / 2.0;
}

void actualizarLEDs(String estado) {
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(LED_ROJO, LOW);

  if (estado == "NORMAL") {
    digitalWrite(LED_VERDE, HIGH);
  } else if (estado == "MEDIO") {
    digitalWrite(LED_AMARILLO, HIGH);
  } else if (estado == "ALTO" || estado == "CRITICO") {
    digitalWrite(LED_ROJO, HIGH);
  }
}

void controlarBuzzerPorRele(String estado) {
  unsigned long ahora = millis();

  if (estado == "NORMAL") {
    apagarRele();
  } else if (estado == "MEDIO") {
    if ((ahora % 2000) < 300) encenderRele();
    else apagarRele();
  } else if (estado == "ALTO") {
    if ((ahora % 1000) < 500) encenderRele();
    else apagarRele();
  } else if (estado == "CRITICO") {
    encenderRele();
  }
}

void mostrarOLED(float temp, float hum, bool lluvia, int lluviaAO, float nivel, String estado) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("Sistema Hidrologico");

  display.setCursor(0, 12);
  display.print("Temp: ");
  display.print(temp, 1);
  display.println(" C");

  display.setCursor(0, 22);
  display.print("Hum: ");
  display.print(hum, 0);
  display.println(" %");

  display.setCursor(0, 32);
  display.print("Lluvia: ");
  display.print(lluvia ? "SI" : "NO");
  display.print(" ");
  display.println(lluviaAO);

  display.setCursor(0, 42);
  display.print("Nivel: ");
  display.print(nivel, 1);
  display.println(" cm");

  display.setCursor(0, 54);
  display.print("Estado: ");
  display.println(estado);

  display.display();
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_FC37_DO, INPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);

  pinMode(PIN_RELE, OUTPUT);
  apagarRele();

  dht.begin();
  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("No se encontro OLED.");
  } else {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Sistema iniciado");
    display.display();
    delay(1200);
  }

  Serial.println("Prueba conjunta basica iniciada.");
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  bool lluvia = leerLluviaDigital();
  int lluviaAO = analogRead(PIN_FC37_AO);

  float distancia = medirDistanciaCM();
  float nivel = 0;

  if (distancia > 0) {
    nivel = DISTANCIA_SENSOR_FONDO_CM - distancia;
    if (nivel < 0) nivel = 0;
    if (nivel > DISTANCIA_SENSOR_FONDO_CM) nivel = DISTANCIA_SENSOR_FONDO_CM;
  }

  String estado = "NORMAL";
  if (lluvia || nivel >= NIVEL_MEDIO_CM) estado = "MEDIO";
  if (nivel >= NIVEL_ALTO_CM) estado = "ALTO";
  if (nivel >= NIVEL_CRITICO_CM) estado = "CRITICO";

  actualizarLEDs(estado);
  controlarBuzzerPorRele(estado);
  mostrarOLED(temp, hum, lluvia, lluviaAO, nivel, estado);

  Serial.println("========== SISTEMA ==========");
  Serial.print("Temp: "); Serial.println(temp);
  Serial.print("Hum: "); Serial.println(hum);
  Serial.print("Lluvia DO: "); Serial.println(lluvia ? "SI" : "NO");
  Serial.print("Lluvia AO: "); Serial.println(lluviaAO);
  Serial.print("Distancia: "); Serial.print(distancia); Serial.println(" cm");
  Serial.print("Nivel: "); Serial.print(nivel); Serial.println(" cm");
  Serial.print("Estado: "); Serial.println(estado);

  delay(800);
}
