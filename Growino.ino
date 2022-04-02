#define prototipo

#ifdef prototipo
  IPAddress local_IP(192,168,15,111);
  IPAddress gateway(192,168,15,1);
  IPAddress subnet(255,255,255,0);
  IPAddress primaryDNS(8,8,8,8);
  IPAddress secondaryDNS(8,8,4,4);
  String apiKey = "GET /update?key=T5BYWX3LLACXUNFY";    // canal ThinkSpeak PROTOTIPO
#else
  IPAddress local_IP(192,168,15,222);
  IPAddress gateway(192,168,15,1);
  IPAddress subnet(255,255,255,0);
  IPAddress primaryDNS(192,168,15,1);
  IPAddress secondaryDNS(192,168,15,1);
  String apiKey = "GET /update?key=VS4A5L3IHYY4DDZW";    // canal ThinkSpeak
#endif

#define sVent 12
#define sOzo  13
#define sAux  14
#define sExa  15
#define sLed1 16
#define sLed2 17
#define sLed3 18
#define sLed4 19
#define ledAz  2
#define ledVd  4
#define ledAm  5
#define ledVm 23
#define sBuzz 25

#define S1RX 32
#define S1TX 33

#define TEMPMAX 30
#define TEMPMIN 20

#define UMIDMAX 60
#define UMIDMIN 30

//################# LIBRARIES ################
#include <WiFi.h>
#include <WebServer.h>                  //https://github.com/Pedroalbuquerque/ESP32WebServer download and place in your Libraries folder
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <sys/time.h>
#include <U8g2lib.h>                    //biblioteca da tela OLED
#include <ccs811.h>                     // include library for CCS811 - Sensor from martin-pennings https://github.com/maarten-pennings/CCS811
#include <ClosedCube_HDC1080.h>
#include <Adafruit_BMP280.h>            // include main library for BMP280 - Sensor
#include <Adafruit_Si7021.h>
#include <SPIFFS.h>
#include <EEPROM.h>
#include <rom/rtc.h>

const char* serverName = "http://api.thingspeak.com/update";

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); //instancia o módulo de tela OLED

CCS811 ccs811;
Adafruit_BMP280 bmp280;
Adafruit_Si7021 Si7021 = Adafruit_Si7021();
ClosedCube_HDC1080 hdc1080;

bool     recebeuIP;
bool     recebeuHora;
bool     is_Si7021;
bool     is_hdc1080;
bool     is_ccs811;
bool     is_bmp280;
float    temperatura, umidade, pressao;
float    tempBMP;
int8_t   tempCO2;
uint16_t CO2, TVOC;

int8_t   timezone = -3;
uint8_t  hora, minuto, segundo, dia, mes, semana;
uint16_t ano;

//################ VARIABLES ################
const char* ssid      = "Haroldinho";     // WiFi SSID
const char* password  = "pilcopilco";     // WiFi Password

WebServer server(80);

String sitetitle      = "GROWINO";               // Appears on the tabe of a Web Browser
String yourfootnote   = "ESP32 Webserver Demonstration"; // A foot note e.g. "My Web Site"

#define sitewidth 880 //1024  // Adjust site page width in pixels as required

String webpage = ""; // General purpose variable to hold HTML code

String field1, field4, field5 , CheckBoxChoice = "";
float  field2 ;
int    field3 ;


const char* serverIndex = 
"<!DOCTYPE html><html><head>"
"<meta http-equiv='refresh' content='600'>"
"<style>"
"body {width:880px;margin:0 auto;font-family:arial;font-size:14px;text-align:center;color:blue;background-color:#F7F2Fd;}"
"h1 {background-color:#ffc66e;margin:16px 30px;}"
"h3 {color:#9370DB;font-size:24px;width:auto;}"
".navbar{overflow:hidden;background-color:#558ED5;position:fixed;top:0;width:880px;margin-left:30px;}"
".navbar a {float:left;display:block;color:yellow;text-align:center;padding:10px 12px;text-decoration: none;font-size:17px;}"
".main{padding:0px;margin:16px;height:1000px;width:880px;}"
".style1{text-align:center;font-size:16px;background-color:#FFE4B5;}"
".style2{text-align:left;font-size:16px;background-color:#F7F2Fd;width:auto;margin:0 auto;}"
"</style>"
"</head><body>"
"<div class='navbar'>"

" <a href='/homepage'>   Home</a>"
" <a href='/programar'>  Programar</a>"
" <a href='/verificar'>  Verificar</a>"
" <a href='/nomes'>      Nomes</a>"
" <a href='/telemetria'> Telemetria</a>"
" <a href='/alertas'>    Alertas</a>"
" <a href='/atualizar'>  Atualizar</a>"
" <a href='/wifiweb'>    WiFi</a>"
" <a href='/reiniciar'>  Reiniciar</a>"
" <a href='/sobre'>      Sobre</a>"
"</div>"
"<br><title>1024</title><br>"
"<br><br>"
"<h3>Atualizar</h3>"
"<br>"
"</html>"

"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";

const char* loginIndex = 
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<td>Username:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/homepage')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";

void WiFiEvent(WiFiEvent_t event, system_event_info_t info) {
  //Serial.println(event);
  switch (event) {
    case SYSTEM_EVENT_STA_LOST_IP:
      WiFi.disconnect(true);
      break;
    case SYSTEM_EVENT_STA_STOP:
      WiFi.mode(WIFI_MODE_STA);
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      //conectadoWiFi = true;
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      recebeuIP = true;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      WiFi.reconnect();
      break;
    default:
      break;
  }
}

typedef struct {
  bool    estado;
  bool    emergencia;
  bool    intermitente;
  uint8_t Mcada;
  uint8_t vezesH;
  uint8_t pino;
  uint8_t horaLiga;
  uint8_t minutoLiga;
  uint8_t horaDesliga;
  uint8_t minutoDesliga;
  String  nome;
} SolidStateRelay;
SolidStateRelay SSR[8];

