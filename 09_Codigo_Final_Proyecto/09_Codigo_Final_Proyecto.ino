/*
  CODIGO FINAL - SISTEMA IoT HIDROLOGICO ESP32

  Incluye:
  - DHT11
  - FC-37 DO y AO
  - Ultrasónico
  - OLED
  - LEDs
  - Relé controlando buzzer
  - Predicción por tendencia
  - ThingSpeak opcional

  Librerías necesarias:
  - WiFi
  - ThingSpeak
  - Adafruit SSD1306
  - Adafruit GFX Library
  - DHT sensor library
  - Adafruit Unified Sensor
*/

#include <WiFi.h>
#include "ThingSpeak.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define USAR_THINGSPEAK 1

const char* WIFI_SSID = "queZJA";
const char* WIFI_PASS = "josexd67";

unsigned long TS_CHANNEL_ID = 3420787;
const char* TS_WRITE_API_KEY = "PNG3H1HI4YMSG9Z0";

WiFiClient client;

#define PIN_DHT 16
#define DHTTYPE DHT11

#define PIN_FC37_AO 36

#define PIN_TRIG 18
#define PIN_ECHO 19

#define LED_VERDE 25
#define LED_AMARILLO 26
#define LED_ROJO 27

#define PIN_BUZZER 23

#define ANCHO_OLED 128
#define ALTO_OLED 64

Adafruit_SSD1306 display(ANCHO_OLED, ALTO_OLED, &Wire, -1);
DHT dht(PIN_DHT, DHTTYPE);

const bool LLUVIA_ACTIVA_EN_LOW = true;
const bool BUZZER_ACTIVO_EN_LOW = true;

const float DISTANCIA_SENSOR_FONDO_CM = 20.0;
const float NIVEL_MEDIO_CM = 4.0;
const float NIVEL_ALTO_CM = 8.0;
const float NIVEL_CRITICO_CM = 10.0;
const float VELOCIDAD_ALTA_CM_MIN = 2.0;
const float MINUTOS_PREDICCION = 3.0;

const unsigned long INTERVALO_LECTURA_MS = 2000;
const unsigned long INTERVALO_THINGSPEAK_MS = 20000;

unsigned long ultimoTiempoLectura = 0;
unsigned long ultimoEnvioThingSpeak = 0;

float nivelAnterior = 0.0;
unsigned long tiempoNivelAnterior = 0;
bool primeraLectura = true;

float temperatura = 0;
float humedad = 0;
int lluviaAnalogica = 0;
float distanciaCM = 0;
float nivelCM = 0;
float velocidadCMMin = 0;
float nivelPredichoCM = 0;

enum EstadoRiesgo {
  NORMAL = 0,
  MEDIO = 1,
  ALTO = 2,
  CRITICO = 3
};

EstadoRiesgo estadoActual = NORMAL;

void encenderBuzzer() {
  digitalWrite(PIN_BUZZER, HIGH);
}

void apagarBuzzer() {
  digitalWrite(PIN_BUZZER, LOW);
}



float medirDistanciaSimpleCM() {
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

float medirDistanciaPromedioCM() {
  const int muestras = 5;
  float suma = 0;
  int validas = 0;

  for (int i = 0; i < muestras; i++) {
    float d = medirDistanciaSimpleCM();

    if (d > 0 && d < 400) {
      suma += d;
      validas++;
    }

    delay(40);
  }

  if (validas == 0) {
    return -1;
  }

  return suma / validas;
}

float calcularNivel(float distancia) {
  if (distancia < 0) {
    return nivelCM;
  }

  float nivel = DISTANCIA_SENSOR_FONDO_CM - distancia;

  if (nivel < 0) nivel = 0;
  if (nivel > DISTANCIA_SENSOR_FONDO_CM) nivel = DISTANCIA_SENSOR_FONDO_CM;

  return nivel;
}

float calcularVelocidad(float nivelActual) {
  unsigned long ahora = millis();

  if (primeraLectura) {
    primeraLectura = false;
    nivelAnterior = nivelActual;
    tiempoNivelAnterior = ahora;
    return 0;
  }

  float minutos = (ahora - tiempoNivelAnterior) / 60000.0;

  if (minutos <= 0) {
    return 0;
  }

  float velocidad = (nivelActual - nivelAnterior) / minutos;

  nivelAnterior = nivelActual;
  tiempoNivelAnterior = ahora;

  if (velocidad < 0) {
    velocidad = 0;
  }

  return velocidad;
}

EstadoRiesgo evaluarEstado() {
  if (nivelCM >= NIVEL_CRITICO_CM || nivelPredichoCM >= NIVEL_CRITICO_CM) {
    return CRITICO;
  }

  if (nivelCM >= NIVEL_ALTO_CM || velocidadCMMin >= VELOCIDAD_ALTA_CM_MIN) {
    return ALTO;
  }

  if (nivelCM >= NIVEL_MEDIO_CM ) {
    return MEDIO;
  }

  return NORMAL;
}

String textoEstado(EstadoRiesgo estado) {
  switch (estado) {
    case NORMAL: return "NORMAL";
    case MEDIO: return "MEDIO";
    case ALTO: return "ALTO";
    case CRITICO: return "CRITICO";
    default: return "ERROR";
  }
}

void actualizarLEDs() {
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(LED_ROJO, LOW);

  if (estadoActual == NORMAL) {
    digitalWrite(LED_VERDE, HIGH);
  } else if (estadoActual == MEDIO) {
    digitalWrite(LED_AMARILLO, HIGH);
  } else if (estadoActual == ALTO || estadoActual == CRITICO) {
    digitalWrite(LED_ROJO, HIGH);
  }
}

void actualizarBuzzer() {
  unsigned long ahora = millis();

  if (estadoActual == NORMAL || estadoActual == MEDIO ) {
    apagarBuzzer();
  } else if (estadoActual == ALTO) {
    if ((ahora % 1000) < 600) encenderBuzzer();
    else apagarBuzzer();
  } else if (estadoActual == CRITICO) {
    encenderBuzzer();
  }
}

void mostrarOLED() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("Sistema Hidrologico");

  display.setCursor(0, 10);
  display.print("Nivel: ");
  display.print(nivelCM, 1);
  display.println(" cm");

  display.setCursor(0, 20);
  display.print("Pred: ");
  display.print(nivelPredichoCM, 1);
  display.println(" cm");

  display.setCursor(0, 30);
  display.print("Vel: ");
  display.print(velocidadCMMin, 1);
  display.println(" cm/min");

  display.setCursor(0, 40);
  display.print("Lluvia: ");
  display.println(lluviaAnalogica < 3000 ? "SI" : "NO");

  display.setCursor(0, 52);
  display.print("Estado: ");
  display.println(textoEstado(estadoActual));

  display.display();
}

