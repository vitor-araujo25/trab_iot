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

#define SAMPLESIZE 100
#define TOLERANCE .01
#define INITDELAYMS 26
#define DURACAO_TRIG 10
 
 
// WIFI
const char* SSID = "hsNCE";
// const char* PASSWORD = "senha";
  
// MQTT
const char* BROKER_MQTT = "10.10.11.231"; //ip do broker
int BROKER_PORT = 1883; 
 
WiFiClient espClient; 
PubSubClient MQTT(espClient);

char sensorOutput = '0';  

uint16_t defaultMS;
uint16_t curMillis;
uint16_t millisTolerance;

void initWiFi();
void initMQTT();
void reconnectWiFi(); 
void checkConnections(void);
uint16_t readoutSensor();
uint16_t SetupDefaultDistance();
void initInput(void);
  
void setup() 
{
    initInput();
    initSerial();
    initWiFi();
    initMQTT();
}
 
void loop() 
{   
    
    uint16_t val;
    
    checkConnections();
    
    for( int i = 0; i < 10; i++)
      readoutSensor();

    curMillis = readoutSensor();

    if (curMillis > defaultMS) 
      val = curMillis-defaultMS;
    else
      val = defaultMS - curMillis;
        
    if(val < 1500) //valor específico para a sala onde testamos, deve ser calibrado
      sensorOutput = '0';
    else
      sensorOutput = '1';

    Serial.println(val);
 
    delay(INITDELAYMS);
 
    publishState();
 
    MQTT.loop();
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
            Serial.println("Falha ao reconectar ao broker.");
            Serial.println("Tentando novamente em 2s");
            delay(2000);
        }
    }
}
  
void reconnectWiFi() 
{
    if (WiFi.status() == WL_CONNECTED)
        return;

    // descomentar de acordo com a rede WiFi (aberta ou fechada)
    WiFi.begin(SSID);
    // WiFi.begin(SSID,PASSWORD);
     
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
 
//atualiza o broker MQTT com o status da sala lido pelo sensor a cada segundo.
void publishState(void)
{
    if (sensorOutput == '0')
      MQTT.publish(TOPICO_PUBLISH, "Sala vazia");
 
    if (sensorOutput == '1')
      MQTT.publish(TOPICO_PUBLISH, "Sala ocupada");
 
    Serial.println("Leitura do sensor enviada ao broker!");
    delay(1000);
}

void initInput(void)
{
    // D0 -- trigger
    // D1 -- ping
    pinMode(D0, OUTPUT);
    pinMode(D1, INPUT);
    
    Serial.begin(115200);
    
    // calcula a média de leituras do sensor (fase de normalização)
    Serial.print("Computing defaults...");
    
    defaultMS = SetupDefaultDistance();
    millisTolerance = (uint16_t)(defaultMS * TOLERANCE);
    
    Serial.println("Done.");         
}


uint16_t readoutSensor()
{
  // O sensor detecta pulsos a partir de 10us
  // Desativando o pulso do trigger por 3us para diminuir o ruído
  digitalWrite(D0, LOW);
  delayMicroseconds(3);

  digitalWrite(D0, HIGH);
  delayMicroseconds(DURACAO_TRIG);
  digitalWrite(D0, LOW);
 
  return pulseIn(D1, HIGH);
}

uint16_t SetupDefaultDistance()
{
  
  uint64_t averageMillis;
  uint32_t i;

  for(i=0, averageMillis = 0; i<SAMPLESIZE; i++) {
    averageMillis += readoutSensor();
    delay(INITDELAYMS);
  }
 
  // média de SAMPLESIZE leituras do sensor -- para normalizar o valor de output
  return (uint16_t)(averageMillis/SAMPLESIZE); 
}