void setup(){
  Serial.begin(115200);

  iniciaMemoria();

  xTaskCreatePinnedToCore(loop_Principal, "loop_Principal", 6000, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(loop_I2C,       "loop_I2C",   6000, NULL, 4, NULL, 0);
  //xTaskCreatePinnedToCore(loop_Atuar,   "loop_Atuar", 6000, NULL, 6, NULL, 0);

  //vTaskDelay (500 / portTICK_RATE_MS);

}

void loop () {
  Serial.println("Iniciou LOOP");
  WiFi.mode(WIFI_OFF);
  WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
  WiFi.mode(WIFI_MODE_STA);
  WiFi.onEvent(WiFiEvent);
  WiFi.begin(ssid, password);

  while (!recebeuIP) vTaskDelay (250 / portTICK_RATE_MS);
  Serial.print("IP ");
  Serial.println(WiFi.localIP().toString());

  digitalWrite(ledVm, LOW);   //VERMELHO

  relogioNTP();
  EEPROM.writeULong(80, time(NULL) + timezone * 3600);
  EEPROM.commit();
  
  digitalWrite(ledAm, LOW);   //AMARELO
  
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/homepage",  homepage);   
  server.on("/programar", programar); 
  server.on("/reiniciar", reiniciarWeb);
  server.on("/resetOK",   resetOK);
  server.on("/alertas",   alertas);
  server.on("/atualizar", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.on("/telemetria", telemetria);
  server.on("/verificar",  verificar);
  server.on("/nomes",      nomes);
  server.on("/wifiweb",    wifiweb);
  server.on("/sobre",      sobre);  
  server.onNotFound(handleNotFound); 
  server.begin();
  Serial.println(F("Webserver iniciado")); // Inicia webserver

  digitalWrite(ledAz, LOW);   //AZUL
 
  while (1) server.handleClient();
}

void iniciaMemoria(){
  Serial.println("Iniciando Memória");
  if (!SPIFFS.begin()) SPIFFS.format();
  EEPROM.begin(100);
  for (uint8_t i = 0; i < 8; i++){
    SSR[i].intermitente  = EEPROM.read (i * 7 + 1);
    SSR[i].vezesH        = EEPROM.read (i * 7 + 2);
    SSR[i].Mcada         = EEPROM.read (i * 7 + 3);
    SSR[i].horaLiga      = EEPROM.read (i * 7 + 4);
    SSR[i].minutoLiga    = EEPROM.read (i * 7 + 5);
    SSR[i].horaDesliga   = EEPROM.read (i * 7 + 6);
    SSR[i].minutoDesliga = EEPROM.read (i * 7 + 7);
    SSR[i].nome          = leituraMemo (i);
  }
}
String leituraMemo(uint8_t nome) {
  File f = SPIFFS.open("/" + String(nome) + ".txt", "r");
  if (!f) return "";
  while (f.available()) {
    String conteudo = f.readStringUntil('\r');
    f.close();
    return conteudo;
  }
  return "";
}
void salvaMemo(uint8_t nome, String texto) {
  File f = SPIFFS.open("/" + String(nome) + ".txt", "w+");
  if (!f) {
  Serial.println("Erro ao salvar SPIFFS");
    return;
  }
  f.println(texto);
  f.close();
}
void loop_I2C (void *pvParameter) {
  u8g2.begin();               //inicializa display
  //u8g2.setBusClock(100000);
  u8g2.enableUTF8Print();     //permite uso de caracteres especiais no display
  u8g2.setContrast(255);      //contraste no máximo]
  u8g2.setPowerSave(0);
  u8g2.setFlipMode(0);
  msgInicio();

  iniciaSensores();
  iniciaGas();

  while (!recebeuHora) vTaskDelay (100 / portTICK_RATE_MS);
  while (1) {
    vTaskDelay (100 / portTICK_RATE_MS);

    if (!leSensores()) iniciaSensores();
    if (!leGas())      iniciaGas();
    verificaAlertas();

    if      (segundo % 9 > 6) mostraCO2();
    else if (segundo % 9 > 3) mostraUmidade();
    else                      mostraTemperatura();
  }
}

void loop_Principal (void *pvParameter) {
  uint8_t segundo_Anterior;
  uint8_t pinosOUT [] = {sVent, sOzo, sAux, sExa, sLed1, sLed2, sLed3, sLed4, sBuzz, ledAz, ledVd, ledAm, ledVm};
  uint8_t pinosIN  [] = {34, 35, 36, 39};

  for (uint8_t i = 0; i < 13; i++) {
    pinMode (pinosOUT [i], OUTPUT);
    if (i < 4) pinMode(pinosIN [i], INPUT);
    if (i < 8) SSR[i].pino = pinosOUT [i];
    if (i > 8) digitalWrite(pinosOUT [i], HIGH);
    #ifdef prototipo 
      digitalWrite(pinosOUT [i], HIGH);
    #endif   
  }

  while (!recebeuHora) vTaskDelay (250 / portTICK_RATE_MS);
  while(1){

    digitalWrite(ledVd, LOW);
    vTaskDelay (100 / portTICK_RATE_MS);
    digitalWrite(ledVd, HIGH);
    vTaskDelay (100 / portTICK_RATE_MS);
    
    relogio();
    //if (segundo == segundo_Anterior) continue;

    segundo_Anterior = segundo;
    if (hora == 23 && minuto == 45 && segundo == 0) relogioNTP();
    for (uint8_t i = 0; i < 8; i++) {
      if (SSR[i].horaLiga + SSR[i].minutoLiga + SSR[i].horaDesliga + SSR[i].minutoDesliga == 0) {
        SSR[i].estado = false;
        continue;
      }

      if (SSR[i].horaLiga <= SSR[i].horaDesliga) {                //9 - hora - 10
        if (hora >= SSR[i].horaLiga && hora <= SSR[i].horaDesliga) {
          if (hora == SSR[i].horaDesliga && minuto >= SSR[i].minutoDesliga) SSR[i].estado = false;
          else if (SSR[i].intermitente) {
            uint8_t PeriodoIntermitencia = 60 / SSR[i].vezesH;    //20min liga 5min
            if (minuto % PeriodoIntermitencia < SSR[i].Mcada) SSR[i].estado = true;
            else SSR[i].estado = false;
          }
          else SSR[i].estado = true;
        }
        else SSR[i].estado = false;
      }
      else if (hora >= SSR[i].horaLiga || hora < SSR[i].horaDesliga) {   //19 - hora - 9
        if (SSR[i].intermitente) {
          uint8_t PeriodoIntermitencia = 60 / SSR[i].vezesH;    //20min liga 5min
          if (minuto % PeriodoIntermitencia < SSR[i].Mcada) SSR[i].estado = true;
          else SSR[i].estado = false;
        } else SSR[i].estado = true;
      } else SSR[i].estado = false;
    }
    atuar ();
  }
}

void atuar() {
  for (uint8_t i = 0; i < 8; i++) {
    if      (SSR[i].emergencia == true) digitalWrite (SSR[i].pino, false);
    else if (SSR[i].estado == true) {
        //Serial.print(SSR[i].pino);
        //Serial.print(" ");
        digitalWrite (SSR[i].pino, true);
      }
    else digitalWrite (SSR[i].pino, false);
  }
}

void verificaAlertas () {
  //if   (temperatura > TEMPMAX || temperatura < TEMPMIN) led1 = true;
  //else led1 = false;
  
  //if   (umidade > UMIDMAX || umidade < UMIDMIN) led2 = true; 
  //else led2 = false; 
}

void enviaThing() {       ////////////////////////////////////////////////////////////////////////////////////////////
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    
    // Your Domain name with URL path or IP address with path
    http.begin(serverName);
    // Specify content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    // Data to send with HTTP POST
    String httpRequestData = "api_key=" + apiKey + "&field1=" + String(CO2) + "&field2=" + String(temperatura) + "&field3=" + String(umidade);           
    // Send HTTP POST request
    int httpResponseCode = http.POST(httpRequestData);

    /*
    // If you need an HTTP request with a content type: application/json, use the following:
    http.addHeader("Content-Type", "application/json");
    // JSON data to send with HTTP POST
    String httpRequestData = "{\"api_key\":\"" + apiKey + "\",\"field1\":\"" + String(random(40)) + "\"}";           
    // Send HTTP POST request
    int httpResponseCode = http.POST(httpRequestData);*/

    http.end();
  }
}

void iniciaSensores(){
  Serial.println("Iniciando sensores");
  if (Si7021.begin()) is_Si7021 = true;
  else if (hdc1080.begin(0x40)) is_hdc1080 = true;

  if (ccs811.begin()) {
    ccs811.start(CCS811_MODE_IDLE);     //ccs811.start(CCS811_MODE_1SEC)
    is_ccs811 = true;
  }
  if (bmp280.begin(0x76)) is_bmp280 = true;
}

void relogioNTP(){
  uint8_t  n = 0;
  uint16_t tempoDelay = 50;
  struct tm timeInfo;
  String   listaNTP[] = {"a.st1.ntp.br", "gps.ntp.br", "south-america.pool.ntp.org", "europe.pool.ntp.org", "pool.ntp.br"};

  configTime(timezone * 3600, 0, listaNTP[n].c_str());
  vTaskDelay(tempoDelay / portTICK_RATE_MS);

    while (!getLocalTime(&timeInfo)) {
      tempoDelay *= 3;
      vTaskDelay(tempoDelay / portTICK_RATE_MS);
      Serial.print("_relogioNTP: * ERRO servidor NTP: ");
      Serial.println(listaNTP[n]);
      if (++n > 4) ESP.restart();
      configTime((timezone * 3600), 0, listaNTP[n].c_str());
    }
  Serial.print("RelogioNTP: ");
  Serial.println(&timeInfo, "%A, %B %d %Y %H:%M:%S");
  
  recebeuHora = true;
}

void relogio() {
  String frase;
  struct tm data_hora_atual;
  getLocalTime(&data_hora_atual);

  segundo = data_hora_atual.tm_sec;
  minuto  = data_hora_atual.tm_min;
  hora    = data_hora_atual.tm_hour;
  dia     = data_hora_atual.tm_mday;
  mes     = data_hora_atual.tm_mon + 1;
  ano     = data_hora_atual.tm_year + 1900;
  semana  = data_hora_atual.tm_wday;

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

  frase += ":";
  if (minuto < 10) frase += "0";
  frase += minuto;
  frase += ":";
  if (segundo < 10) frase += "0";
  frase += segundo;
  
  //Serial.println(frase);

}

void mostraTemperatura(){
  int8_t altura;
  
  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.setDrawColor(1);

  altura = map(temperatura, 20, 30, 64, 0);
  if (altura < 0) altura = 0;
  u8g2.drawBox(0, altura, 128, 64);
  u8g2.setDrawColor(2);
  u8g2.setFont(u8g2_font_courB10_tf); 
  u8g2.setCursor(0, 13);

  u8g2.print("  TEMPERATURA");
  u8g2.setFont(u8g2_font_fub35_tf);
  
  u8g2.setCursor(6, 58);
  u8g2.print(String(temperatura, 1) + char(176));
  u8g2.sendBuffer();
}

void mostraUmidade(){
  int8_t altura;
  
  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.setDrawColor(1);

  altura = map(umidade, 40, 60, 64, 0);
  if (altura < 0) altura = 0;
  u8g2.drawBox(0, altura, 128, 64);
  u8g2.setDrawColor(2);
  u8g2.setFont(u8g2_font_courB10_tf); 
  u8g2.setCursor(0, 13);

  u8g2.print("    UMIDADE");
  u8g2.setFont(u8g2_font_fub35_tf);
  
  u8g2.setCursor(12, 58);
  u8g2.print(String(umidade, 0) + "%");
  u8g2.sendBuffer();
}


void mostraCO2 (){
  int8_t altura;
  
  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.setDrawColor(1);

  altura = map(CO2, 400, 1250, 64, 0);
  if (altura < 0) altura = 0;
  u8g2.drawBox(0, altura, 128, 64);
  u8g2.setDrawColor(2);
  u8g2.setFont(u8g2_font_courB10_tf); 
  u8g2.setCursor(0, 13);

  u8g2.print("   CO2  " + String(tempCO2) + char(176));
  u8g2.setFont(u8g2_font_fub35_tf);
  
  if (CO2 > 999)  u8g2.setCursor(6, 58);
  else u8g2.setCursor(22, 58);
  u8g2.print(String(CO2));
  u8g2.sendBuffer();
}


bool leSensores(){
  uint16_t eco2, etvoc, errstat, raw;     // Read CCS811
    
  if (is_Si7021) {
    umidade     = Si7021.readHumidity();
    temperatura = Si7021.readTemperature();
  }
  else if (is_hdc1080) {
    umidade     = hdc1080.readHumidity();
    temperatura = hdc1080.readTemperature();
  }
  vTaskDelay (60 / portTICK_RATE_MS);
  if (is_ccs811){
    ccs811.set_envdata(temperatura - 2, umidade);
    ccs811.read(&eco2, &etvoc, &errstat, &raw);
    if (errstat == CCS811_ERRSTAT_OK) {
      CO2  = eco2;
      TVOC = etvoc;
    }
  }
  vTaskDelay (60 / portTICK_RATE_MS);
  if (is_bmp280) {
    tempBMP = bmp280.readTemperature();
    pressao = bmp280.readPressure() / 100;
  }
    /*Serial.print(umidade);
    Serial.print("% ");
    Serial.print(temperatura);
    Serial.print("° ");
    Serial.println(tempBMP);*/
  if (isnan(temperatura) || isnan(umidade)) return false;
  return true;
}

bool leGas(){
  uint8_t  response[9] = {0}, check = 0;
  static uint8_t contaErro;
  const byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
    
  for (uint8_t i = 0; i < 9; i++) Serial1.write(cmd[i]);
  if (Serial1.available()) {
    for(int i=0; i < 9; i++) {
      response[i] = Serial1.read();
    }
    for(uint8_t i = 1; i < 8; i++) check += response[i];
    if ((255 - check) + 1 == response[8]) {
      contaErro = 0;
      CO2     = word(response[2], response[3]);
      tempCO2 = response[4] - 42;
      if (CO2 == 0) {tempCO2 = 0; contaErro++;}
    } else contaErro++;
  } else contaErro++;
  if (contaErro > 50) {CO2 = 1; iniciaGas(); contaErro = 0; return false;}
  return true;
}


void Console (String mensagem, byte linha) {
  uint8_t  coluna = 0;
  
  if (mensagem.indexOf('\n') <  0) mensagem += '\n';
  u8g2.setFont(u8g2_font_courR10_tf);
  u8g2.setCursor(coluna, 13 * linha);
  u8g2.print(mensagem);
}

void msgInicio (){  
  u8g2.clear();
  u8g2.setFontMode(1);
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_courR10_tf); 
  u8g2.setCursor(0, 13);
  u8g2.print(" # AUTOMAÇÃO #");
  u8g2.drawBox(0,16,128,48);
  u8g2.setFont(u8g2_font_fub30_tf);
  u8g2.setDrawColor(2);
  u8g2.drawStr(1, 56, "ESP32");
  u8g2.sendBuffer();

  //u8g2.setDrawColor(1);
  //u8g2.setFont(u8g2_font_courR10_tf); 
}