void conectarWiFi() {
#if USAR_THINGSPEAK
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long inicio = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 10000) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi conectado.");
  } else {
    Serial.println("No se pudo conectar WiFi.");
  }
#endif
}

void enviarThingSpeak() {
#if USAR_THINGSPEAK
  conectarWiFi();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Sin WiFi, no se envia a ThingSpeak.");
    return;
  }

  ThingSpeak.setField(1, nivelCM);
  ThingSpeak.setField(2, lluviaAnalogica < 3000 ? 1 : 0);
  ThingSpeak.setField(3, temperatura);
  ThingSpeak.setField(4, humedad);
  ThingSpeak.setField(5, velocidadCMMin);
  ThingSpeak.setField(6, nivelPredichoCM);
  ThingSpeak.setField(7, (int)estadoActual);
  ThingSpeak.setField(8, lluviaAnalogica);

  int respuesta = ThingSpeak.writeFields(TS_CHANNEL_ID, TS_WRITE_API_KEY);

  if (respuesta == 200) {
    Serial.println("ThingSpeak OK.");
  } else {
    Serial.print("Error ThingSpeak: ");
    Serial.println(respuesta);
  }
#endif
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_FC37_AO, INPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);

  pinMode(PIN_BUZZER, OUTPUT);
  apagarBuzzer();

  dht.begin();
  Wire.begin(21, 22);
  delay(10000);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("No se encontro OLED.");
  } else {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Sistema IoT");
    display.println("Iniciando...");
    display.display();
    delay(1500);
  }

#if USAR_THINGSPEAK
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  conectarWiFi();
#endif

  Serial.println("Sistema general iniciado.");
}

void loop() {
  unsigned long ahora = millis();

  actualizarLEDs();
  actualizarBuzzer();

  if (ahora - ultimoTiempoLectura >= INTERVALO_LECTURA_MS) {
    ultimoTiempoLectura = ahora;

    temperatura = dht.readTemperature();
    humedad = dht.readHumidity();

    lluviaAnalogica = analogRead(PIN_FC37_AO);

    distanciaCM = medirDistanciaPromedioCM();
    nivelCM = calcularNivel(distanciaCM);

    velocidadCMMin = calcularVelocidad(nivelCM);
    nivelPredichoCM = nivelCM + velocidadCMMin * MINUTOS_PREDICCION;

    estadoActual = evaluarEstado();
    mostrarOLED();

    Serial.println("========== DATOS ==========");
    Serial.print("Temp: "); Serial.println(temperatura);
    Serial.print("Hum: "); Serial.println(humedad);
    Serial.print("Lluvia analogica: "); Serial.println(lluviaAnalogica);
    Serial.print("Distancia: "); Serial.print(distanciaCM); Serial.println(" cm");
    Serial.print("Nivel: "); Serial.print(nivelCM); Serial.println(" cm");
    Serial.print("Velocidad: "); Serial.print(velocidadCMMin); Serial.println(" cm/min");
    Serial.print("Prediccion: "); Serial.print(nivelPredichoCM); Serial.println(" cm");
    Serial.print("Estado: "); Serial.println(textoEstado(estadoActual));
  }

  if (USAR_THINGSPEAK && ahora - ultimoEnvioThingSpeak >= INTERVALO_THINGSPEAK_MS) {
    ultimoEnvioThingSpeak = ahora;
    enviarThingSpeak();
  }
}