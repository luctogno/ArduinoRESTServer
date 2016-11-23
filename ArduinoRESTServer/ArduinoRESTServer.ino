#include <ArduinoHttpServer.h>

#include <SPI.h>
#include <Ethernet.h>

//IO Setting
int val;
const int tempPin = 0;
const int ledPin = 8;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

IPAddress ip(192, 168, 0, 205);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);


//JSON STUFF
const String esIndex = "temperature_monitor";
const String esType = "temp_entry";
const String esTempKey = "temperature";
const String esTimestampKey  = "ttimestamp";
const String esAuthorKey = "author";
const String esAuthorValue = "LTArduino";

void setup() {
  pinMode(ledPin, OUTPUT);  //LED DI MONITORAGGIO

  digitalWrite(ledPin, HIGH);

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  float cel = getTemperature();

  // give the Ethernet shield a second to initialize:
  delay(1000);

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("Server started at ");
  Serial.println(Ethernet.localIP());

  getTemperature();
}

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("New Client Arrived");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          //client.println("Refresh: 10");  // refresh the page automatically every 5 sec
          float cel = getTemperature();
          String json = getJsonWithTimestamp(cel);
          client.print("Content-Length: ");
          client.println(json.length());

          client.println();

          client.println(json);
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }


        delay(1000);
        digitalWrite(ledPin, LOW);
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("Client DISCONNECTED");
  }
}

float getTemperature() {
  float val = analogRead(tempPin);
  float temp = (5 * val * 100) / 1024; //converte voltagem em temperatura

  Serial.print("TEMPERATURE = ");
  Serial.print(temp);
  Serial.print("*C");
  Serial.println();

  return temp;
}

unsigned long inline ntpUnixTime (UDP &udp)
{
  static int udpInited = udp.begin(123); // open socket on arbitrary port

  const char timeServer[] = "pool.ntp.org";  // NTP server

  // Only the first four bytes of an outgoing NTP packet need to be set
  // appropriately, the rest can be whatever.
  const long ntpFirstFourBytes = 0xEC0600E3; // NTP request header

  // Fail if WiFiUdp.begin() could not init a socket
  if (! udpInited)
    return 0;

  // Clear received data from possible stray received packets
  udp.flush();

  // Send an NTP request
  if (! (udp.beginPacket(timeServer, 123) // 123 is the NTP port
         && udp.write((byte *)&ntpFirstFourBytes, 48) == 48
         && udp.endPacket()))
    return 0;       // sending request failed

  // Wait for response; check every pollIntv ms up to maxPoll times
  const int pollIntv = 150;   // poll every this many ms
  const byte maxPoll = 15;    // poll up to this many times
  int pktLen;       // received packet length
  for (byte i = 0; i < maxPoll; i++) {
    if ((pktLen = udp.parsePacket()) == 48)
      break;
    delay(pollIntv);
  }
  if (pktLen != 48)
    return 0;       // no correct packet received

  // Read and discard the first useless bytes
  // Set useless to 32 for speed; set to 40 for accuracy.
  const byte useless = 40;
  for (byte i = 0; i < useless; ++i)
    udp.read();

  // Read the integer part of sending time
  unsigned long time = udp.read();  // NTP time
  for (byte i = 1; i < 4; i++)
    time = time << 8 | udp.read();

  // Round to the nearest second if we want accuracy
  // The fractionary part is the next byte divided by 256: if it is
  // greater than 500ms we round to the next second; we also account
  // for an assumed network delay of 50ms, and (0.5-0.05)*256=115;
  // additionally, we account for how much we delayed reading the packet
  // since its arrival, which we assume on average to be pollIntv/2.
  time += (udp.read() > 115 - pollIntv / 8);

  // Discard the rest of the packet
  udp.flush();

  return time - 2208988800ul;   // convert NTP time to Unix time
}

String getJsonWithTimestamp(float temp) {
  EthernetUDP udp;
  unsigned long unixTime = ntpUnixTime(udp);
  String json = "{";
  json.concat("\"" + esTempKey + "\":");
  json.concat(temp);
  json.concat(",");
  json.concat("\"" + esTimestampKey + "\":");
  json.concat(unixTime);
  json.concat(",");
  json.concat("\"" + esAuthorKey + "\":");
  json.concat("\"" + esAuthorValue + "\"");
  json.concat("}");

  return json;
}