void iniciaGas(){
  Serial1.end();
  vTaskDelay(500 / portTICK_RATE_MS);
  Serial1.begin(9600, SERIAL_8N1, S1RX, S1TX);
}


void handleNotFound(){
  String message = "The request entered could not be found, please try again with a different option\n";
  server.send(404, "text/plain", message);
}

void homepage() {
  append_HTML_header();
  //webpage += "<div class='main'><h1> ControlESP #Automa&ccedil;&atilde;o </h1>";
  webpage += "<h1> ControlESP #Automa&ccedil;&atilde;o </h1>";
  webpage += "<br>";
  webpage += "<P class='style2'>Praticidade na Automa&ccedil;&atilde;o!";
  webpage += "<br><br>";
  webpage += "<b>ControlESP</b> &eacute; um aparelho IoT - <i>Internet of Things</i> - da Internet das Coisas. Utiliza o microcontrolador ESP32 que possui Bluetooth e WiFi embarcados.";
  webpage += "<br><br>";
  webpage += "Com este hardware &eacute; poss&iacute;vel controlar 8 sa&iacute;das de pot&ecirc;ncia, al&eacute;m de possuir mais 9 conex&otilde;es ";
  webpage += "dispon&iacute;veis para serem utilizadas como entradas ou sa&iacute;das digitais ou anal&oacute;gicas.";
  webpage += "<br><br>";
  webpage += "ControlESP possui display OLED, 4 bot&otilde;es retroiluminados program&aacute;veis, fonte de alimenta&ccedil;&atilde;o e sa&iacute;das de tens&atilde;o 3V3 e 5V.";
  webpage += "<br><br>";
  webpage += "Todas as entradas e sa&iacute;das est&atilde;o dispon&iacute;veis atrav&eacute;s de bornes, garantindo seguran&ccedil;a e praticidade nas conex&otilde;es.";
  webpage += "<br><br>";
  webpage += "ControlESP foi desenvolvido na plataforma Arduino, possuindo c&oacute;digo fonte aberto - <i>open source</i>.";
  webpage += "<br><br>";
  webpage += "Todas estas caracter&iacute;sticas permitem ao usu&aacute;rio personalizar a aplica&ccedil;&atilde;o conforme a sua necessidade.";
  webpage += "</p>";
  webpage += "<br><br><br>";
  append_HTML_footer();
  server.send(200, "text/html", webpage);
}

