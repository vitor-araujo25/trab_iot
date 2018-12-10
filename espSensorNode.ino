#include <ESP8266WiFi.h> 
#include <PubSubClient.h>
  
#define TOPICO_PUBLISH  "Sala01"   
                                             
#define ID_MQTT  "node01"  
                                
#define D0    16
#define D1    5
#define D2    4
#define D3    0
#define D4    2
#define D5    14
#define D6    12
#define D7    13
#define D8    15
#define D9    3
#define D10   1
 
 
// WIFI
const char* ssid = "labnet (The Lab)";
const char* password = "l4bn3t00";
  
// MQTT
const char* BROKER_MQTT = "IP_DO_RASPBERRY"; 
int BROKER_PORT = 1883; 
 
WiFiClient espClient; 
PubSubClient MQTT(espClient);

//TODO:verificar o tipo de output do sensor de presença 
char sensorOutput = '0';  

void initSerial();
void initWiFi();
void initMQTT();
void reconnectWiFi(); 
void checkConnections(void);

// void InitInput(void);
 
void setup() 
{
    // InitInput();
    initSerial();
    initWiFi();
    initMQTT();
}
  
void initSerial() 
{
    Serial.begin(115200);
}
 
void initWiFi() 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
     
    reconnectWiFi();
}
  
void initMQTT() 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);  
}

void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("Conectando-se ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Tentando novamente em 2s");
            delay(2000);
        }
    }
}
  
void reconnectWiFi() 
{
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(SSID, PASSWORD);
     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}
 
void checkConnections(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); 
     
    reconnectWiFi(); 
}
 
//atualiza o broker MQTT com o status da sala lido pelo sensor a cada 60 segundos.
void publishState(void)
{
    if (sensorOutput == '0')
      MQTT.publish(TOPICO_PUBLISH, "Sala vazia");
 
    if (sensorOutput == '1')
      MQTT.publish(TOPICO_PUBLISH, "Sala ocupada");
 
    Serial.println("Leitura do sensor enviada ao broker!");
    delay(60000);
}

//TODO:verificar o tipo de output do sensor de presença 

// void InitInput(void)
// {
//     pinMode();         
// }
 
void loop() 
{   
    
    checkConnections();

    //lê input da serial e usa como se fosse dado do sensor (para teste)
    while(Serial.available()){
        sensorOutput = Serial.readString();
    }
 
    publishState();
 
    MQTT.loop();
}