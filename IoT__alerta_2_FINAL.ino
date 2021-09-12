#include "Arduino.h"
#include <EMailSender.h>
#include <ESP8266WiFi.h>
#include <ThingerESP8266.h>
#define USERNAME "ISTEA"
#define DEVICE_ID "NodeMCU"
#define DEVICE_CREDENTIAL "qipFB8O@nAni66wd"
#define boton_2 15//D8// Pin de señal de entrada del pulsador
#define botton A0// Pin de señal de entrada del encendido/apagado desde la IoT
const int receiverPin = 5; // D1/// Pin de señal de entrada del receptor / detector (el módulo usado solo devuelve un estado digital)    
const int buzzerPin =  4; // D2///  Pin de señal de salida del buzzer que indica cuando el sistema se dispara o detecta la presencia de intruso   
const int laserPin =  14; // D5/// Pin de señal de salida del módulo láser / puntero láser   
const int rele_1 =  12; // D6// Pin de señal de salida del rele_1
//const int rele_2 =  13; // D7// Pin de señal de salida del rele_2
ThingerESP8266 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);
int receiverState = 0; 
int estado = 0;      
int contador = 0;
int alarma = 0;
int i;
int led_pulsador = 2; //D4/// // Pin de señal de salida del led que indica el encendido/apagado del sistema 
//const int LEDPin =  2;  //D4/// Pin de señal de salida del led que indica cuando el sistema se dispara o detecta la presencia de intruso   

bool cambio=false;
int pulsos_a = 0;
boolean anterior_a;
unsigned long tiempo1 = 0;
unsigned long tiempo2 = 0;

    uint8_t connection_state = 0;
    uint16_t reconnect_interval = 10000;
    EMailSender emailSend("isteaesmeraldas@gmail.com", "ISTEA2021");