void programar() {
  bool    interOP = false;
  bool    sempreLigado = false;
  uint8_t opcoes;
  uint8_t ligaH, ligaM, desligaH, desligaM, vezesH, Mcada;
  String  horaString, minString;
  String  client_response;
  String  Argument_Name;

  webpage = ""; // don't delete this command, it ensures the server works reliably!
  append_HTML_header();
  String IPaddress = WiFi.localIP().toString();
  webpage += "<br><br>";
  webpage += "<h3>Programar</h3>";
  webpage += "<br><br>";
  webpage += "<form action=\"http://"+IPaddress+"/programar\" method=\"POST\">";
  webpage += "<label><input type='checkbox' name='CheckBoxChoice' value='1'>" + SSR[0].nome + "</label> ";
  webpage += "<label><input type='checkbox' name='CheckBoxChoice' value='2'>" + SSR[1].nome + "</label> ";
  webpage += "<label><input type='checkbox' name='CheckBoxChoice' value='4'>" + SSR[2].nome + "</label> ";
  webpage += "<label><input type='checkbox' name='CheckBoxChoice' value='8'>" + SSR[3].nome + "</label> ";
  webpage += "<label><input type='checkbox' name='CheckBoxChoice' value='16'>"  + SSR[4].nome + "</label> ";
  webpage += "<label><input type='checkbox' name='CheckBoxChoice' value='32'>"  + SSR[5].nome + "</label> ";
  webpage += "<label><input type='checkbox' name='CheckBoxChoice' value='64'>"  + SSR[6].nome + "</label> ";
  webpage += "<label><input type='checkbox' name='CheckBoxChoice' value='128'>" + SSR[7].nome + "</label>";
  webpage += "<br><br><br>";
  webpage += "<label><input type='checkbox' name='CheckBoxSempre' value='sempre'> Sempre ligado</label>";
  webpage += "<br><br>";
  webpage += "<label> LIGADO_ ";
  webpage += "<input type='time' maxlength='2' style='text-align:center' name='timeLiga' value='' size='1'></label>";
  webpage += "<br><br>";
  webpage += "<label>DESLIGA ";
  webpage += "<input type='time' maxlength='2' style='text-align:center' name='timeDesliga' value='' size='1'></label>";
  webpage += "<br><br>";
  webpage += "<label><input type='checkbox' name='CheckInter' value='inter'>Intermitente: liga por </label>";
  webpage += "<input type='text' maxlength='2' style='text-align:center' name='Mcada' size='1'>";
  webpage += "<label for='estilo'> minutos,  </label>";
  webpage += "<input list='text' maxlength='2' style='text-align:center' id='teste id' name='vezesH' size='1'>";
  webpage += "<datalist id='teste list'>";
  webpage += "</datalist>";
  webpage += "<label for='estilo'> vez(es) por hora.</label>";
  webpage += "<br><br><br><br>";
  webpage += "<input type='submit' value=' SALVAR '>";
  webpage += "<br><br><br>";
  webpage += "Obs.: 1. Para apagar, marque apenas a(s) sa&iacute;da(s) e salve;<br>";
  webpage += "2. O programa estar&aacute; salvo quando os campos forem apagados;<br>";
  webpage += "3. Confira o programa na tela VERIFICAR.<br>";
  webpage += "<br><br><br>";
  webpage += "</form></body>";
  append_HTML_footer();
  server.send(200, "text/html", webpage); // Send a response to the client to enter their inputs, if needed, Enter=defaults
  
  if (server.args() > 0 ) { // Arguments were received
    opcoes = 0;
    for ( uint8_t i = 0; i <= server.args(); i++ ) {
      Argument_Name   = server.argName(i);
      client_response = String(server.arg(i));
      if (Argument_Name == "CheckBoxChoice") opcoes += client_response.toInt();
      if (Argument_Name == "timeLiga") {
        Serial.println (client_response.length());
        horaString = client_response.substring(0, 2);
        client_response.remove(0, 3);
        minString  = client_response;
        ligaH = horaString.toInt();
        ligaM = minString.toInt();
      }
      if (Argument_Name == "timeDesliga")  {
        Serial.println (client_response.length());
        horaString = client_response.substring(0, 2);
        client_response.remove(0, 3);
        minString  = client_response;
        desligaH = horaString.toInt();
        desligaM = minString.toInt();
      }
      if (Argument_Name == "CheckInter" && client_response == "inter") interOP = true;
      if (Argument_Name == "CheckBoxSempre" && client_response == "sempre") sempreLigado = true;
      if (Argument_Name == "vezesH")    vezesH = client_response.toInt();
      if (Argument_Name == "Mcada")     Mcada = client_response.toInt();
    }
    if (opcoes > 0) {
      if (sempreLigado) {ligaH = 0; ligaM = 0; desligaH = 24; desligaM = 0;}
      for (uint8_t i = 0; i < 8; i++) {
        if (uint8_t(1 << i) & opcoes) {
          SSR[i].intermitente  = interOP;
          SSR[i].vezesH        = vezesH;
          SSR[i].Mcada         = Mcada;
          SSR[i].horaLiga      = ligaH;
          SSR[i].minutoLiga    = ligaM;
          SSR[i].horaDesliga   = desligaH;
          SSR[i].minutoDesliga = desligaM;
        }
      }
      
      for (uint8_t i = 0; i < 8; i++) {
        Serial.print (SSR[i].intermitente);
        Serial.print (" ");
        Serial.print (SSR[i].vezesH);
        Serial.print (" ");
        Serial.print (SSR[i].Mcada);
        Serial.print (" ");
        Serial.print (SSR[i].horaLiga);
        Serial.print (" ");
        Serial.print (SSR[i].minutoLiga);
        Serial.print (" ");
        Serial.print (SSR[i].horaDesliga);
        Serial.print (" ");
        Serial.println (SSR[i].minutoDesliga);
        EEPROM.write(i * 7 + 1, SSR[i].intermitente);
        EEPROM.write(i * 7 + 2, SSR[i].vezesH);
        EEPROM.write(i * 7 + 3, SSR[i].Mcada);
        EEPROM.write(i * 7 + 4, SSR[i].horaLiga);
        EEPROM.write(i * 7 + 5, SSR[i].minutoLiga);
        EEPROM.write(i * 7 + 6, SSR[i].horaDesliga);
        EEPROM.write(i * 7 + 7, SSR[i].minutoDesliga);
        EEPROM.commit();
      }
    }
  }
  //webpage = "";
  //homepage();
}

