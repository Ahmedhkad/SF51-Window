#include <Arduino.h>
#include "settings.h" //GPIO defines, NodeMCU good-pin table
#include "secret.h"   //Wifi and mqtt server info

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>

const char *ssid = ssidWifi;
const char *password = passWifi;
const char *mqtt_server = mqttURL;

IPAddress local_IP(192, 168, 1, 162);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   // optional
IPAddress secondaryDNS(8, 8, 4, 4); // optional

const char *deviceName = "SF51-Window";

WiFiClient espClient;
PubSubClient client(espClient);

StaticJsonDocument<150> doc;
int device;
int valuejson;
int datajson;

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

  for (unsigned i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  DeserializationError error = deserializeJson(doc, payload);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  // Print the values to data types
  device = doc["device"].as<unsigned int>();
  valuejson = doc["value"].as<unsigned int>();
  datajson = doc["data"].as<unsigned int>();

  switch (device)
  {
  case 1:
    if (valuejson == 1) // roll up
    {
      analogWrite(MotorEnable, datajson);
      digitalWrite(Window1DirA, HIGH);
      digitalWrite(Window1DirB, LOW);
      client.publish("status", "UP");
      client.publish("SF51-Projector-Screen",
                     "{\"device\":\"5\",\"value\":\"1\"}");
    }
    else if (valuejson == 2) // roll down
    {
      analogWrite(MotorEnable, datajson);
      digitalWrite(Window1DirA, LOW);
      digitalWrite(Window1DirB, HIGH);
      client.publish("status", "DOWN");
      client.publish("SF51-Projector-Screen",
                     "{\"device\":\"5\",\"value\":\"1\"}");
    }
    else if (valuejson == 3) // off and lock
    {
      digitalWrite(MotorEnable, HIGH);
      digitalWrite(Window1DirA, HIGH);
      digitalWrite(Window1DirB, HIGH);
      client.publish("status", "OFF");
      client.publish("SF51-Projector-Screen",
                     "{\"device\":\"5\",\"value\":\"0\"}"); // Power Supply OFF
    }

    break;
  case 2:
    if (valuejson == 1) // roll up
    {
      analogWrite(MotorEnable, datajson);
      digitalWrite(Window2DirA, HIGH);
      digitalWrite(Window2DirB, LOW);
      client.publish("status", "UP");
      client.publish("SF51-Projector-Screen",
                     "{\"device\":\"5\",\"value\":\"1\"}");
    }
    else if (valuejson == 2) // roll down
    {
      analogWrite(MotorEnable, datajson);
      digitalWrite(Window2DirA, LOW);
      digitalWrite(Window2DirB, HIGH);
      client.publish("status", "DOWN");
      client.publish("SF51-Projector-Screen",
                     "{\"device\":\"5\",\"value\":\"1\"}");
    }
    else if (valuejson == 3) // off and lock
    {
      digitalWrite(MotorEnable, HIGH);
      digitalWrite(Window2DirA, HIGH);
      digitalWrite(Window2DirB, HIGH);
      client.publish("status", "OFF");
      client.publish("SF51-Projector-Screen",
                     "{\"device\":\"5\",\"value\":\"0\"}"); // Power Supply OFF
    }
    break;

  default:
    client.publish("ALL", "ERROR");
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
      // ... and resubscribe
      client.subscribe("SF51-Window");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  pinMode(MotorEnable, OUTPUT);
  pinMode(Window1DirA, OUTPUT);
  pinMode(Window1DirB, OUTPUT);
  pinMode(Window2DirA, OUTPUT);
  pinMode(Window2DirB, OUTPUT);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

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