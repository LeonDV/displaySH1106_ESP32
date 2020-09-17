/*
   Interface para selección de parámetros de respirador.
   Cuenta con 5 botones:
      - Dos botones para subir y bajar los valores de los parámetros
      - Dos botones para subir y bajar los parámetros
      - Un botón para escoger
   Usa una pantalla OLED para visualizar los valores
   Parámetros:
     - Definición de relación I:E  (1:1) (1:2) (1:3) (1:4)
     - Pulsaciones por minuto de 8 a 40 en pasos de a 2 y en forma cíclica
     - Volumen de aire (250 - 800 ml) en pasos de 50ml
     - Umbral de presión máxima 15 - 40cmH20 en pasos de 5cmH20

   Distribución de EEPROM
   0 - 9      definición de configuración inicial 1=Configurado 0=nuevo, por configurar
   10 - 19    número espiración I:E
   20 - 29    respiraciones por minuto BPM
   30 - 39    volumen de inspiración
   40 - 49    presión máxima
   50 - 59    Posición seleccionada de vector relacion I:E
   60 - 69    Posición seleccionada de vector bpm
   70 - 79    Posición seleccionada de vector volumen
   80 - 89    Posición seleccionada de vector presMaxima

*/
//----- Archivos locales -----
#include "respirador.h"

//----- Pines de entrada -----
#define pinUp 25        //--- Recorre las posiciones de los arrays en forma ascendente
#define pinDown 33      //--- Recorre las posiciones de los arrays en forma descendente
#define paramsUp 32     //--- Interrupción, ingresa al menú de parámetros que estará disponible por 5 segundos
#define paramsDown 39   //--- Interrupción, ingresa al menú de parámetros que estará disponible por 5 segundos
#define confirmacion 34 //--- Graba configuración en EEPROM y sale de la función de menú

//----- Temporizadores -----
unsigned long tIniMenu;
#define tMaxMenu 10000
const int tiempoRebote = 250;
int long tiempoBoton = 0;

int volPrueba = 0;

//---- Variables a mostrar para selección ----
byte selector;
byte posicionValor;

//----- Definición de variables de memoria -----
#define EEconfig        0
#define EErelacion      10
#define EEbpm           20
#define EEvolumen       30
#define EEpresMax       40
#define EEposRelacion   50
#define EEposBpm        60
#define EEposVolumen    70
#define EEposPresMax    80

//----- Estructuras de datos -----
struct sVolumen {
  String nom = "Volumen";
  int valor[13] = {250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800};
  int posActual;
  byte limite = 12;
};
struct sBpm {
  String nom = "BPM";
  int valor[18] = {8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40};
  int  posActual;
  byte limite = 17;
};
struct sRelacion {
  String nom = "Relacion I:E";
  int valor[5] = {1, 2, 3, 4};
  int posActual;
  byte limite = 4;
};
struct sPresMax {
  String nom = "Presion Max";
  int valor[7] = {15, 20, 25, 30, 35, 40};
  int posActual;
  byte limite = 6;
};
struct sVolumen volumen;
struct sBpm bpm;
struct sRelacion relacion;
struct sPresMax presMax;