void wifiweb(){
  webpage = ""; // don't delete this command, it ensures the server works reliably!
  append_HTML_header();
  String IPaddress = WiFi.localIP().toString();
  webpage += "<br><br>";
  webpage += "<h3>WiFi</h3>";
  webpage += "<br><br>";
  webpage += "<form action=\"http://"+IPaddress+"/wifiweb\" method=\"POST\">";
  webpage += "<label> Endereco IP <input type='text' size='13' name='endIP' required></label>";
  webpage += "<br><br>";
  webpage += "<label>Nome da Rede <input type='text' size='15' name='nomeRede' required> </label>";
  webpage += "<br><br>";
  webpage += "<label>Senha da Rede <input type='text' size='15' name='senhaRede' required> </label>";

  webpage += "<br><br><br><br>";
  webpage += "<input type='submit' value=' SALVAR '><br>";
  webpage += "<br><br><br>";
  webpage += "Endere&ccedil;o IP atual: <b>" + WiFi.localIP().toString() + "</b><br>";
  webpage += "Rede WiFi atual: <b>" + WiFi.SSID() + "</b><br>";
  webpage += "<br><br>";
  webpage += "Novo IP ap&oacute;s reiniciar: <b>" + WiFi.localIP().toString() + "</b><br>";
  webpage += "Nova rede WiFi ap&oacute;s reiniciar: <b>" + WiFi.SSID() + "</b><br>";
  webpage += "<br><br>";

  append_HTML_footer();
  server.send(200, "text/html", webpage);
}

