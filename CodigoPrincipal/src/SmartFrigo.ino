//////////////////////////////////////////////////////////////////////////////
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
#include <Arduino.h>
#include <ESP_Mail_Client.h>

//////////////////////////////////////////////////////////////////////////////
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
#define BUTTON_PIN 19
#define DHTPIN 18 
#define SINAL 17

//////////////////////////////////////////////////////////////////////////////
uint32_t delayMS;

// Replace with your network credentials
const char* ssid1     = "SmartFrigo";
const char* password1 = "123456789";

String ssid,Password,Contato,Whataspp,Chave_API,SetPoint,Email;
bool FirstMessage = 0;
int flag = 0;
int t1=0,t2=0,dt=0,Tmonitoramento=0,Tled=0;

int contador=0;


unsigned long currentTime=0;
unsigned long previousTime=0,previousTime2,previousTime3,previousTime4;

/** The smtp host name e.g. smtp.gmail.com for GMail or smtp.office365.com for Outlook or smtp.mail.yahoo.com */
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

/* The sign in credentials */
#define AUTHOR_EMAIL "smartfrigo50@gmail.com"
#define AUTHOR_PASSWORD "xoqt ravu ivnh jawf";
//Senha email: "SmartFrigo_11_21_23"

/* Recipient's email*/
String RECIPIENT_EMAIL;

uint8_t Count_Email =0;

//////////////////////////////////////////////////////////////////////////////
Preferences preferences;

DHT_Unified dht(DHTPIN, DHTTYPE);

SMTPSession smtp;

//////////////////////////////////////////////////////////////////////////////
void smtpCallback(SMTP_Status status);
void Email_Sender(String msg);
void sendMessage(String message);
void ReadDataStored();
void  InitWifi();
void GetData();

