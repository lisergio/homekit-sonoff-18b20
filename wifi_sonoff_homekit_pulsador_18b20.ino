#include <DallasTemperature.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>  

IPAddress server(192, 168, 0, 56); // IP de la raspberry Pi
const char* host = "Sonoff1estado18b20"; // nombre del entorno

#define ONE_WIRE_BUS 14 // GPIO 14 del ESP8266
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int rele = 12;
int led = 13;
int pulsador = 0;
boolean estado_pulsador ;
boolean estado_luz = 0;
int temporizador=0;
float temp;


int ssid_length;
int passw_length;
String ssid; 
String passw;
const char* ssid2;
const char* passw2;

#define BUFFER_SIZE 100

WiFiClient wclient;
PubSubClient client(wclient, server);

void callback(const MQTT::Publish& pub) {
  Serial.println (pub.payload_string());
    if(pub.payload_string() == "on")
    {
      digitalWrite(rele, HIGH); // en caso de que el modulo rele funcione al reves, cambiarl LOW por HIGH
      digitalWrite(led, LOW);
      estado_luz=1;
      Serial.println("Switch On");         
    }
    if(pub.payload_string() == "off")
    {
      digitalWrite(rele, LOW); // en caso de que el modulo rele funcione al reves, cambiarl HIGH por LOW
      digitalWrite(led, HIGH);
      estado_luz=0;
      Serial.println("Switch Off");
    }

    if (estado_luz != EEPROM.read(0)) EEPROM.write(0,estado_luz);
    EEPROM.commit();
}

void setup() 
{
  pinMode(rele,OUTPUT);
  pinMode(led,OUTPUT);
  pinMode(pulsador,INPUT);
  pinMode(14,INPUT);
  delay(1000);
  digitalWrite(led, LOW);
  Serial.begin(115200);
  EEPROM.begin(512);
  delay(10);
  Serial.println();
  Serial.println();
  client.set_callback(callback);


    if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid2, passw2); 
    if (WiFi.waitForConnectResult() != WL_CONNECTED){
      Serial.println("WiFi not connected");
      WiFiManager wifiManager;  
      if (!wifiManager.startConfigPortal("Sonoff-18b20")) { // SSID 
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        ESP.reset();
        delay(5000);
        }
      Serial.print("connected to ");
      ssid=WiFi.SSID();
      passw=WiFi.psk();
      Serial.println(ssid);
      Serial.println(passw);
      ssid_length=(ssid.length());
      passw_length=(passw.length());
      Serial.println(ssid_length);
      Serial.println(passw_length);
      Serial.println("Guardando el SSID en la EEPROM");
 
      for (int i=0; i<WiFi.SSID().length(); ++i){
        EEPROM.write(i+1,WiFi.SSID()[i]);
        }
      Serial.println("Guardando el PASSWORD en la EEPROM");
  
      for (int i=0; i<WiFi.psk().length(); ++i){
       EEPROM.write(50+i,WiFi.psk()[i]);
       }
  
      EEPROM.write(100, WiFi.SSID().length());
      EEPROM.write(101,WiFi.psk().length());
      EEPROM.commit();
      delay(2000);
    }
    else
   {
      Serial.println("WiFi connected");                       
   }
  } 
  
  estado_luz=EEPROM.read(0);
  digitalWrite(rele,EEPROM.read(0));
  digitalWrite(led,!EEPROM.read(0));
  delay(200);
}

void loop() {

    
  estado_pulsador=digitalRead(pulsador);
  if (estado_pulsador==LOW)
  {
    if (estado_luz==0)
    {
      estado_luz=1;
      client.publish("Sonoff1estado18b20","SwitchedOn");
      Serial.println("Switch On");
      digitalWrite(rele,HIGH);  // en caso de que el modulo rele funcione al reves, cambiarl HIGH por LOW
      digitalWrite(led, LOW);     
    }
    else
    {
      estado_luz=0; 
      client.publish("Sonoff1estado18b20","SwitchedOff");
      Serial.println("Switch Off");
      digitalWrite(rele,LOW); // en caso de que el modulo rele funcione al reves, cambiarl LOW por HIGH
      digitalWrite(led, HIGH);          
    }

    delay(1000);
    EEPROM.write(0,estado_luz);    
  }

    if (WiFi.status() == WL_CONNECTED) {
    if (temporizador >= 5000) 
      {
      comprobar_temperatura();
      if (client.connected())
      {
        Serial.println(String(temp)+"°")  ;
        client.publish("Temperatura18b20",String(temp));
      }
      temporizador=0;    
      }
 
    if (!client.connected()) {
      if (client.connect("ESP8266: Sonoff1estado18b20")) {
        client.publish("outTopic",(String)"hello world, I'm "+host);
        client.subscribe(host+(String)"/#");
      }
    }
    if (client.connected())
      {
      }
      client.loop();
  }
  delay(1);
  temporizador++;
}

void comprobar_temperatura()
{
  sensors.requestTemperatures();
  temp=sensors.getTempCByIndex(0);
  Serial.print("La temperatura es de : ");
  Serial.println(temp);
  //temp = temp - 2.00; // resta o suma aqui la diferencia de temperatura con otro tipo de termometro o comenta la linea si no hay que compensar nada...
}

