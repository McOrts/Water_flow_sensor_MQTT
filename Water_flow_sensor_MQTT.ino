#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include "settings.h"

volatile bool buttonPressed = false;

// Interrupt Service Routine (ISR)
ICACHE_RAM_ATTR void buttonInterrupt() {
  buttonPressed = true;
}

/* Configuración cliente WiFi */
WiFiClient espClient;

/* Configuración MQTT */
PubSubClient clientMqtt(espClient);
char msg[50];
String mqttcommand = String(14);

/* Configuración sensor de flujo */
int FlowSensorState = 0;
float CountFlow = 0;
float CountLitre = 0;
long CountStart = 0;

void setup() {
  Serial.begin(74880);
  digitalWrite(LED_BUILTIN, HIGH);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
    
  /* Iiniciar wifi */
  setup_wifi();
  clientMqtt.setServer(mqtt_server, mqtt_port);
  clientMqtt.setCallback(callback);
  delay(500);

  /* Iniciar sensor */
  pinMode(FlowSensorPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FlowSensorPin), buttonInterrupt, FALLING);  
}

void setup_wifi() {
  delay(500);

  // Comienza el proceso de conexión a la red WiFi
  Serial.println();
  Serial.print("[WIFI]Conectando a ");
  Serial.println(ssid);

  // Modo estación
  WiFi.mode(WIFI_STA);
  // Inicio WiFi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("[WIFI]WiFi conectada");
  Serial.print("[WIFI]IP: ");
  Serial.print(WiFi.localIP());
  Serial.println("");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("[MQTT]Mensaje recibido (");
  Serial.print(topic);
  Serial.print(") ");
  mqttcommand = "";
  for (int i = 0; i < length; i++) {
    mqttcommand += (char)payload[i];
  }
  Serial.print(mqttcommand);
  Serial.println();
  // Switch on the LED if an 1 was received as first character
  if (mqttcommand == "command") {
    Serial.println("don");
  } 
}

void reconnect() {
  Serial.print("[MQTT]Intentando conectar a servidor MQTT... ");
  // Bucle hasta conseguir conexión
  while (!clientMqtt.connected()) {
    Serial.print(".");
    // Intento de conexión
    if (clientMqtt.connect(mqtt_id)) { // Ojo, para más de un dispositivo cambiar el nombre para evitar conflicto
      Serial.println("");
      Serial.println("[MQTT]Conectado al servidor MQTT");
      // Once connected, publish an announcement...
      clientMqtt.publish(mqtt_sub_topic_healthcheck, "starting");
      // ... and subscribe
      clientMqtt.subscribe(mqtt_sub_topic_operation);
    } else {
      Serial.print("[MQTT]Error, rc=");
      Serial.print(clientMqtt.state());
      Serial.println("[MQTT]Intentando conexión en 5 segundos");
      delay(5000);
    }
  }
}

void loop() {
  if (!clientMqtt.connected()) {
    reconnect();
  }
  clientMqtt.loop();

  if (buttonPressed) {
    buttonPressed = false;
    CountFlow += 1;
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)  
    Serial.print(".");
  }

  long now_sensors = millis();
  if (now_sensors - CountStart > update_time_sensors) {
    digitalWrite(LED_BUILTIN, HIGH) ;    // turn the LED off by making the voltage LOW
    CountStart = now_sensors;
    Serial.println("Resume--------------------------------------");
    if ( CountFlow == 0) {
      clientMqtt.publish(mqtt_pub_topic_waterflow, "0");  
    } else {
      //Flow pulse characteristics (6.6*L/Min)
      CountLitre = CountFlow * 0.00225;
      snprintf (msg, 10, "%6.2f", CountLitre);
      clientMqtt.publish(mqtt_pub_topic_waterflow, msg);   
      Serial.print(msg);
      Serial.println(" liters");
    }
    Serial.println(CountFlow);
    Serial.println("");
    CountFlow = 0;
  }  
}
