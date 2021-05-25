#include <WiFi.h>
#include <Wire.h>
#include <sys/time.h>
#include <U8g2lib.h>  //biblioteca da tela OLED
#include <ccs811.h>                     
#include <ClosedCube_HDC1080.h>
#include <Adafruit_BMP280.h>            
#include <Adafruit_Si7021.h>
#include "tabela.h"

#define S0 12
#define S1 13
#define S2 14
#define S3 15
#define S4 16
#define S5 17
#define S6 18
#define S7 19
#define ledAz 2
#define ledVd 4
#define ledAm 5
#define ledVm 23
#define buzz  25

const char* ssid       = "NOME DA REDE WiFi";     //altere com os dados de sua rede
const char* password   = "SENHA DA REDE WiFi";    //altere com os dados de sua rede

//TwoWire I2Cone = TwoWire(0);
//TwoWire I2Ctwo = TwoWire(1);

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); //instancia o módulo de tela OLED
CCS811 ccs811;
Adafruit_BMP280 bmp280;
Adafruit_Si7021 Si7021 = Adafruit_Si7021();
ClosedCube_HDC1080 hdc1080;

RTC_DATA_ATTR time_t now;
RTC_DATA_ATTR struct tm * timeinfo;

bool is_Si7021;
bool is_hdc1080;
bool is_ccs811;
bool is_bmp280;
float temperatura, umidade, pressao;
uint16_t CO2, TVOC;

uint8_t hora, minuto, segundo, dia, mes, semana;
uint16_t ano;

const long  gmtOffset_sec = -3 * 3600;
const int   daylightOffset_sec = 0;

volatile bool btVermelhoPress = 0;
volatile bool btAmareloPress = 0;
volatile bool btVerdePress = 0;
volatile bool btAzulPress = 0;
void btVermelho() {btVermelhoPress = true;}
void btAmarelo()  {btAmareloPress  = true;}
void btVerde()    {btVerdePress    = true;}
void btAzul()     {btAzulPress     = true;}

void setup() {
  uint8_t pinosOUT [] = {ledAz, ledVd, ledAm, ledVm, S0, S1, S2, S3, S4, S5, S6, S7, buzz};
  uint8_t pinosIN  [] = {34, 35, 36, 39};
  for (uint8_t i = 0; i < 13; i++) {
    pinMode(pinosOUT[i], OUTPUT);
    if (i < 4) pinMode(pinosIN[i], INPUT);
  }
  u8g2.begin();               //inicializa display
  u8g2.enableUTF8Print();     //permite uso de caracteres especiais no display
  u8g2.setContrast(255);      //contraste no máximo
  msgInicio();
  ledcAttachPin(buzz, 0); //
  ledcSetup(0, 2000, 10);
                                                                                                                                                                                                                                                                                                                                                                    Wire.begin(21,22);
  if (Wifi()) configTime(gmtOffset_sec, daylightOffset_sec, "south-america.pool.ntp.org", "pool.ntp.org");
  //I2Cone.begin(32,33,200000); // SDA pin 21, SCL pin 22, 200kHz frequency: inicializa o Wire nos pinos SDA e SCL padrões do ESP32
  
  attachInterrupt(digitalPinToInterrupt(34), btAzul,     FALLING); //subrotina de acionamento do botão azul
  attachInterrupt(digitalPinToInterrupt(35), btVerde,    FALLING); //subrotina de acionamento do botão verde
  attachInterrupt(digitalPinToInterrupt(36), btAmarelo,  FALLING); //subrotina de acionamento do botão amarelo
  attachInterrupt(digitalPinToInterrupt(39), btVermelho, FALLING); //subrotina de acionamento do botão vermelho

  for (uint8_t i = 12; i < 20;  i++) digitalWrite(i, HIGH);
  digitalWrite(ledVm, HIGH);
  digitalWrite(ledAm, HIGH);
  digitalWrite(ledVd, HIGH);
  digitalWrite(ledAz, HIGH);
  while(1);
}

void loop() {
  static uint8_t x = 12;
  static bool estado = HIGH;
  
  for (uint8_t i = 12; i < 20;  i++) digitalWrite(i, HIGH);
  u8g2.clearBuffer();

  printLocalTime();
  u8g2.sendBuffer();

  if (btVermelhoPress)  {
    digitalWrite(ledVm, HIGH);
    digitalWrite(ledAz, LOW);
    ledcWrite(0, 512);
  }

  if (btAmareloPress) {
    digitalWrite(ledAm, HIGH);
  }
  if (btVerdePress) {
    digitalWrite(ledVd, HIGH);
  }

  if (btAzulPress) {
    digitalWrite(ledVm, LOW);
    digitalWrite(ledAm, LOW);
    digitalWrite(ledVd, LOW);
    digitalWrite(ledAz, HIGH);
    ledcWrite(0, 0);
  }
  
  btVermelhoPress = false;
  btAmareloPress  = false;
  btVerdePress    = false;
  btAzulPress     = false;
  
  return;
  
  for (int i = 0; i > -1;  i += x) {
    if (!btAzulPress && !btVerdePress && !btAmareloPress && !btVermelhoPress) return;                                                                              if (!btAzulPress && !btVerdePress && !btAmareloPress && !btVermelhoPress) return;
    if (btAzulPress)      ledcWrite(0, gama[i]);
    if (btVerdePress)     ledcWrite(1, gama[i]);
    if (btAmareloPress)   ledcWrite(2, gama[i]);
    if (btVermelhoPress)  ledcWrite(3, gama[i]);
    delayMicroseconds(500);
    if (i == 1023) x = -1;
  }
}

bool Wifi(){
  uint8_t wifiCounter;
  if (WiFi.status() == WL_CONNECTED) return true;
  
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  WiFi.begin(ssid, password);

  Serial.println("Inicia WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (++wifiCounter > 30) return (false); // restart after 15 s
  }
  return (true);
}

void printLocalTime() {
  time(&now);
  timeinfo = localtime (&now);
  String frase;
  
  segundo = timeinfo->tm_sec;
  minuto  = timeinfo->tm_min;
  hora    = timeinfo->tm_hour;
  dia     = timeinfo->tm_mday;
  mes     = timeinfo->tm_mon;
  ano     = timeinfo->tm_year + 1900;
  semana  = timeinfo->tm_wday;

  if (dia < 10) frase = "0";
  frase += dia;
  frase += "/";
  if (mes < 10) frase += "0";
  frase += mes;
  frase += " ";
  if (hora < 10) frase += "0";
  frase += hora;
  frase += ":";
  if (minuto < 10) frase += "0";
  frase += minuto;
  frase += ":";
  if (segundo < 10) frase += "0";
  frase += segundo;

  Console(frase, 1);
}

//---------------------------------------------------------------------------------

void Console (String mensagem, byte linha) {
  uint8_t  coluna = 0;
  
  if (mensagem.indexOf('\n') <  0) mensagem += '\n';
  u8g2.setFont(u8g2_font_courR10_tf);
  u8g2.setCursor(coluna, 13 * linha);
  u8g2.print(mensagem);
}

void msgInicio (){
  u8g2.clear();
  
  u8g2.setFont(u8g2_font_courR10_tf); 
  u8g2.setCursor(0, 13);
  u8g2.setDrawColor(1);
  u8g2.print(" # AUTOMAÇÃO #");
  u8g2.drawBox(0,16,127,47);
  u8g2.setFont(u8g2_font_fub30_tf);

  u8g2.setFontMode(0);
  u8g2.setDrawColor(0);
  u8g2.drawStr(1, 56, "ESP32");
  u8g2.sendBuffer();

  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_courR10_tf); 
}