void sobre(){
  append_HTML_header(); 
  webpage += "<br><br>";
  webpage += "<H3>Sobre</H3>";
  webpage += "<p class='style2'>";
  webpage += "<table style='font-family:arial,sans-serif;font-size:16px;border-collapse:collapse;text-align:center;width:90%;margin-left:auto;margin-right:auto;'>";
  webpage += "<b>ControlESP #AUTOMA&Ccedil;&Atilde;O</b> foi desenvolvido pela empresa Projetando no Ap&ecirc;.<br>";
  webpage += "<br>";
  webpage += "Este software &eacute; um exemplo de aplica&ccedil;&atilde;o e pode ser alterado pelo usu&aacute;rio.<br>";
  webpage += "<br>";
  webpage += "Como o software &eacute; aberto e colaborativo, n&atilde;o possui suporte e n&atilde;o &eacute; comercializado.<br>";
  webpage += "</p>";
  webpage += "Vers&atilde;o 1.0.<br>";

  webpage += "<br><br><br>";

  append_HTML_footer();
  server.send(200, "text/html", webpage);
}
String print_reset_reason(RESET_REASON reason){
  switch (reason)  {
    case 1  : return ("Vbat power on reset"); break;
    case 3  : return ("Software reset digital core"); break;
    case 4  : return ("Legacy watch dog reset digital core"); break;
    case 5  : return ("Deep Sleep reset digital core"); break;
    case 6  : return ("Reset by SLC module, reset digital core"); break;
    case 7  : return ("Timer Group0 Watch dog reset digital core"); break;
    case 8  : return ("Timer Group1 Watch dog reset digital core"); break;
    case 9  : return ("RTC Watch dog Reset digital core"); break;
    case 10 : return ("Instrusion tested to reset CPU"); break;
    case 11 : return ("Time Group reset CPU"); break;
    case 12 : return ("Software reset CPU"); break;
    case 13 : return ("RTC Watch dog Reset CPU"); break;
    case 14 : return ("for APP CPU, reseted by PRO CPU"); break;
    case 15 : return ("Reset when the vdd voltage is not stable"); break;
    case 16 : return ("RTC Watch dog reset digital core and rtc module"); break;
    default : return ("NO_MEAN");
  }
}
void alertas(){
  time_t epoch = EEPROM.readULong(80);

  webpage = ""; // don't delete this command, it ensures the server works reliably!
  append_HTML_header();
  String IPaddress = WiFi.localIP().toString();
  webpage += "<br><br>";
  webpage += "<H3>Alertas</H3>";
  webpage += "<br>";
  webpage += "<form action=\"http://" + IPaddress + "/wifiweb\" method=\"POST\">";
  webpage += "CPU0 motivo de reset: <b>" + print_reset_reason(rtc_get_reset_reason(0)) + "</b><br>";
  webpage += "CPU1 motivo de reset: <b>" + print_reset_reason(rtc_get_reset_reason(1)) + "</b><br>";
  webpage += "&Uacuteltimo reset em: <b>" + String(asctime(gmtime(&epoch))) + "</b><br>";
  webpage += "<br><br><br>";
  append_HTML_footer();
  server.send(200, "text/html", webpage);
}
void reiniciarWeb() {
  webpage = "";
  append_HTML_header();
  String IPaddress = WiFi.localIP().toString();
  webpage += "<form action=\"http://"+IPaddress+"/resetOK\" method=\"POST\">";
  webpage += "<br><br>";
  webpage += "<h3>Reiniciar</h3>";
  webpage += "<br><br><input type='submit' name='botao' value=' REINICIAR '><br>";
  webpage += "<br><br><br>";
  webpage += "</body>";
  webpage += "</form>";  
  append_HTML_footer();
  server.send(200, "text/html", webpage);
}
void resetOK() {
  webpage = "";
  webpage += "<!DOCTYPE html><html><head>";
  webpage += "<body>Reiniciando... retorne ap&oacutes ControlESP iniciar</body>";
  webpage += "</html>";
  server.sendHeader("Connection", "close");
  server.send (200, "text/html", webpage);
  delay(1000);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.print("# SOLICITAÇÃO WebPage - REINICIANDO");
  ESP.restart();
}

