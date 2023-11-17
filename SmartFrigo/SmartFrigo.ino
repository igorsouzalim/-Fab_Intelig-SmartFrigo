///////////////////////////////////////

#include <Preferences.h>
#include <ETH.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiSTA.h>
#include <WiFiType.h>
#include <WiFiUdp.h>
#include <UrlEncode.h>
#include <HTTPClient.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
///////////////////////////////////////

#define DHTTYPE    DHT22     // DHT 22 (AM2302)
#define BUTTON_PIN 19
#define DHTPIN 18 
#define SINAL 17
///////////////////////////////////////

uint32_t delayMS;

// Replace with your network credentials
const char* ssid1     = "SmartFrigo";
const char* password1 = "123456789";

String ssid,Password,Contato,Whataspp,Chave_API,SetPoint;

int flag = 0;
int t1=0,t2=0,dt=0,Tmonitoramento=0,Tled=0;

int contador=0;


unsigned long currentTime=0;
unsigned long previousTime=0,previousTime2,previousTime3;
///////////////////////////////////////

Preferences preferences;

DHT_Unified dht(DHTPIN, DHTTYPE);
///////////////////////////////////////

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SINAL,OUTPUT);
  
  delay(2500);

  digitalWrite(SINAL,1);
  delay(1500);

  Serial.begin(115200);
  
  
  if(ssid != ""){
    InitWifi();
  }

  preferences.begin("Temperatura", false);
  dht.begin();
  
  ReadDataStored();
  
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;

}

void loop() {
  bool Reset = digitalRead(BUTTON_PIN);
  uint8_t operacao=3;
  
  currentTime=millis();

  
  
  if(WiFi.status() != WL_CONNECTED){
    InitWifi();
    digitalWrite(SINAL,HIGH);
  }else{
  
    if((currentTime-previousTime3)>18000000){  //18000000
      previousTime3=currentTime;
      ESP.restart();
    }
  
    delay(100);
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    
    t2=millis();
  
    
  
    if(Reset==LOW){
      if((currentTime-previousTime2)>3000){  
        previousTime2=currentTime;
        WiFi.disconnect(true,true);
        GetData();
        ESP.restart();
      }
      
    }else{  
      if (isnan(event.temperature)) {
        Serial.println(F("Error reading temperature!"));
        
        operacao=1;
        
        dt=t2-Tmonitoramento;
        if(dt>60000){
          //sendMessage("Algo de errado aconteceu com o sensor, verifique");
          Tmonitoramento=millis();
        }
      }
      else {
        
        dt=t2-Tmonitoramento;
        if(event.temperature > SetPoint.toFloat()){
          operacao=2;
          contador+=1;
          if(dt>30000){
            sendMessage("Temperatura maior que o esperado para monitorar, "+String(event.temperature));
            Tmonitoramento=millis();
            
          }
          
        }
        
        Serial.print(F("Temperature: "));
        Serial.print(event.temperature);
        Serial.println(F("°C"));
      }
    }
  
    if(((currentTime-previousTime)>100) && (operacao ==1)){
      previousTime=currentTime;
      digitalWrite(SINAL,HIGH);
      delay(50);
      digitalWrite(SINAL,LOW);
    }
  
    if(((currentTime-previousTime)>400) && (operacao ==2)){
      previousTime=currentTime;
      digitalWrite(SINAL,HIGH);
      delay(45);
      digitalWrite(SINAL,LOW);
      delay(45);
      digitalWrite(SINAL,HIGH);
      delay(45);
      digitalWrite(SINAL,LOW);
      delay(45);
      digitalWrite(SINAL,HIGH);
      delay(100);
      digitalWrite(SINAL,LOW);
      delay(80);
    }
  
    if(((currentTime-previousTime)>1000) && (operacao ==3)){
      previousTime=currentTime;
      digitalWrite(SINAL,!digitalRead(SINAL));
    }
  }
}
///////////////////////////////////////
void GetData(){
  while(flag == 0){
    // Connect to Wi-Fi network with SSID and password
    Serial.print("Setting AP (Access Point)…");
    // Remove the password parameter, if you want the AP (Access Point) to be open
    
    IPAddress StaticIP(192,168,10,9); 
    IPAddress gateway(192,168,4,1);
    IPAddress subnet(255,255,255,0);
    
    WiFi.mode(WIFI_AP);
  
    WiFi.softAP(ssid1, password1,2,0);
    WiFi.config(StaticIP,gateway,subnet);
  
    
    WiFiServer server(80);
    server.begin();
  
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    
    WiFiClient clientAP = server.available();   // Listen for incoming clients
  
    while(1){
      delay(1);
      clientAP = server.available();
      if(clientAP){
          break;
      }
    }
    if(flag == 0){
      if(clientAP){
        Serial.println("New Client.");
        }
    
      // Display the HTML web page
      clientAP.println("<!DOCTYPE html><html>");
      clientAP.println("<html><head><title>Medidor de Qualidade</title>");
      clientAP.println("</head>");
      
      // Web Page Heading
      clientAP.println("<body><h2>Dados da Rede Local</h2>");
      clientAP.println("<form><label for=\"Rede Wifi\">Rede Wifi:</label><br>");
      clientAP.println("<input type=\"text\" id=\"SSID1\" name=\"SSID1\"><br><label for=\"PASSWORD1\">Senha:</label><br>");
      clientAP.println("<input type=\"text\" id=\"PASSWORD1\" name=\"PASSWORD1\"><br><br>");
      clientAP.println("<label for=\"User\">Nome do Contato:</label><br><input type=\"text\" id=\"User\" name=\"User\"><br><br>");
      clientAP.println("<label for=\"PassMQTT\">Numero do Whatsapp:</label><br><input type=\"text\" id=\"PassMQTT\" name=\"PassMQTT\"><br><br>");
      clientAP.println("<label for=\"MQTTSERVER\">Codigo dado pela API:</label><br><input type=\"text\" id=\"MQTTSERVER\" name=\"MQTTSERVER\"><br><br>");
      clientAP.println("<label for=\"Setpoint\">Temperatura a ser monitorada:</label><br><input type=\"text\" id=\"Setpoint\" name=\"Setpoint\"><br><br>");
      clientAP.println("<input type=\"submit\" value=\"Submit\">");
      clientAP.println("</form></body></html>");
      
      
      String req = clientAP.readStringUntil('\r');
      if(req.indexOf("SSID")>=0){
        
        
        req        = req.substring(req.indexOf("SSID")+1,req.indexOf("HTTP")-1);
        ssid         = req.substring(req.indexOf("SID")+5,req.indexOf("&PASS"));
        Password       = req.substring(req.indexOf("&PASS")+11,req.indexOf("&User"));
        Contato   = req.substring(req.indexOf("&User")+6,req.indexOf("&PassMQTT"));
        Whataspp   = req.substring(req.indexOf("&PassMQTT")+10,req.indexOf("&MQTTSERVER"));
        Chave_API = req.substring(req.indexOf("&MQTTSERVER")+12,req.indexOf("&Setpoint"));
        SetPoint   = req.substring(req.indexOf("&Setpoint")+10);
  
        Whataspp="+55" + Whataspp;
        
        preferences.putString("ssid", ssid); 
  
        preferences.putString("Password", Password); 
  
        preferences.putString("Contato", Contato); 
  
        preferences.putString("Whataspp", Whataspp); 
  
        preferences.putString("Chave_API", Chave_API); 
  
        preferences.putString("SetPoint", SetPoint); 
        
        Serial.println(ssid);
        Serial.println(Password);
        Serial.println(Contato);
        Serial.println(Whataspp);
        Serial.println(Chave_API);
        Serial.println(SetPoint);
        
        flag =1;
        clientAP.println("<meta http-equiv=\"refresh\" content=\"0.5\">");
      }
    }else{
      // Display the HTML web page
      clientAP.println("<!DOCTYPE html><html>");
      clientAP.println("<html><head><title>Configurando</title>");
      clientAP.println("</head>");
      
      // Web Page Heading
      clientAP.println("<body><h2>Obrigado pela entrada de dados</h2>");
  
      clientAP.println("<h3>Dentro de alguns instantes o monitoramento deve iniciar</h3></body></html>");
    }
    clientAP.flush();  
    server.close();
  }
  flag=0;
}

