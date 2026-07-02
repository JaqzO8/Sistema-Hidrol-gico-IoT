/*
  PRUEBA DE OLED 0.96" I2C - ESP32

  Librerías necesarias:
  - Adafruit SSD1306
  - Adafruit GFX Library

  Conexiones:
  OLED VCC -> 3V3
  OLED GND -> GND
  OLED SDA -> P21
  OLED SCL -> P22
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ANCHO_OLED 128
#define ALTO_OLED 64

Adafruit_SSD1306 display(ANCHO_OLED, ALTO_OLED, &Wire, -1);

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("No se encontro OLED en direccion 0x3C.");
    Serial.println("Prueba cambiar 0x3C por 0x3D.");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("OLED funcionando");
  display.println("ESP32 + IoT");
  display.println("SDA: P21");
  display.println("SCL: P22");
  display.display();

  Serial.println("OLED funcionando correctamente.");
}

void loop() {
}
