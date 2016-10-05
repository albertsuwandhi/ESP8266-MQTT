#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


// Update these with values suitable for your board
#define LEDPin 2
#define ButtonPin 12
#define LED_ON LOW
#define LED_OFF HIGH

const char* ssid = "SSID";
const char* password = "PASSWORD";
const char* mqtt_server = "MQTT";
const char* server_username = "USERID";
const char* server_password = "PASS";
const char* data_topic = "DATATOPIC";
const char* control_topic = "CTRLTOPIC";

WiFiClient espClient;
PubSubClient client(espClient);

int value;
String ledString = "";
//long lastMsg = 0;
//char msg[50];
//int value = 0;
long currentTime = 0, lastTime = 0;
bool needtoPublish = false, buttonState = false;

void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void intHandler();

void setup() {
  pinMode(LEDPin, OUTPUT); // Initialize the LEDPin pin as an output
  pinMode(ButtonPin, INPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  attachInterrupt(ButtonPin,intHandler,RISING);
}


void intHandler()
{
  if((currentTime-lastTime)>500){
      needtoPublish = true;
      if (buttonState){
        ledString = "{\"webLED\":1}";
        buttonState = false;
        Serial.println("Publish webLED : 1");
      }
      else{
        ledString = "{\"webLED\":0}";
        buttonState = true;
        Serial.println("Publish webLED : 0");
      }
  lastTime=millis();
}
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  char json [100];
  //String json;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    json[i]= (char)payload[i];
  }

  //String json = String((char*)payload).substring.(0,length);
  Serial.println();
//----------------------------------------------------------------------
   // Root of the object tree.
  //
  // It's a reference to the JsonObject, the actual bytes are inside the
  // JsonBuffer with all the other nodes of the object tree.
  // Memory is freed when jsonBuffer goes out of scope.

  StaticJsonBuffer<100> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);

  // Test if parsing succeeds.
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  else{
    Serial.println("Parsing Success");
  }

  // Fetch values.
  //
  // Most of the time, you can rely on the implicit casts.
  // In other case, you can do root["time"].as<long>();
  //const char* sensor = root["sensor"];
  //long time = root["time"];
  //double latitude = root["data"][0];
  //double longitude = root["data"][1];
  if (root.containsKey("LED")){
  value = root["LED"];
  Serial.print("Value is : ");
  Serial.println(value);
  }
  else {
  Serial.println("Invalid JSON");
  }
  // Print values.
  //Serial.println(sensor);
  //Serial.println(time);
  //Serial.println(latitude, 6);
  //Serial.println(longitude, 6)



  // Switch on the LED if an 1 was received as first character
  //{"LED":1}
  //if ((char)payload[7] == '1') {
  if (value == 1) {
    digitalWrite(LEDPin, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(LEDPin, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client",server_username,server_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe(control_topic);
    } else {
      Serial.print("failed, retry count =");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 3 seconds before retrying
      delay(3000);
    }
  }
}
void loop() {
  currentTime = millis();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (needtoPublish){
     needtoPublish=false;
     client.publish(data_topic, ledString.c_str());
  }

  //client.publish(data_topic, ledString.c_str());
  //long now = millis();
  //if (now - lastMsg > 2000) {
  //  lastMsg = now;
  //  ++value;
  //  snprintf (msg, 75, "hello world #%ld", value);
  //  Serial.print("Publish message: ");
  //  Serial.println(msg);
  //  client.publish("outTopic", msg);
  }
