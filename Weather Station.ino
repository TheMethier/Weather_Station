#include <WiFi.h> // Include WIFi Library for ESP32
#include <WebServer.h> // Include WebSwever Library for ESP32
#include <ArduinoJson.h> // Include ArduinoJson Library
#include <WebSocketsServer.h>  // Include Websocket Library
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <DHT11.h>
#include<DallasTemperature.h>
const char* ssid = "SSID";
const char* password = "PASSWORD";
unsigned long previousTime = 0;
const long delayTime = 1000;
String web="<!DOCTYPE html><html><head> <meta charset='UTF-8' /> <title>title</title></head><body> <div> <h1> Temperatura: </h1><h1 id='temp'></h1> <h1> Wilgotność: </h1><h1 id='hum'></h1> </div></body><script> var Socket; var Temperature=document.getElementById('temp'); var Humidity=document.getElementById('hum'); function getDataFromServer(event) { var data=JSON.parse(event.data); Temperature.textContent=data['temp']; Humidity.textContent=data['hum']; console.log(data); } function socketInit() { Socket=new WebSocket('ws://'+window.location.hostname+':81/'); Socket.onmessage=((event)=>{ getDataFromServer(event); }); } window.onload=function(event){ socketInit(); }</script></html>";
String jsonStr="";
float temp=0;
float hum=0;
OneWire oneWire(13);
DallasTemperature mierz1(&oneWire);
DHT11 mierz(27);
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
void setup() {
  Serial.begin(9600);
  Serial.print("Connecting to:");
  Serial.println(ssid);
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected.");
  Serial.println("IP address: ");
  Serial.print(WiFi.localIP());
  server.on("/",[](){server.send(200,"text\html",web);});
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketE);

}
void webSocketE(byte num, WStype_t type, uint8_t * payload, size_t length) 
{
  switch(type)
  {
    case WStype_DISCONNECTED:
      Serial.println("Disconnected");
    break;
    case WStype_CONNECTED:
      Serial.println("Connected");
      sendDataToClient();    
    break;
  }
}
void getReadingsFromSensors()
{
mierz1.requestTemperatures();
float h=float(mierz.readHumidity());
if(h!=-1)
{
  temp=mierz1.getTempCByIndex(0);
  hum=h;
}
}
void sendDataToClient()
{
  StaticJsonDocument<500> doc;
  JsonObject obj=doc.to<JsonObject>();
  obj["temp"]=temp;
  obj["hum"]=hum;
  serializeJson(doc,jsonStr);
  webSocket.broadcastTXT(jsonStr);
  jsonStr="";
}
void loop() {
  server.handleClient();
  webSocket.loop();
  unsigned long currentTime=millis();
  if(currentTime-previousTime>=delayTime)
  {
    getReadingsFromSensors();
    sendDataToClient();
  }
}