void nomes() {
  String field[8];
  CheckBoxChoice = "";
  webpage = "";
  append_HTML_header();
  String IPaddress = WiFi.localIP().toString();
  webpage += "<br><br>";
  webpage += "<h3>Nomes das sa&iacute;das</h3>";
  webpage += "<form action=\"http://"+IPaddress+"/nomes\" method=\"POST\">";
  webpage += "<table style='font-family:arial,sans-serif;font-size:16px;border-collapse:collapse;text-align:center;width:80%;margin-left:auto;margin-right:auto;'>";
  webpage += "<br>";
  webpage += "<tr>";
  webpage += "<th style='border:0px solid black;text-align:center;padding:2px;'>Sa&iacuteda 1</th>";
  webpage += "<th style='border:0px solid black;text-align:center;padding:2px;'>Sa&iacuteda 2</th>";
  webpage += "<th style='border:0px solid black;text-align:center;padding:2px;'>Sa&iacuteda 3</th>";
  webpage += "<th style='border:0px solid black;text-align:center;padding:2px;'>Sa&iacuteda 4</th>";
  webpage += "<th style='border:0px solid black;text-align:center;padding:2px;'>Sa&iacuteda 5</th>";
  webpage += "<th style='border:0px solid black;text-align:center;padding:2px;'>Sa&iacuteda 6</th>";
  webpage += "<th style='border:0px solid black;text-align:center;padding:2px;'>Sa&iacuteda 7</th>";
  webpage += "<th style='border:0px solid black;text-align:center;padding:2px;'>Sa&iacuteda 8</th>";
  webpage += "</tr>";
  webpage += "<tr>";
  webpage += "<td style='border:0px solid black;text-align:center;padding:4px;'><input type='text' size='7' style='text-align:center' name='field1' value='" + SSR[0].nome + "'></td>";
  webpage += "<td style='border:0px solid black;text-align:center;padding:4px;'><input type='text' size='7' style='text-align:center' name='field2' value='" + SSR[1].nome + "'></td>";
  webpage += "<td style='border:0px solid black;text-align:center;padding:4px;'><input type='text' size='7' style='text-align:center' name='field3' value='" + SSR[2].nome + "'></td>";
  webpage += "<td style='border:0px solid black;text-align:center;padding:4px;'><input type='text' size='7' style='text-align:center' name='field4' value='" + SSR[3].nome + "'></td>";
  webpage += "<td style='border:0px solid black;text-align:center;padding:4px;'><input type='text' size='7' style='text-align:center' name='field5' value='" + SSR[4].nome + "'></td>";
  webpage += "<td style='border:0px solid black;text-align:center;padding:4px;'><input type='text' size='7' style='text-align:center' name='field6' value='" + SSR[5].nome + "'></td>";
  webpage += "<td style='border:0px solid black;text-align:center;padding:4px;'><input type='text' size='7' style='text-align:center' name='field7' value='" + SSR[6].nome + "'></td>";
  webpage += "<td style='border:0px solid black;text-align:center;padding:4px;'><input type='text' size='7' style='text-align:center' name='field8' value='" + SSR[7].nome + "'></td>";
  webpage += "</tr>";
  webpage += "</table><br><br>";
  webpage += "<br><br><input type='submit' value=' SALVAR '><br>";
  webpage += "<br><br>";
  webpage += "Obs.: Depois de salvar, atualize a p&aacute;gina.";
  webpage += "<br><br><br>";
  webpage += "</form></body>";
  append_HTML_footer();
  server.send(200, "text/html", webpage); // Send a response to the client to enter their inputs, if needed, Enter=defaults
  if (server.args() > 0 ) { // Arguments were received
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      String Argument_Name   = server.argName(i);
      String client_response = server.arg(i);
      if (Argument_Name == "field1") field[0] = client_response;
      if (Argument_Name == "field2") field[1] = client_response;
      if (Argument_Name == "field3") field[2] = client_response;
      if (Argument_Name == "field4") field[3] = client_response;
      if (Argument_Name == "field5") field[4] = client_response;
      if (Argument_Name == "field6") field[5] = client_response;
      if (Argument_Name == "field7") field[6] = client_response;
      if (Argument_Name == "field8") field[7] = client_response;
    }
    for (uint8_t i = 0; i < 8; i++) if (field[i] != SSR[i].nome) {
      SSR[i].nome = field[i];
      salvaMemo(i, field[i]);
      Serial.print("SSR[" + String(i) + "].nome ");
      Serial.println (SSR[i].nome);
    }
  }
}

void telemetria(){
  append_HTML_header();
  webpage += "<br><br>";
  webpage += "<H3>Leitura dos sensores</H3>";
  webpage += "<br>";
  webpage += "<table style='font-family:arial,sans-serif;font-size:16px;border-collapse:collapse;text-align:center;width:75%;margin-left:auto;margin-right:auto;'>";
  webpage += "<tr>";
  webpage += " <th style='border:0px solid black;padding:2px;'>Temperatura</th>";
  webpage += " <th style='border:0px solid black;padding:2px;'>Umidade</th>";
  webpage += " <th style='border:0px solid black;padding:2px;'>Press&atilde;o</th>";
  webpage += " <th style='border:0px solid black;padding:2px;'>CO2</th>";
  webpage += " <th style='border:0px solid black;padding:2px;'>tempCO2</th>";
  webpage += " <th style='border:0px solid black;padding:2px;'>Hor&aacute;rio</th>";
  webpage += "</tr>";
  webpage += "<tr>";
  webpage += " <td style='border:0px solid black;padding:2px;'>"+String(temperatura,2)+"&deg;C</td>";
  webpage += " <td style='border:0px solid black;padding:2px;'>"+String(umidade,2)+" %</td>";
  webpage += " <td style='border:0px solid black;;padding:2px;'>"+String(pressao,2)+" hPa</td>";
  webpage += " <td style='border:0px solid black;padding:2px;'>"+String(CO2)+" ppm</td>";
  webpage += " <td style='border:0px solid black;padding:2px;'>"+String(tempCO2)+"</td>";
  webpage += " <td style='border:0px solid black;padding:2px;'>"+String(hora)+":"+String(minuto)+":"+String(segundo)+"</td>";
  webpage += "</tr></table>";
  webpage += "<br><br>";
  append_HTML_footer();
  server.send(200, "text/html", webpage);
}


boolean isValidNumber(String str) {
  str.trim();
  if(!(str.charAt(0) == '+' || str.charAt(0) == '-' || isDigit(str.charAt(0)))) return false; // Failed if not starting with +- or a number
  for(byte i=1;i<str.length();i++) {if(!(isDigit(str.charAt(i)) || str.charAt(i) == '.')) return false;} // Anything other than a number or . is a failure
  return true;
}

