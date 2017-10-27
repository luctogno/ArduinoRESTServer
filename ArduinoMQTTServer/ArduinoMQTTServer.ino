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

#include <avr/io.h>
#include <avr/wdt.h>

#define Reset_AVR() wdt_enable(WDTO_30MS); while(1) {}

//IOT PIN
#define LIGHT_PIN 5
#define TEMP_PIN A0
#define timerInvio 60000
#define timerRiavvio 

#define ardu_pos "man_ard" // sostituire a mano sotto

//IOT TOPIC
#define light_on_payload "ON"
#define light_off_payload "OFF"
#define sLightSWTopic "iot/light/man_ard/switch"
#define sLightSTTopic "iot/light/man_ard/status"
#define sLightSTCommandTopic "iot/light/man_ard/status/read"
#define sMessageTopic "iot/message"
#define sResetTopic "arduino/reset"
#define sTempTopic "iot/temperature/man_ard"

elapsedMillis sinceTest1;

// Update these with values suitable for your network.
byte mac[] = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 205);
IPAddress server(192, 168, 0, 65);

// Callback function header
void callback(char* topic, byte* payload, unsigned int length);

EthernetClient ethClient;
PubSubClient client(server, 8883, callback, ethClient);

// Callback function
void callback(char* topic, byte* payload, unsigned int lengthi) {
  if (strcmp(sLightSWTopic, topic) == 0) {
    //GET PAYLOAD
    char messagePayload[lengthi + 1];
    memcpy( messagePayload, &payload[0], lengthi );
    messagePayload[lengthi] = '\0';

    if (strcmp(light_on_payload, messagePayload) == 0) {
      turnLightOnOff(LIGHT_PIN, true);
      publishLightState(LIGHT_PIN);
    } else {
      if (strcmp(light_off_payload, messagePayload) == 0) {
        turnLightOnOff(LIGHT_PIN, false);
        publishLightState(LIGHT_PIN);
      } else {
        client.publish(sMessageTopic, " LIGHT ERROR!");
      }
    }
  }

  if (strcmp(sLightSTCommandTopic, topic) == 0) {
    publishLightState(LIGHT_PIN);
  }

  if (strcmp(sResetTopic, topic) == 0) {
    Reset_AVR();
  }

  //client.publish(sMessageTopic, "RECEIVED");
}

void setup()
{
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  if (client.connect("man_ard_Client", "admin", "inter")) {
    client.publish(sMessageTopic, "Arduino man_ard connected!");
    client.subscribe("iot/#");
    client.subscribe("arduino/#");
  }
  pinMode(LIGHT_PIN, OUTPUT);
}

void loop()
{
  if (sinceTest1 >= timerInvio)
  {
    sinceTest1 = sinceTest1 - timerInvio;
    float cel = getTemperature();
    char lll[10];
    dtostrf(cel, 7, 3, lll);
    client.publish(sTempTopic, lll);
  }
  client.loop();
}

void turnLightOnOff(int pin, boolean on)
{
  if (on == true) {
    Serial.println("ON");
    digitalWrite(LIGHT_PIN, HIGH);
  } else {
    Serial.println("OFF");
    digitalWrite(LIGHT_PIN, LOW);
  }
}

// function called to publish the state of the light (on/off)
void publishLightState(int pin) {
  if (digitalRead(pin) == HIGH) {
    client.publish(sLightSTTopic, light_on_payload, true);
  } else {
    client.publish(sLightSTTopic, light_off_payload, true);
  }
}

float getTemperature() {
  float val = analogRead(TEMP_PIN);
  float temp = val * 0.48828125; //converte voltagem em temperatura
  return temp;
}