//////////////////////////////////////////////////////////////////////////////
void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SINAL,OUTPUT);

  digitalWrite(SINAL,1);
  delay(2000);

  Serial.begin(115200);

  preferences.begin("Temperatura", false);
  ReadDataStored();

  delay(100);

  if(ssid != ""){
    InitWifi();
  }

  delay(500);
  dht.begin();
  
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
  }
  else{
  
    if((currentTime-previousTime3)>18000000){  //18000000
      previousTime3=currentTime;
      ESP.restart();
    }
    //delay(500);
    sensors_event_t event;

    if((currentTime-previousTime4)>1000){  
      previousTime4 = currentTime;
      dht.temperature().getEvent(&event);
      Serial.print(F("Temperature: "));
      Serial.print(event.temperature);
      Serial.println(F("graus Celsius"));
    }

    t2=millis();
  
    if(Reset==LOW){
    delay(3000);
      if(Reset==LOW){
        digitalWrite(SINAL,LOW);
        WiFi.disconnect(true,true);
        GetData();
        ESP.restart();
      }
    }
    else{  
      if (isnan(event.temperature)) {
        Serial.println(F("Error reading temperature!"));
        operacao=1;
      }
      else {
        
        dt=t2-Tmonitoramento;
        if(event.temperature > SetPoint.toFloat()){
          operacao=2;
          contador+=1;

          if(FirstMessage == 0){ //60000 
            sendMessage("A temperatura do freezer esta acima  do limite estabelecido. Temperatura atual: "+String(event.temperature)+" graus Celsius");
            String Mensagem= "A temperatura do freezer esta acima do limite estabelecido. Temperatura atual: "+String(event.temperature)+"graus Celsius";
            Email_Sender(Contato,"Notificação SmatFrigo",Mensagem);
            FirstMessage = 1;
          }

          if(dt>65000){  //300000
            sendMessage("A temperatura do freezer esta acima  do limite estabelecido. Temperatura atual: "+String(event.temperature)+"graus Celsius");
            Tmonitoramento=millis();
            Count_Email+=1;
            if(Count_Email == 5){
              Count_Email=0;
              String Mensagem= "A temperatura do freezer esta acima do limite estabelecido. Temperatura atual: "+String(event.temperature)+"graus Celsius";
              Email_Sender(Contato,"Notificação SmatFrigo",Mensagem);
            }
          }
          
        }
        else{
          FirstMessage = 0;
        }
      }
    }
  
    if(((currentTime-previousTime)>100) && (operacao == 1)){
      previousTime=currentTime;
      digitalWrite(SINAL,!digitalRead(SINAL));
    }
  
    if(((currentTime-previousTime)>500) && (operacao == 2)){
      previousTime=currentTime;
      digitalWrite(SINAL,!digitalRead(SINAL));
    }
  
    if(((currentTime-previousTime)>1000) && (operacao == 3)){
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
      clientAP.println("<label for=\"Email\">Email do contato:</label><br><input type=\"email\" id=\"Email\" name=\"Email\"><br><br>");
      clientAP.println("<input type=\"submit\" value=\"Submit\">");
      clientAP.println("</form></body></html>");
      
      
      String req = clientAP.readStringUntil('\r');
      if(req.indexOf("SSID")>=0){
        
        
        req        = req.substring(req.indexOf("SSID")+1,req.indexOf("HTTP")-1);
        ssid       = req.substring(req.indexOf("SID")+5,req.indexOf("&PASS"));
        Password   = req.substring(req.indexOf("&PASS")+11,req.indexOf("&User"));
        Contato    = req.substring(req.indexOf("&User")+6,req.indexOf("&PassMQTT"));
        Whataspp   = req.substring(req.indexOf("&PassMQTT")+10,req.indexOf("&MQTTSERVER"));
        Chave_API  = req.substring(req.indexOf("&MQTTSERVER")+12,req.indexOf("&Setpoint"));
        SetPoint   = req.substring(req.indexOf("&Setpoint")+10,req.indexOf("&Email"));
        Email      =req.substring(req.indexOf("&Email")+7);



        Whataspp="+55" + Whataspp;

//        tratativa Email 
        String Part_1_Email = Email.substring(0,Email.indexOf("%"));
        Email.remove(0,Email.indexOf("%40")+3);
        
        

        Email = Part_1_Email+"@"+Email;
        
        preferences.putString("ssid", ssid); 
  
        preferences.putString("Password", Password); 
  
        preferences.putString("Contato", Contato); 
  
        preferences.putString("Whataspp", Whataspp); 
  
        preferences.putString("Chave_API", Chave_API); 
  
        preferences.putString("SetPoint", SetPoint); 

        preferences.putString("Email", Email); 
        
        Serial.println(ssid);
        Serial.println(Password);
        Serial.println(Contato);
        Serial.println(Whataspp);
        Serial.println(Chave_API);
        Serial.println(SetPoint);
        Serial.println(Email);
        
        
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

    uint16_t wifi_cicle=0;
    WiFi.mode(WIFI_STA);
    delay(200);

    Serial.println("Conectarei a: " + String(ssid));

    WiFi.begin(ssid.c_str(), Password.c_str());

    while(WiFi.status() != WL_CONNECTED ) {
      digitalWrite(SINAL,!digitalRead(SINAL));
      delay(30);

      wifi_cicle++;

      if(digitalRead(BUTTON_PIN)==LOW){
        WiFi.disconnect(true,true);
        digitalWrite(SINAL,LOW);
        GetData();
        ESP.restart();
      }

      if(wifi_cicle>400)     //reinicia para refazer o begin e tentar conectar de novo
      {
        Serial.println("Reiniciando...");
        wifi_cicle = 0;
        digitalWrite(SINAL,LOW);
        ESP.restart();
      }
    }


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
  RECIPIENT_EMAIL=preferences.getString("Email", "");
  
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

void Email_Sender(String destinatario,String assunto,String msg){
  /*  Set the network reconnection option */
  MailClient.networkReconnect(true);

  /** Enable the debug via Serial port
   * 0 for no debugging
   * 1 for basic level debugging
   *
   * Debug port can be changed via ESP_MAIL_DEFAULT_DEBUG_PORT in ESP_Mail_FS.h
   */
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the Session_Config for user defined session credentials */
  Session_Config config;

  /* Set the session config */
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";

  /*
  Set the NTP config time
  For times east of the Prime Meridian use 0-12
  For times west of the Prime Meridian add 12 to the offset.
  Ex. American/Denver GMT would be -6. 6 + 12 = 18
  See https://en.wikipedia.org/wiki/Time_zone for a list of the GMT/UTC timezone offsets
  */
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 3;
  config.time.day_light_offset = 0;

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = F("SmartFrigo");
  message.sender.email = AUTHOR_EMAIL;
  message.subject = assunto;
  message.addRecipient(destinatario, RECIPIENT_EMAIL);
    


   
  //Send raw text message
  String textMsg = msg;
  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;


  /* Connect to the server */
  if (!smtp.connect(&config)){
    ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!smtp.isLoggedIn()){
    Serial.println("\nNot yet logged in.");
  }
  else{
    if (smtp.isAuthenticated())
      Serial.println("\nSuccessfully logged in.");
    else
      Serial.println("\nConnected with no Auth.");
  }

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());

}
/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status){
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()){
    // ESP_MAIL_PRINTF used in the examples is for format printing via debug Serial port
    // that works for all supported Arduino platform SDKs e.g. AVR, SAMD, ESP32 and ESP8266.
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);

      // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
      // your device time was synched with NTP server.
      // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
      // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)
      
      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}