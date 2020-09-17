#include <Arduino.h>
//----- Librerías de Display -----
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <Fonts/FreeSans9pt7b.h>

//------ Librería memoria EEPROM -----
#include <EEPROM.h>

//----- Variables de I2C -----
#define OLED_SDA 21
#define OLED_SCL 22

//----- Declaraciones de Display -----
Adafruit_SH1106 display(OLED_SDA, OLED_SCL);

//----- Inicializa Display -----
void setupDisplay() {
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.setTextColor(WHITE);
  display.display();
  delay(2000);
  display.clearDisplay();
}

void clearDisplay() {
  display.clearDisplay();
}

void mostrarReal(String v, String b, String i, String p, String c, String corr) {
  display.clearDisplay();
  //----- Títulos -----
  display.setFont();
  display.setCursor(0, 5);
  display.setTextSize(1);
  display.print("VOLUMEN: ");
  display.setCursor(0, 15);
  display.setTextSize(1);
  display.print("BPM: ");
  display.setCursor(0, 25);
  display.setTextSize(1);
  display.print("RELACION (I:E): ");
  display.setCursor(0, 35);
  display.setTextSize(1);
  display.print("PRESION MAXIMA");
  display.setCursor(0, 45);
  display.setTextSize(1);
  display.print("PRESION ACTUAL");
  display.setCursor(0, 55);
  display.setTextSize(1);
  display.print("CORRIENTE MOTOR");
  //----- Valores actualizables -----
  display.setCursor(95, 5);
  display.setTextSize(1);
  display.print(v);

  display.setCursor(95, 15);
  display.setTextSize(1);
  display.print(b);

  display.setCursor(95, 25);
  display.setTextSize(1);
  display.print("1:" + i);

  display.setCursor(95, 35);
  display.setTextSize(1);
  display.print(p);

  display.setCursor(95, 45);
  display.setTextSize(1);
  display.print(c);

  display.setCursor(95, 55);
  display.setTextSize(1);
  display.print(corr + "A");

  display.display();
}


void mostrarParametroMenu(String param, String value) {
  Serial.print(param); Serial.print(" / "); Serial.println(value);
  display.clearDisplay();
  display.setFont(&FreeSans9pt7b);
  display.setCursor(10, 15);
  display.print(param);

  display.setCursor(10, 45);
  display.print(value);

  display.display();
}

void mostrarConfirmacion(){
  display.clearDisplay();
  display.setFont(&FreeSans9pt7b);
  display.setCursor(25, 30);
  display.print("Guardado");
  display.display();
  delay(2000);
}

//----- Funciones de lectura y escritura EEPROM -----
void setupEEPROMM(int x) {
  EEPROM.begin(x);
}

String leerEEPROM10(int addr) {
  byte lectura;
  String strLectura;
  for (int i = addr; i < addr + 10; i++) {
    lectura = EEPROM.read(i);
    if (lectura != 255) {
      strLectura += (char)lectura;
    }
  }
  //Serial.print("leído :");
  //Serial.println(strLectura);
  return strLectura;
}
void grabarEEPROM10(int addr, String a) {
  Serial.print("Guardando 10 bytes :");
  Serial.println(a);
  int tamano = a.length();
  char inchar[10];
  a.toCharArray(inchar, tamano + 1);
  if (inchar[0] != 0) {
    for (int i = 0; i < tamano ; i++) {
      EEPROM.write(addr + i, inchar[i]);
    }
    for (int i = tamano; i < 10; i++) {
      EEPROM.write(addr + i, 255); // Rellena los bytes restantes menores de 10 con 11111111
    }
    EEPROM.commit();
  }
}
