/*
#include <Arduino.h>
 
#define LED_VERDE 2
#define LED_AMARELO 4
#define LED_VERMELHO 5

const int potPin = 34;
int potValue = 0;
 
void setup() {
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  Serial.begin(9600);
}
 
void loop() {
  potValue = analogRead(potPin);
  Serial.println(potValue);
  delay(500);
  if (potValue >= 500 && potValue <= 1000)
  {
    digitalWrite(LED_VERDE, HIGH);
    Serial.println("Led Verde acesso!");
  }
  else if (potValue >= 1001 && potValue <= 2000)
  {
    digitalWrite(LED_AMARELO, HIGH);
    Serial.println("Led Amarelo acesso!");
  }
  else if (potValue >= 2001)
  {
    digitalWrite(LED_VERMELHO, HIGH);
    Serial.println("Led Vermelho acesso!");
  }
  else
  {
    digitalWrite(LED_AMARELO, LOW);
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_VERMELHO, LOW);
  }
}
*/

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define TOPICO_SUBSCRIBE    "/TEF/device001/cmd"      //tópico MQTT de escuta
#define TOPICO_PUBLISH      "/TEF/device001/attrs"    //tópico MQTT de envio de informações para Broker
#define TOPICO_PUBLISH_2    "/TEF/device001/attrs/p"  //tópico MQTT de envio de informações para Broker
                                                      //IMPORTANTE: recomendamos fortemente alterar os nomes
                                                      //            desses tópicos. Caso contrário, há grandes
                                                      //            chances de você controlar e monitorar o ESP32
                                                      //            de outra pessoa.
#define ID_MQTT  "fiware_001"    //id mqtt (para identificação de sessão)
                                 //IMPORTANTE: este deve ser único no broker (ou seja, 
                                 //            se um client MQTT tentar entrar com o mesmo 
                                 //            id de outro já conectado ao broker, o broker 
                                 //            irá fechar a conexão de um deles).
                                 // o valor "n" precisa ser único!

#define LED_Verde 2
#define LED_Vermelho 5

// WIFI
const char* SSID = "POCO F5"; // SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = "senha1234"; // Senha da rede WI-FI que deseja se conectar

// MQTT
const char* BROKER_MQTT = "34.203.196.154"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT

int LED_AMARELO = 4;

WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient
char EstadoSaida = '0';  //variável que armazena o estado atual da saída

void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void InitOutput(void);

void setup() {
    pinMode(LED_Verde, OUTPUT);
    pinMode(LED_Vermelho, OUTPUT);
    InitOutput();
    initSerial();
    initWiFi();
    initMQTT();
    delay(5000);
    MQTT.publish(TOPICO_PUBLISH, "s|off");

}

void initSerial(){
    Serial.begin(9600);
}

void initWiFi(){
    delay(10);
    Serial.println("-------Conexao WI-FI-------");
    Serial.print("Conectando na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");

    reconectWiFi();
}

void initMQTT(){
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);
    MQTT.setCallback(mqtt_callback);
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
    
    Serial.print("- Mensagem recebida: ");
    Serial.println(msg);

    if (msg.equals("device001@on|"))
    {
        digitalWrite(LED_AMARELO, HIGH);
        EstadoSaida = '1';
    }
    if (msg.equals("device001@off|"))
    {
        digitalWrite(LED_AMARELO, LOW);
        EstadoSaida = '0';
    }
}

void reconnectMQTT() {
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
            digitalWrite(LED_Vermelho, HIGH);
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}

void reconectWiFi(){
    if (WiFi.status() == WL_CONNECTED)
        return;
    WiFi.begin(SSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
        Serial.print(".");
    }
    
    digitalWrite(LED_Verde, HIGH);
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}

void VerificaConexoesWiFIEMQTT(void){
    if (!MQTT.connected()) 
        reconnectMQTT(); 
     reconectWiFi(); 
}

void EnviaEstadoOutputMQTT(void){
    if (EstadoSaida == '1')
    {
      MQTT.publish(TOPICO_PUBLISH, "s|on");
      Serial.println("- Led Ligado");
    }
    if (EstadoSaida == '0')
    {
      MQTT.publish(TOPICO_PUBLISH, "s|off");
      Serial.println("- Led Desligado");
    }
    Serial.println("- Estado do LED onboard enviado ao broker!");
    delay(1000);
}

void InitOutput(void){
    pinMode(LED_AMARELO, OUTPUT);
    digitalWrite(LED_AMARELO, HIGH);
    
    boolean toggle = false;

    for(int i = 0; i <= 10; i++)
    {
        toggle = !toggle;
        digitalWrite(LED_AMARELO,toggle);
        delay(200);           
    }

    digitalWrite(LED_AMARELO, LOW);
}

void loop(){
    const int potPin = 34;
    int potValue = 0;

    char msgBuffer[4];
    //garante funcionamento das conexões WiFi e ao broker MQTT
    VerificaConexoesWiFIEMQTT();
 
    //envia o status de todos os outputs para o Broker no protocolo esperado
    EnviaEstadoOutputMQTT();

    potValue = analogRead(potPin);
    Serial.println(potValue);
    float value_potentiometer = map(potValue, 0, 4095, 0, 100); // Normalizar o valor da luminosidade entre 0% e 100%
    delay(500);
    dtostrf(value_potentiometer, 4, 2, msgBuffer);
    MQTT.publish(TOPICO_PUBLISH_2,msgBuffer);
    MQTT.loop();
}