uint8_t WiFiConnect(const char* nSSID = nullptr, const char* nPassword = nullptr)
{
    static uint16_t attempt = 0;
    Serial.print("Connecting to ");
    if(nSSID) {
        WiFi.begin(nSSID, nPassword);
        Serial.println(nSSID);
    }

    uint8_t i = 0;
    while(WiFi.status()!= WL_CONNECTED && i++ < 50)
    {
        delay(200);
        Serial.print(".");
    }
    ++attempt;
    Serial.println("");
    if(i == 51) {
        Serial.print("Connection: TIMEOUT on attempt: ");
        Serial.println(attempt);
        if(attempt % 2 == 0)
            Serial.println("Check if access point available or SSID and Password\r\n");
        return false;
    }
    Serial.println("Connection: ESTABLISHED");
    Serial.print("Got IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}
void Awaits()
{
    uint32_t ts = millis();
    while(!connection_state)
    {
        delay(50);
        if(millis() > (ts + reconnect_interval) && !connection_state){
            connection_state = WiFiConnect();
            ts = millis();
        }
    }
}
void setup()
{
  Serial.begin(115200);
  pinMode(laserPin, OUTPUT); // Se establece el pin láser en modo de salida     
  //pinMode(LEDPin, OUTPUT);  // Se establece el pin LED en modo de salida    
  pinMode(buzzerPin, OUTPUT);  // Se establece el pin buzzer en modo de salida 
  pinMode(rele_1, OUTPUT); // Se establece el pin del rele_1 en modo de salida
  //pinMode(rele_2, OUTPUT); // Se establece el pin del rele_2 en modo de salida, en caso de averiarse el rele_1   
  pinMode(receiverPin, INPUT); // Se establece el pin receptor en modo de entrada, el módulo del receptor láser envía una señal ALTA cuando el láser lo ilumina. 
  pinMode(botton, INPUT); // Se establece el pin de encendido/apagado desde la IoT en modo de salida
  attachInterrupt(digitalPinToInterrupt(boton_2), boton, CHANGE);
  pinMode(led_pulsador,OUTPUT); // Se establece el pin de encendido/apagado del sistema en modo de salida 
  pinMode(boton_2, INPUT); // Se establece el pin del pulsador en modo de entrada
  anterior_a = digitalRead(boton_2);
  tiempo1 = millis();
  const char* ssid = "STHERSIT@";
  const char* password = "Mijhairstalin1993@";
  connection_state = WiFiConnect(ssid, password);
  if(!connection_state)  // if not connected to WIFI
  Awaits();          // constantly trying to connec
  alarma=1;
  digitalWrite(laserPin, LOW); // Laser apagado 
  digitalWrite(buzzerPin, LOW); // Buzzer apagado
  digitalWrite(rele_1, HIGH); // Led apagado
  cambio=false;
  //thing.add_wifi(SSID, SSID_PASSWORD);
  thing["botton"] << digitalPin(botton);
  thing["led_pulsador"] << digitalPin(led_pulsador);
  thing["botton"]<< [](pson& activated){
    bool on = activated;
    if(activated){
      Serial.println("Sistema encendido remotamente");
      pulsos_a = 1;
      }
      else{
        pulsos_a = 2;
      }
      };
     }
void loop()
{   
  Serial.println(alarma);  
  if(pulsos_a==1){
    digitalWrite(laserPin, HIGH);
    Serial.println("sistema encendido"); 
    digitalWrite(led_pulsador, HIGH); 
    if (cambio==false){      
      receiverState = digitalRead(receiverPin);// leer el estado del valor de los receptores: 
      if (receiverState == HIGH) {    // si lo es, reciverState es HIGH: 
        digitalWrite(laserPin, HIGH); //Laser encendido 
        digitalWrite(rele_1, !HIGH); //Rele_1 apagado
        //digitalWrite(rele_2, LOW); //Rele_2 apagado
        digitalWrite(buzzerPin, LOW); //Buzzer apagado
        //digitalWrite(LEDPin, LOW); //Led apagado   
        Serial.println("alarma desactivada");  
      } 
      else { // Activa la alarma y sonara:
        thing.stop();
        digitalWrite(buzzerPin, HIGH);//Se enciende el buzzer
        //digitalWrite(LEDPin, HIGH); //Se enciende el led
        digitalWrite(rele_1, !LOW); //Se enciende el Rele_1 
        //digitalWrite(rele_2, HIGH); //Se enciende el Rele_2 
        digitalWrite(laserPin, HIGH);//El laser permanece encendido
        Serial.println("enviando mensaje");
        enviar_email();//Se envia el mensaje al correo electronico del administrador
        Serial.println("mensaje enviado"); 
        alarma=1; 
        thing.handle();
         tiempo2 = millis();
      if(tiempo2 > (tiempo1+1000)){  //Si ha pasado 1 segundo ejecuta el IF
          tiempo1 = millis(); //Actualiza el tiempo actual
       }
      }
    }
  }
  if (alarma == 1){
    cambio=true;
    digitalWrite(buzzerPin, HIGH);//Se enciende el buzzer
    //digitalWrite(LEDPin, HIGH); //Se enciende el led
    digitalWrite(rele_1, !LOW); //Se enciende el Rele_1 
        //digitalWrite(rele_2, HIGH); //Se enciende el Rele_2 
    digitalWrite(laserPin, HIGH);//El laser permanece encendido
   }
  if (alarma == 0){
    cambio=false;
    //digitalWrite(laserPin, HIGH);//El laser permanece encendido
    digitalWrite(rele_1, !HIGH); //Se apaga el Rele_1 
    //digitalWrite(rele_2, LOW); //Se apaga el el Rele_2 
    digitalWrite(buzzerPin, LOW); //Se apaga el buzzer
    //digitalWrite(LEDPin, LOW); //Se apaga el led 
     }    
  if(pulsos_a>=2)
  {
    alarma=1;
    Serial.println("Sistema apagado");
    digitalWrite(led_pulsador, LOW);
    pulsos_a=0;
  }
  thing.handle();
}

ICACHE_RAM_ATTR void boton() {
  //delay(200);
 boolean estado_a = digitalRead(boton_2);
  if (anterior_a != estado_a) {
    if (estado_a == HIGH) {  //flanco descendente pull-up
      tiempo2 = millis();
      if(tiempo2 > (tiempo1+200)){  //Si ha pasado 1 segundo ejecuta el IF
          tiempo1 = millis(); //Actualiza el tiempo actual
          pulsos_a++;
          Serial.println(pulsos_a);
      }
    }
  }
}

void enviar_email(){
    EMailSender::EMailMessage message;
    message.subject = "URGENTE ALARMA ACTIVADA";
    message.message = "Alerta la seguridad del Instituto ha sido vulnerada";
    EMailSender::Response resp = emailSend.send("isteaesmeraldas@gmail.com",message);
    Serial.println("Sending status: ");
    Serial.println(resp.status);
    Serial.println(resp.code);
    Serial.println(resp.desc);
    
}
