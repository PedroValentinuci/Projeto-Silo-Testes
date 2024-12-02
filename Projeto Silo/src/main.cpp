#include <WiFi.h>
#include <PubSubClient.h> 
#include <WiFiManager.h>
 
#define TOPICO_SUBSCRIBE    "/TEF/device035/cmd"      
#define TOPICO_PUBLISH      "/TEF/device035/attrs"    
#define TOPICO_PUBLISH_2    "/TEF/device035/attrs/p"                                                       
#define ID_MQTT  "fiware_035"   

// Wifi                                
const char* SSID = "";
const char* PASSWORD = "123456789"; 
  
// MQTT
const char* BROKER_MQTT = "46.17.108.133"; 
int BROKER_PORT = 1883; 
int D4 = 2;

//Variáveis e objetos globais
WiFiClient espClient; 
PubSubClient MQTT(espClient); 
char EstadoSaida = '0';
bool apATIVO = false;
  
//Prototypes
void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void InitOutput(void);
 
//Programa
void setup() {
    InitOutput();
    initSerial();
    initWiFi();
    initMQTT();
    delay(5000);
    MQTT.publish(TOPICO_PUBLISH, "s|off");
}


void initSerial() {
    Serial.begin(9600);
}
 

void initWiFi() {
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    
    reconectWiFi();
}


void reconectWiFi() {
     WiFi.begin(SSID, PASSWORD);
     delay(5000);
    if(WiFi.status() == WL_CONNECTED){
       return;
    }
    else{
        Serial.println("Falha na conexão. Iniciando o modo de Ponto de Acesso.");
        WiFiManager wm;
        wm.setConfigPortalTimeout(0);
        if (!wm.autoConnect("ESP32_AP", "0000")){
            Serial.println("Ponto de Acesso ativo. Aguardando configuração de rede...");
        }
    }
    while (WiFi.status() != WL_CONNECTED) {
            delay(100);
            Serial.print(".");
        }

    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}

void initMQTT() {
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   
    MQTT.setCallback(mqtt_callback);            
}
  

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    String msg;
     
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) {
       char c = (char)payload[i];
       msg += c;
    }
    
    Serial.print("- Mensagem recebida: ");
    Serial.println(msg);
    
    if (msg.equals("device001@on|")){
        digitalWrite(D4, HIGH);
        EstadoSaida = '1';
    }
 
    if (msg.equals("device001@off|")){
        digitalWrite(D4, LOW);
        EstadoSaida = '0';
    }
}
  

void reconnectMQTT() {
    while (!MQTT.connected()) {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else{
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}
  

void VerificaConexoesWiFIEMQTT(void){
    if (!MQTT.connected()) 
        reconnectMQTT(); 
     
     reconectWiFi(); 
}
 

void EnviaEstadoOutputMQTT(void){
    if (EstadoSaida == '1'){
      MQTT.publish(TOPICO_PUBLISH, "s|on");
      Serial.println("- Led Ligado");
    }
    if (EstadoSaida == '0'){
      MQTT.publish(TOPICO_PUBLISH, "s|off");
      Serial.println("- Led Desligado");
    }
    Serial.println("- Estado do LED onboard enviado ao broker!");
    delay(1000);
}
 

void InitOutput(void){
    pinMode(D4, OUTPUT);
    digitalWrite(D4, HIGH);
    
    boolean toggle = false;

    for(int i = 0; i <= 10; i++){
        toggle = !toggle;
        digitalWrite(D4,toggle);
        delay(200);           
    }

    digitalWrite(D4, LOW);
}
 
 
//programa principal
void loop() {   
    const int SinalSensor = 34;
    char msgBuffer[4];
    
    VerificaConexoesWiFIEMQTT();
    EnviaEstadoOutputMQTT();

    int sensorValue = analogRead(SinalSensor);
    float value_potentiometer = map(sensorValue, 0, 4095, 0, 100);
    Serial.print("Value potentiometer: ");
    Serial.print(value_potentiometer);
    Serial.println("%");
    dtostrf(value_potentiometer, 4, 2, msgBuffer);
    MQTT.publish(TOPICO_PUBLISH_2,msgBuffer);

    MQTT.loop();
}
