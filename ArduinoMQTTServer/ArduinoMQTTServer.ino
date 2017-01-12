/*
  Publishing in the callback
  - connects to an MQTT server
  - subscribes to the topic "inTopic"
  - when a message is received, republishes it to "outTopic"
  This example shows how to publish messages within the
  callback function. The callback function header needs to
  be declared before the PubSubClient constructor and the
  actual callback defined afterwards.
  This ensures the client reference in the callback function
  is valid.
*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <elapsedMillis.h>

//IOT PIN
const int LIGHT_PIN = 5;
const int TEMP_PIN = A1;
const int timerInvio = 10000;

//IOT TOPIC
const char* sLightTopic = "iot/light";
const char* sMessageTopic = "iot/message";
const char* sTempTopic = "iot/temperature";
elapsedMillis sinceTest1;

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 205);
IPAddress server(192, 168, 0, 65);

// Callback function header
void callback(char* topic, byte* payload, unsigned int length);

EthernetClient ethClient;
PubSubClient client(server, 8883, callback, ethClient);

// Callback function
void callback(char* topic, byte* payload, unsigned int lengthi) {
  if (strcmp(sLightTopic, topic) == 0) {
    char messagePayload[lengthi + 1];

    memcpy( messagePayload, &payload[0], lengthi );
    messagePayload[lengthi] = '\0';

    char* light_on_message = "accendi";
    char* light_off_message = "spegni";

    if (strcmp(light_on_message, messagePayload) == 0) {
      turnLightOnOff(true);
    } else {
      if (strcmp(light_off_message, messagePayload) == 0) {
        turnLightOnOff(false);
      } else {
        client.publish(sMessageTopic, "BHO");
      }
    }
  }

  client.publish(sMessageTopic, "RECEIVED");
}

void setup()
{
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  if (client.connect("arduinoClient", "admin", "inter")) {
    client.publish(sMessageTopic, "Arduino HELLO!");
    client.subscribe(sLightTopic);
  }
  pinMode(LIGHT_PIN, OUTPUT);
}

void loop()
{
  if (sinceTest1 >= timerInvio)
  {
    sinceTest1 = sinceTest1 - timerInvio;
    float cel = getTemperature();
    Serial.println(cel);
    char lll[15];
    dtostrf(cel, 7, 3, lll);
    client.publish(sTempTopic, lll);
  }
  client.loop();
}

void turnLightOnOff(boolean on)
{
  if (on == true) {
    Serial.println("ON");
    digitalWrite(LIGHT_PIN, HIGH);
  } else {
    Serial.println("OFF");
    digitalWrite(LIGHT_PIN, LOW);
  }
}

float getTemperature() {
  float val = analogRead(TEMP_PIN);
  float temp = (5 * val * 100) / 1024; //converte voltagem em temperatura

  return temp;
}