void verificar(){
  String frase, temp;
  String IPaddress = WiFi.localIP().toString();

  webpage = ""; // don't delete this command, it ensures the server works reliably!
  append_HTML_header();
  webpage += "<br><br>";
  webpage += "<h3>Verificar programa&ccedil;&atilde;o</h3>";
  webpage += "<form action=\"http://" + IPaddress + "/verificar\" method=\"POST\">";
  webpage += "<br>";
  for (uint8_t i = 0; i < 8; i++){
    String Hliga;
    String Hdesliga;
    String Mliga;;
    String Mdesliga;
    if (SSR[i].horaLiga < 10)      Hliga = "0";
    Hliga += String(SSR[i].horaLiga);
    if (SSR[i].minutoLiga < 10)    Mliga = "0";
    Mliga += String(SSR[i].minutoLiga);
    if (SSR[i].horaDesliga < 10)   Hdesliga = "0";
    Hdesliga += String(SSR[i].horaDesliga);
    if (SSR[i].minutoDesliga < 10) Mdesliga = "0";
    Mdesliga += String(SSR[i].minutoDesliga);

    //webpage += "<p style=font-size:16px;'> </p>";
    //if (SSR[i].estado) webpage += "<p style='background-color:#ffc66e;' style=font-size:16px;'>";
    if (SSR[i].estado) webpage += "<mark>";
    webpage += String(i + 1) + " - ";
    webpage += "<u>" + SSR[i].nome + "</u><br>";
    if (SSR[i].horaLiga + SSR[i].minutoLiga + SSR[i].horaDesliga + SSR[i].minutoDesliga == 0) 
      webpage += "<i>(sempre desligado)</i><br>"; 
    else webpage += "<b>" + Hliga    + ":" + Mliga + " - " + Hdesliga + ":" + Mdesliga + "</b><br>";
    if (SSR[i].intermitente) {
      if (SSR[i].Mcada > 1) webpage += "<i>(liga por "  + String(SSR[i].Mcada) + " minutos, ";
      else webpage += "<i>(liga por "  + String(SSR[i].Mcada) + " minuto, ";
      if (SSR[i].vezesH > 1) webpage += String(SSR[i].vezesH) + " vezes por hora)</i><br>";
      else webpage += String(SSR[i].vezesH) + " vez por hora)</i><br>";
    }
    else if (SSR[i].horaDesliga == 24) webpage += "<i>(sempre ligado)</i><br>"; 
    webpage += "</mark>";
    webpage += "<br>";
  }
  //bool    emergencia;
  webpage += "<br>";
  if (dia < 10) temp = "0";
  else temp = "";
  frase = temp + String(dia) + "/";

  if (mes < 10) temp = "0";
  else temp = "";
  frase += temp + String(mes);
  frase += "   &agrave;s  ";

  if (hora < 10) temp = "0";
  else temp = "";
  frase += temp + String(hora) + ":";

  if (minuto < 10) temp = "0";
  else temp = "";
  frase += temp + String(minuto) + ":";

  if (segundo < 10) temp = "0";
  else temp = "";
  frase += temp + String(segundo);
  frase += ".";
  
  webpage += "<br>";
  webpage += "Situa&ccedil;&atilde;o em " + frase;
  webpage += "<br><br><br>";
  append_HTML_footer();
  server.send(200, "text/html", webpage);
}

void append_HTML_header() {
  webpage  = "";
  webpage += "<!DOCTYPE html><html><head>";
  webpage += "<meta http-equiv='refresh' content='600'>"; // 5-min refresh time
  webpage += "<style>";
  webpage += "body {width:"+String(sitewidth)+"px;margin:0 auto;font-family:arial;font-size:14px;text-align:center;color:blue;background-color:#F7F2Fd;}";
  webpage += "h1 {background-color:#ffc66e;margin:30px;width:" +String(sitewidth)+ "px;}";
  webpage += "h3 {color:#9370DB;font-size:24px;width:auto;}";
  webpage += ".navbar{overflow:hidden;background-color:#558ED5;position:fixed;top:0;width:"+String(sitewidth)+"px;margin-left:30px;}";
  webpage += ".navbar a {float:left;display:block;color:yellow;text-align:center;padding:10px 12px;text-decoration: none;font-size:17px;}";
  webpage += ".main{padding:0px;height:1000px;width:"+String(sitewidth)+"px;}";
  webpage += ".style1{text-align:center;font-size:16px;background-color:#FFE4B5;}";
  webpage += ".style2{padding:30px;text-align:left;font-size:16px;background-color:#F7F2Fd;width:auto;margin:0 auto;}";
  webpage += "</style>";
  webpage += "</head><body>";
  webpage += "<div class='navbar'>";

  webpage += " <a href='/homepage'>   Home</a>";
  webpage += " <a href='/programar'>  Programar</a>";
  webpage += " <a href='/verificar'>  Verificar</a>";
  webpage += " <a href='/nomes'>      Nomes</a>";
  webpage += " <a href='/telemetria'> Telemetria</a>";
  webpage += " <a href='/alertas'>    Alertas</a>";
  webpage += " <a href='/atualizar'>  Atualizar</a>";
  webpage += " <a href='/wifiweb'>    WiFi</a>";
  webpage += " <a href='/reiniciar'>  Reiniciar</a>";
  webpage += " <a href='/sobre'>      Sobre</a>";
  webpage += "</div>";
  webpage += "<br><title>"+sitetitle+"</title><br>";
}

void append_short_HTML_header() { // Needed because Google Charts appears to limit the amount of data that can be sent to its server
  webpage  = "";
  webpage += "<!DOCTYPE html><html><head>";
  webpage += "<style>";
  webpage += "body {width:"+String(sitewidth)+"px;margin:0 auto;font-family:arial;font-size:14px;text-align:center;color:blue;background-color:#F7F2Fd;}";
  webpage += "h1 {background-color:#d8d8d8;margin:16px 30px;}";
  webpage += "h3 {color:#9370DB;font-size:24px;width:auto;}";
  webpage += ".navbar{overflow:hidden;background-color:#558ED5;position:fixed;top:0;width:"+String(sitewidth)+"px;margin-left:30px;}";
  webpage += ".navbar a {float:left;display:block;color:yellow;text-align:center;padding:14px 16px;text-decoration: none;font-size:17px;}";
  webpage += ".main{padding:0px;margin:16px;height:1000px;width:"+String(sitewidth)+"px;}";
  webpage += ".style1{text-align:center;font-size:16px;background-color:#FFE4B5;}";
  webpage += ".style2{text-align:left;font-size:16px;background-color:#F7F2Fd;width:auto;margin:0 auto;}";
  // Note: You cannot include table, tr, td, or th styles if you want Google Charts to work!
  webpage += "</style></head>";
  webpage += "<body>";
  webpage += "<div class='navbar'>";
  webpage += " <a href='/homepage'>Home</a>";
  webpage += "</div>";
  webpage += "<title>"+sitetitle+"</title>";
  webpage += "<div class='main'>";
}

void append_HTML_footer() {
  webpage += "<footer><p>&copy; ";
  webpage += "ControlESP 2021<br>";
  webpage += "Projetando no Ap&ecirc;</p></footer>";
  webpage += "</div></body></html>";
}