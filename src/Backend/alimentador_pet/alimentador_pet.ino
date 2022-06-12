#include <Arduino.h> //Bibliotecas
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Stepper.h>
#include <Wire.h>
#include "RTClib.h"
#include "index_html.h"
#include <NTPClient.h>
#include <WiFiUdp.h>  

AsyncWebServer server(80);

// Nome da rede wifi e senha
const char* ssid = "coloque aqui o nome da red";
const char* password = "e a senha dela";

//variaveis para puxar informação do html e transforma ela em int para poder ser comparada com a hora
const char* PARAM_Hour = "inputHour";
const char* PARAM_Minute = "inputMinute";

int hourClient;
int minuteClient;

String minuteC;
String hourC;

//Pinos para usar o Stepper D1 D5 D2 D6
const int stepsPerRevolution = 200;
Stepper myStepper(stepsPerRevolution, D1, D5, D2, D6);

//Define NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org"); //Site para puxar a hora

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  myStepper.setSpeed(60); //Define a velocidade do motor
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Erro na conexão");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin(); //inicializa o NTP para pegar o tempo
  timeClient.setTimeOffset(-10800); //ajusta para o horario do brasil GMT -3, -3*60*60

  // Manda a pagina para a Web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Manda o pedido para o ESP
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // Pegar o input da hora
    if (request->hasParam(PARAM_Hour)) {
      hourC = request->getParam(PARAM_Hour)->value();
      
    }
    // Pegar o input do Minuto
    else if (request->hasParam(PARAM_Minute)) {
      minuteC = request->getParam(PARAM_Minute)->value();
      }
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  timeClient.update(); //
  hourClient = hourC.toInt(); // hora pega do site
  minuteClient = minuteC.toInt(); // minuto 
  int currentHour = timeClient.getHours(); //hora pega do servidor
  int currentMinute = timeClient.getMinutes(); //minuto
  int currentSecond = timeClient.getSeconds(); //segundos
  Serial.print(hourClient); //essa linhas seguintes servem para mostra o horario do servidor e horaio inserido pelo usario
  Serial.print(":");
  Serial.print(minuteClient);
  Serial.println();
  Serial.println(currentHour);
  Serial.print(":");
  Serial.println(currentMinute);
  Serial.print(":");
  Serial.println(currentSecond);
  Serial.println();
  delay(1000);
  if(hourClient == currentHour && minuteClient == currentMinute && currentSecond == 0){ //if e for para quando a hora e o minuto forem igual aos inseridos no site
    for(int i = 0; i<10; i++){
    myStepper.step(stepsPerRevolution); //roda o motor
    yield(); //usado para nao deixar o watch dog resetar o eps
    }
    }
}
