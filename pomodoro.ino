#include <WiFi.h>
#include <PubSubClient.h>

int redPin_g = 19;
int greenPin_g = 17;
int bluePin_g = 18;

int redPin_i = 21;
int greenPin_i = 2;
int bluePin_i = 4;

#define SERVO_PIN 16

const int buttonPin = 15;
int buttonState = 0;

int channel = 1;
int hz = 50;
int depth = 16;

const char* ssid = "SSID";
const char* password = "password";
const char* server = "192.168.1.105";
const char* topic = "/pomodoro/+";
const char* clientName = "com.gonzalo123.esp32";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void wifiConnect() {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("*");
  }

  Serial.print("WiFi connected: ");
  Serial.println(WiFi.localIP());
}

void mqttReConnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientName)) {
      Serial.println("connected");
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);

  String data;
  for (int i = 0; i < length; i++) {
    data += (char)payload[i];
  }

  int value = data.toInt();

  if (strcmp(topic, "/pomodoro/gonzalo") == 0) {
    Serial.print("[gonzalo]");
    switch (value) {
      case 1:
        ledcWrite(1, 3400);
        setColor_g(0, 255, 0);
        break;
      case 2:
        setColor_g(255, 0, 0);
        break;
      case 3:
        ledcWrite(1, 6400);
        setColor_g(0, 0, 255);
        break;
    }
  } else {
    Serial.print("[ibai]");
    switch (value) {
      case 1:
        setColor_i(0, 255, 0);
        break;
      case 2:
        setColor_i(255, 0, 0);
        break;
      case 3:
        setColor_i(0, 0, 255);  // green
        break;
    }
  }

  Serial.print("] value:");
  Serial.println(data);
}

void setup()
{
  Serial.begin(115200);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(redPin_g, OUTPUT);
  pinMode(greenPin_g, OUTPUT);
  pinMode(bluePin_g, OUTPUT);

  pinMode(redPin_i, OUTPUT);
  pinMode(greenPin_i, OUTPUT);
  pinMode(bluePin_i, OUTPUT);

  ledcSetup(channel, hz, depth);
  ledcAttachPin(SERVO_PIN, channel);
  wifiConnect();
  client.setServer(server, 1883);
  client.setCallback(callback);

  delay(1500);
}

void mqttEmit(String topic, String value)
{
  client.publish((char*) topic.c_str(), (char*) value.c_str());
}

void loop()
{
  if (!client.connected()) {
    mqttReConnect();
  }

  client.loop();

  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    mqttEmit("/start/gonzalo", (String) "3");
  }

  delay(200);
}

void setColor_i(int red, int green, int blue)
{
  digitalWrite(redPin_i, red);
  digitalWrite(greenPin_i, green);
  digitalWrite(bluePin_i, blue);
}

void setColor_g(int red, int green, int blue)
{
  digitalWrite(redPin_g, red);
  digitalWrite(greenPin_g, green);
  digitalWrite(bluePin_g, blue);
}