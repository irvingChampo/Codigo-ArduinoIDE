#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Pines de los sensores
#define PIR_PIN 14        // Pin del sensor PIR
#define TRIG_PIN 13       // Pin Trig del sensor ultrasónico
#define ECHO_PIN 12       // Pin Echo del sensor ultrasónico
#define DHT_PIN 4         // Pin del sensor DHT11
#define DHTTYPE DHT11     // Tipo de sensor DHT

// Configuración del WiFi
const char* ssid = "Honor";
const char* password = "12345678";

// Configuración de MQTT
const char* mqtt_server = "44.197.73.155";  // Dirección IP de tu computadora
const int mqtt_port = 1883;               // Puerto MQTT por defecto
const char* mqtt_user = "samuel";          // Usuario MQTT
const char* mqtt_password = "samuel2004";
const char* mqtt_topic = "esp32.mqtt";     // Topic para enviar datos

WiFiClient espClient;
PubSubClient client(espClient);

DHT dht(DHT_PIN, DHTTYPE);
long duration;
int distance;
String pirStatus;

void setup() {
  Serial.begin(9600);

  // Inicializa los pines de los sensores
  pinMode(PIR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  dht.begin();

  // Conéctate a la red WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Configura el cliente MQTT
  client.setServer(mqtt_server, mqtt_port);

  // Espera para que todo se inicialice
  delay(2000);
  Serial.println("Inicialización completada");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("Conectado");
    } else {
      Serial.print("Error, rc=");
      Serial.print(client.state());
      Serial.println(" Intenta de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

void leerPIR() {
  // Lectura del sensor PIR
  int pirState = digitalRead(PIR_PIN);
  pirStatus = (pirState == HIGH) ? "Movimiento detectado" : "Sin movimiento";
  Serial.println(pirStatus);
}

void leerUltrasonico() {
  // Medición de distancia con el sensor ultrasónico
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;

  Serial.print("Distancia: ");
  Serial.print(distance);
  Serial.println(" cm");
}

void leerDHT11() {
  // Lectura del sensor DHT11
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Error al leer el sensor DHT11");
  } else {
    // Muestra los valores en el monitor serial
    Serial.print("Humedad: ");
    Serial.print(humidity);
    Serial.print(" %\t");
    Serial.print("Temperatura: ");
    Serial.print(temperature);
    Serial.println(" *C");
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  leerPIR();
  leerUltrasonico();
  leerDHT11();

  // Publica los datos al broker MQTT
  String payload = "PIR: " + pirStatus + ", Distancia: " + String(distance) + " cm, Temp: " + String(dht.readTemperature()) + " C, Hum: " + String(dht.readHumidity()) + " %";
  client.publish(mqtt_topic, payload.c_str());

  delay(2000); // Esperar 2 segundos entre lecturas
}