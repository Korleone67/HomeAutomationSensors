#include <DigitalIO.h>  
#include <SPI.h>  
#include <MySensor.h>
#include <MyGateway.h>  
#include <stdarg.h>
#include <Ethernet.h>   


#define INCLUSION_MODE_TIME 1 // Number of minutes inclusion mode is enabled
#define INCLUSION_MODE_PIN  3 // Digital pin used for inclusion mode button

#define RADIO_CE_PIN        5  // radio chip enable
#define RADIO_SPI_SS_PIN    6  // radio SPI serial select
#define RADIO_ERROR_LED_PIN 7  // Error led pin
#define RADIO_RX_LED_PIN    8  // Receive led pin
#define RADIO_TX_LED_PIN    9  // the PCB, on board LED

#define IP_PORT 5003        // The port you want to open 
IPAddress myIp (192, 168, 1, 7);  // Configure your static ip-address here    COMPILE ERROR HERE? Use Arduino IDE 1.5.7 or later!

// The MAC address can be anything you want but should be unique on your network.
// Newer boards have a MAC address printed on the underside of the PCB, which you can (optionally) use.
// Note that most of the Ardunio examples use  "DEAD BEEF FEED" for the MAC address.
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x4A, 0x3E };  // DEAD BEEF FEED

EthernetServer server = EthernetServer(IP_PORT);

// No blink or button functionality. Use the vanilla constructor.
MyGateway gw(RADIO_CE_PIN, RADIO_SPI_SS_PIN, INCLUSION_MODE_TIME);

char inputString[MAX_RECEIVE_LENGTH] = "";    // A string to hold incoming commands from serial/ethernet interface
int inputPos = 0;

String incoming;

void setup()  
{ 
  Ethernet.begin(mac, myIp);
  delay(1000);

  // Initialize gateway at maximum PA level, channel 70 and callback for write operations 
  gw.begin(RF24_PA_LEVEL_GW, RF24_CHANNEL, RF24_DATARATE, writeEthernet);

  // start listening for clients
  server.begin();
}

// This will be called when data should be written to ethernet 
void writeEthernet(char *writeBuffer) {
  server.write(writeBuffer);
}


void loop()
{
  // if an incoming client connects, there will be
  // bytes available to read via the client object
  EthernetClient client = server.available();

  if (client) {
    Serial.println("Incoming message");
    
      // if got 1 or more bytes
      if (client.available()) {
         // read the bytes incoming from the client
         char inChar = client.read();
         incoming = "Incoming char: " + inChar;
         Serial.println(incoming);
         if (inputPos<MAX_RECEIVE_LENGTH-1) { 
           // if newline then command is complete
           if (inChar == '\n') {  
             Serial.println("End if command");
              // a command was issued by the client
              // we will now try to send it to the actuator
              inputString[inputPos] = 0;

              // echo the string to the serial port
              Serial.print(inputString);

              gw.parseAndSend(inputString);

              // clear the string:
              inputPos = 0;
           } else {  
             // add it to the inputString:
             inputString[inputPos] = inChar;
             inputPos++;
           }
        } else {
           // Incoming message too long. Throw away 
           Serial.print(inputString);
           inputPos = 0;
        }
      }
   }  
   gw.processRadioMessage();    
}


