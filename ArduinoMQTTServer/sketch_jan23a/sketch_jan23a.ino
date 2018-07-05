/*
  Arduino OpenWRT Serial Debrick Sketch
  Piero Toffanin - 2013

  Usage:
    Power off the TP-Link
    Connect TP_OUT --> Arduino RX Pin
    Connect TP_IN --> Arduino TX Pin
    (Optional) Connect 1000 Ohm resistor in series with an Led --> Pin 13
    Setup TFTP server with IP 192.168.1.100/24
    Place your image in the root TFTP folder and rename it "openwrt.bin"
    Connect Ethernet cable from TP-Link to TFTP server
    Upload Sketch to Arduino
    Power up the TP-Link
    LED will turn on during the de-bricking process
    Wait ~1 minute
    LED will flash 10 times when process is over
    Remove connections
    Reboot the TP-Link by turning it off and on
    Buy me a beer

    MIT License
  Copyright © 2013

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the “Software”), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/


#define BUFZSIZE 800
#define DEBUGPIN 13 // Connect a resistor + LED to this pin to see debug activity

char buffer[BUFZSIZE];
int c;
boolean tpl;
int command;
boolean done;
void setup()
{
  pinMode(DEBUGPIN, OUTPUT);
  digitalWrite(DEBUGPIN, LOW);

  Serial.begin(115200);
  int i;
  for (int i = 0; i < BUFZSIZE; i++) buffer[i] = 0;
  tpl = false;
  done = false;
  command = 0;
}

void loop() // run over and over
{
start:
  if (!done && Serial.available() > 0) {
    char ch = Serial.read();
    if (ch == '#') goto start;
    buffer[c] = ch;

      if (c >= 6) {
        digitalWrite(DEBUGPIN, HIGH);
        delay(8000);
        if (command == 0) {
          Serial.print("setenv ipaddr 192.168.1.120\n");
        } else if (command == 1) {
          Serial.print("setenv serverip 192.168.1.100\n");
        } else if (command == 2) {
          Serial.print("tftpboot 0x0800000 u-boot.kwb\n");
        } else if (command == 3) {
          Serial.print("sf protect off\n");
        } else if (command == 4) {
          Serial.print("sf erase all\n");
        } else if (command == 5) {
          Serial.print("sf write 0x800000 0 0x40aec\n");
        } else if (command == 6) {
          Serial.print("sf protect on\n");
        } else if (command == 7) {

          //Serial.print("boot.m 9f020000\n");
          //Serial.print("reset\n");

          done = true;
          int j;
          for (j = 0; j < 10; j++) {
            digitalWrite(DEBUGPIN, LOW);
            delay(500);
            digitalWrite(DEBUGPIN, HIGH);
            delay(500);
          }

          digitalWrite(DEBUGPIN, LOW);

          /* Uncomment to print out buffer at the end (useful for debugging)    */
                          for (j = 0; j < BUFZSIZE; j++){
                             Serial.print(buffer[j]);
                          }
       
          // Stop
          while (true) {
            delay(100);
          }
        }

        command++;
      

    }

    if (c++ > BUFZSIZE) c = 0;
  }
}

