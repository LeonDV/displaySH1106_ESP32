//----- Display -----
void setupDisplay();
void mostrarReal(String v, String b, String i, String p, String c, String corr);
void mostrarParametroMenu(String param, String value);
void mostrarConfirmacion();

void clearDisplay();

//----- EEPROM -----
void setupEEPROMM(int x);
void grabarEEPROM10(int addr, String a);
String leerEEPROM10(int addr);
