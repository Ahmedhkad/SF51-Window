#include <Arduino.h>
#include "settings.h" //GPIO defines, NodeMCU good-pin table
#include "secret.h"   //Wifi and mqtt server info

#include <IRsend.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <ArduinoOTA.h>

const char *ssid = ssidWifi;
const char *password = passWifi;
const char *mqtt_server = mqttURL;

IPAddress local_IP(192, 168, 1, 162);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   // optional
IPAddress secondaryDNS(8, 8, 4, 4); // optional

const char *deviceName = "SF51-Window";


WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
int valFan;

Servo servo;
bool RemoteMode = true;

// GPIO to use to control the IR LED circuit. Recommended: 4 (D2).
const uint16_t kIrLedPin = IRtransmit;

// The Serial connection baud rate.
// NOTE: Make sure you set your Serial Monitor to the same speed.
const uint32_t kBaudRate = 115200;

// As this program is a special purpose capture/resender, let's use a larger
// than expected buffer so we can handle very large IR messages.
const uint16_t kCaptureBufferSize = 1024; // 1024 == ~511 bits

// kTimeout is the Nr. of milli-Seconds of no-more-data before we consider a
// message ended.
const uint8_t kTimeout = 50; // Milli-Seconds
// kFrequency is the modulation frequency all UNKNOWN messages will be sent at.
const uint16_t kFrequency = 38000; // in Hz. e.g. 38kHz.
//42 - 47KHz tested for sopny middle range
IRsend irsend(kIrLedPin);


void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.hostname(deviceName); // DHCP Hostname (useful for finding device for static lease)
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
  {
    Serial.println("STA Failed to configure");
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(300);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  switch ((char)payload[0])
  {
  
  case 'M':
    switch ((char)payload[1])
    {
    case 'L': // Motor move left
      digitalWrite(SilderEnable, HIGH);
      digitalWrite(SliderDir1, HIGH);
      digitalWrite(SliderDir2, LOW);
      client.publish("Projector-Door-Left", "ON");
      client.publish("Projector-Door-Right", "OFF");
      break;
    case 'R': // Motor move right
      digitalWrite(SilderEnable, HIGH);
      digitalWrite(SliderDir1, LOW);
      digitalWrite(SliderDir2, HIGH);
      client.publish("Projector-Door-Right", "ON");
      client.publish("Projector-Door-Left", "OFF");
      break;
    case 'S': // Motor move right
      digitalWrite(SilderEnable, LOW);
      digitalWrite(SliderDir1, LOW);
      digitalWrite(SliderDir2, LOW);
      client.publish("Projector-Door-Right", "OFF");
      client.publish("Projector-Door-Left", "OFF");
      break;
    case 'X': // Motor move right
      digitalWrite(SilderEnable, HIGH);
      digitalWrite(SliderDir1, HIGH);
      digitalWrite(SliderDir2, HIGH);
      client.publish("Projector-Door-Right", "ON");
      client.publish("Projector-Door-Left", "ON");
      break;
    default:
      break;
    }
    break;


  default:
    break;
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqttClient, mqttName, mqttPASS))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("lifeTopic", "SF51-Window connected");
      client.publish("RemoteMode", "ON");
      // ... and resubscribe
      client.subscribe("SF51-Window");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      for (int b = 0; b < 10; b++)
      {
        // digitalWrite(Blinker, HIGH);
        delay(250);
        // digitalWrite(Blinker, LOW);
        delay(250);
      }
    }
  }
}

void setup()
{
  pinMode(SilderEnable, OUTPUT);
  pinMode(SliderDir1, OUTPUT);
  pinMode(SliderDir2, OUTPUT);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  irsend.begin();      // Start up the IR sender.

  ArduinoOTA.setHostname(mqttClient);
  ArduinoOTA.begin();
}

void loop()
{
  ArduinoOTA.handle();

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
}