void setup() {
  //----- Inicializa Serial ----
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Interfaz Usuario Respirador");
  Serial.println("Versión 0.0.1");
  //----- Definición de entradas -----
  pinMode(pinUp, INPUT);
  pinMode(pinDown, INPUT);
  pinMode(paramsUp, INPUT);
  pinMode(paramsDown, INPUT);
  pinMode(confirmacion, INPUT);
  //----- Inicializa Display SH1106 I2C 128 X 64 pixeles -----
  setupDisplay();
  //----- Inicializa EEMPROM -----
  setupEEPROMM(128);
  //----- Configuración inicial -----
  if (leerEEPROM10(EEconfig).toInt() != 1) {
    //--- Se inicializa con los valores mínimos de cada parámetro---
    grabarEEPROM10(EEvolumen, String(volumen.valor[0]));
    grabarEEPROM10(EEposVolumen, "0");
    grabarEEPROM10(EEbpm, String(bpm.valor[0]));
    grabarEEPROM10(EEposBpm, "0");
    grabarEEPROM10(EErelacion, String(relacion.valor[0]));
    grabarEEPROM10(EEposRelacion, "0");
    grabarEEPROM10(EEpresMax, String(presMax.valor[0]));
    grabarEEPROM10(EEposPresMax, "0");
    grabarEEPROM10(EEconfig, "1");                   //--- Declara el equipo configurado
    Serial.println("Equipo configurado correctamente");
  }
  volumen.posActual = leerEEPROM10(EEposVolumen).toInt();
  bpm.posActual = leerEEPROM10(EEposBpm).toInt();
  relacion.posActual = leerEEPROM10(EEposPresMax).toInt();
  presMax.posActual = leerEEPROM10(EEposRelacion).toInt();
}

void loop() {
  //----- Lectura de pulsadores de selección que activan el menú de configuración -----
  byte pUp = digitalRead(pinUp);
  byte pDown = digitalRead(pinDown);
  byte pConfirmacion = digitalRead(confirmacion);
  Serial.print("pinUp(25): "); Serial.print(pUp); Serial.print(" - pinDown(33): "); Serial.print(pDown); Serial.print(" - confirmacion(36)"); Serial.println(pConfirmacion);
  byte up = digitalRead(paramsUp);
  byte down = digitalRead(paramsDown);
  Serial.print("paramsUp(32): "); Serial.print(up); Serial.print(" - paramsDown(39): "); Serial.println(down);
  if (up || down) {
    Serial.println("oprimido parametros");
    tIniMenu = millis();
    selector = 0;
    menuConfig();
  }
  volPrueba++;
  String x = String(volPrueba);
  mostrarReal(leerEEPROM10(30), leerEEPROM10(20), leerEEPROM10(10), leerEEPROM10(40), x, x);
  delay(1000);

}