void  InitWifi() {
    uint8_t wifi_cicle=0;
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to: "); 
    Serial.print(ssid.c_str());
    WiFi.begin(ssid.c_str(), Password.c_str());


    while(WiFi.status() != WL_CONNECTED ) {
        digitalWrite(SINAL,!digitalRead(SINAL));
        delay(100);
        Serial.print('.');
        wifi_cicle+=1;
        
        Serial.print(".");
        if(digitalRead(BUTTON_PIN)==LOW){
            WiFi.disconnect(true,true);
            GetData();
            ESP.restart();
        }
    
        if(wifi_cicle>150)     //reinicia para refazer o begin e tentar conectar de novo
        {
          wifi_cicle = 0;
          for(int i =0;i<20;i++)
          {
            digitalWrite(SINAL,!digitalRead(SINAL));
            delay(50);
          }
          
          ESP.restart();
        }

    }
    Serial.println("");

    if(WiFi.status() != WL_CONNECTED) {
        Serial.println("Failed to connect");
        Serial.println("");
    }else{
        Serial.print("WiFi connected in: "); 
        Serial.print(millis());
        Serial.print(", IP address: "); 
        Serial.println(WiFi.localIP());
    
    }
    
}

void ReadDataStored(){

  ssid=preferences.getString("ssid", "");   
  Password=preferences.getString("Password", "");   
  Contato=preferences.getString("Contato", "");   
  Whataspp=preferences.getString("Whataspp", "");   
  Chave_API=preferences.getString("Chave_API", "");   
  SetPoint=preferences.getString("SetPoint", "");   
  
}

void sendMessage(String message){

  // Data to send with HTTP POST

      
      Serial.println(Whataspp);
      Serial.println(Chave_API);

  String url = "https://api.callmebot.com/whatsapp.php?phone=" + Whataspp + "&apikey=" + Chave_API + "&text=" + urlEncode(message);    
  HTTPClient http;
  http.begin(url);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200){
    Serial.print("Message sent successfully");
  }
  else{
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}
