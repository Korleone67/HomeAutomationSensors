// The actuator will remember relay state even after power failure.
#include <MySensor.h>
#include <SPI.h>

#define RELAY_1  5  // Arduino Digital I/O pin number for first relay (second on pin+1 etc)
#define NUMBER_OF_RELAYS 1 // Total number of attached relays
#define RELAY_ON 1  // GPIO value to write to turn on attached relay
#define RELAY_OFF 0 // GPIO value to write to turn off attached relay

MySensor gw;

void setup()  
{   
  // Initialize library and add callback for incoming messages
  gw.begin(incomingMessage, AUTO, true);
  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Relay", "1.0");

  // Fetch relay status
  // Register all sensors to gw (they will be created as child devices)
  for (int sensor=1, pin=RELAY_1; sensor<=NUMBER_OF_RELAYS;sensor++, pin++) {
    // Register all sensors to gw (they will be created as child devices)
    gw.present(sensor, S_LIGHT);
    // Then set relay pins in output mode
    pinMode(pin, OUTPUT);   
    // Set relay to last known state (using eeprom storage) 
    digitalWrite(pin, gw.loadState(sensor)?RELAY_ON:RELAY_OFF);
  }
}


void loop() 
{
  gw.process();
}

void incomingMessage(const MyMessage &message) {
  if (message.type==V_LIGHT) {
     // Change relay state
     digitalWrite(message.sensor-1+RELAY_1, message.getBool()?RELAY_ON:RELAY_OFF);
     // Store state in eeprom
     gw.saveState(message.sensor, message.getBool());
   } 
}