void menuConfig() {
  const int tRebote = 150;
  int long tBoton = 0;
  selector = 0;
  bool presionado = false;
  byte pUp, pDown, pConfirmacion, up, down;
  String xRelacion;

  while (millis() - tIniMenu < tMaxMenu) {
    //----- Lectura de Botones -----
    pUp = digitalRead(pinUp);
    pDown = digitalRead(pinDown);
    pConfirmacion = digitalRead(confirmacion);
    up = digitalRead(paramsUp);
    down = digitalRead(paramsDown);

    //----- Lectura de botones de cambio de parámetro
    Serial.print(millis() - tiempoBoton); Serial.print("-");
    if (millis() - tiempoBoton > tiempoRebote) { //----- Antirebote selector de parámetro
      if (up) {
        selector = (selector + 1) % 4;
        presionado = true;
      } else if (down) {
        presionado = true;
        if (selector >= 1) {
          selector--;
        }
        else {
          selector = 3;
        }
      }
      if (presionado) {
        tiempoBoton = millis(); //----- Reinicia el contador del rebote
        tIniMenu = millis();    //----- Reinicia el contador para salir del menú de configuración
      }
      presionado = false;
    }

    //----- De acuerdo al parámetro seleccionado ingresa a cada case
    switch (selector) {
      case 0:  //VOLUMEN
        if (millis() - tBoton > tRebote) {
          if (pUp) {
            volumen.posActual = (volumen.posActual + 1) % volumen.limite;
            presionado = true;
          } else if (pDown) {
            presionado = true;
            if (volumen.posActual >= 1) {
              volumen.posActual--;
            }
            else {
              volumen.posActual = volumen.limite - 1;
            }
          }
          tBoton = millis();
        }
        if (pConfirmacion) {
          presionado = true;
          grabarEEPROM10(EEvolumen, String(volumen.valor[volumen.posActual]));
          grabarEEPROM10(EEposVolumen, String(volumen.posActual));
          Serial.print("Configuración Guardada");
          mostrarConfirmacion();
        }
        if (presionado) {
          tiempoBoton = millis(); //----- Reinicia el contador del rebote
          tIniMenu = millis();   //----- Reinicia el contador para salir del menú de configuración
          Serial.println("presionado valor volumen");
        }
        mostrarParametroMenu(volumen.nom, String(volumen.valor[volumen.posActual]));
        presionado = false;
        break;

      case 1://BPM
        if (millis() - tBoton > tRebote) {
          if (pUp) {
            bpm.posActual = (bpm.posActual + 1) % bpm.limite;
            presionado = true;
          } else if (pDown) {
            presionado = true;
            if (bpm.posActual >= 1) {
              bpm.posActual--;
            }
            else {
              bpm.posActual = bpm.limite - 1;
            }
          }
          tBoton = millis();
        }
        if (pConfirmacion) {
          presionado = true;
          grabarEEPROM10(EEbpm, String(bpm.valor[bpm.posActual]));
          grabarEEPROM10(EEposBpm, String(bpm.posActual));
          Serial.print("Configuración Guardada");
          mostrarConfirmacion();
        }
        if (presionado) {
          tiempoBoton = millis(); //----- Reinicia el contador del rebote
          tIniMenu = millis();   //----- Reinicia el contador para salir del menú de configuración
          Serial.println("presionado valor BPM");
        }
        mostrarParametroMenu(bpm.nom, String(bpm.valor[bpm.posActual]));
        presionado = false;
        break;

      case 2://RELACIÓN
        if (millis() - tBoton > tRebote) {
          if (pUp) {
            relacion.posActual = (relacion.posActual + 1) % relacion.limite;
            presionado = true;
          } else if (pDown) {
            presionado = true;
            if (relacion.posActual >= 1) {
              relacion.posActual--;
            }
            else {
              relacion.posActual = relacion.limite - 1;
            }
          }
          tBoton = millis();
        }
        if (pConfirmacion) {
          presionado = true;
          grabarEEPROM10(EErelacion, String(relacion.valor[relacion.posActual]));
          grabarEEPROM10(EEposRelacion, String(relacion.posActual));
          Serial.print("Configuración Guardada");
          mostrarConfirmacion();
        }
        if (presionado) {
          tiempoBoton = millis(); //----- Reinicia el contador del rebote
          tIniMenu = millis();   //----- Reinicia el contador para salir del menú de configuración
          Serial.println("presionado valor Relación I:E");
        }
        xRelacion = "1:" + String(relacion.valor[relacion.posActual]);
        mostrarParametroMenu(relacion.nom, xRelacion);
        presionado = false;
        break;

      case 3://PRESIÓN MÁXIMA
        if (millis() - tBoton > tRebote) {
          if (pUp) {
            presMax.posActual = (presMax.posActual + 1) % presMax.limite;
            presionado = true;
          } else if (pDown) {
            presionado = true;
            if (presMax.posActual >= 1) {
              presMax.posActual--;
            }
            else {
              presMax.posActual = presMax.limite - 1;
            }
          }
          tBoton = millis();
        }
        if (pConfirmacion) {
          presionado = true;
          grabarEEPROM10(EEpresMax, String(presMax.valor[presMax.posActual]));
          grabarEEPROM10(EEposPresMax, String(presMax.posActual));
          Serial.print("Configuración Guardada");
          mostrarConfirmacion();
        }
        if (presionado) {
          tiempoBoton = millis(); //----- Reinicia el contador del rebote
          tIniMenu = millis();   //----- Reinicia el contador para salir del menú de configuración
          Serial.println("presionado valor Presion Maxima");
        }
        mostrarParametroMenu(presMax.nom, String(presMax.valor[presMax.posActual]));
        presionado = false;
        break;
      default:
        tIniMenu = tMaxMenu;
        break;
    }
  }
}
