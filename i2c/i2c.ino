// #include <TinyWireS.h>
#include <USIWire.h>
#define I2C_SLAVE_ADDRESS 0x4
#define BATTERY_READ_INTERVAL 1000
#define BATTERY_NUMREADINGS 5

int ADCPin = A1;

uint8_t vIndex = 1;
uint16_t voltages[BATTERY_NUMREADINGS] = { 0 };
volatile uint8_t addr = 0;

// ----- STATE -----
typedef enum state {BOOTUP, RUNNING, SHUTDOWN} State;

typedef struct {
  State current_state;
  uint16_t battery_voltage;
} SystemState;

SystemState system_state;

uint16_t interval_millis;
uint64_t previous_millis;

/* Reads the pin voltage and stores
 * the average in system_state.battery_voltage
 */
void readBatteryVoltage() {
  if (previous_millis + BATTERY_READ_INTERVAL <= millis()) {
    previous_millis = millis();

    // Read the voltage
    voltages[vIndex] = analogRead(ADCPin);
    float v = voltages[vIndex] * (5.00 / 1023.00);

    vIndex++;
    if (vIndex >= BATTERY_NUMREADINGS) {
      vIndex = 0;
    }
    // Total the values
    int sum = 0;
    for (int i = 1; i < BATTERY_NUMREADINGS; i++) {
      sum += voltages[i];
    }
    // Re-calculate the average
    system_state.battery_voltage = sum / BATTERY_NUMREADINGS;

    // Create dummy data for system_state
    system_state.battery_voltage = 300;
    system_state.current_state = RUNNING;
  }
}

// ----- I2C -----
/* Writes the SystemState struct to the I2C bus */
void requestEvent() {
  // Copy system_state into a byte array
  void* p = &system_state;
  uint8_t buffer[sizeof(SystemState)];
  memcpy(buffer, p, sizeof(SystemState));

  uint8_t i = sizeof(buffer);
  
  // transmit the requested bytes
  while (i) {
    Wire.write(buffer[addr]);
    addr++; // set next addr
    if (addr >= 4) addr = 0; // start at 0 on register end
    i--;
  }
}

/* Used to take instructions from the I2C master python script
 * eg. change polling frequency of battery Reads
 * eg. enable/disable power switch
 */
void receiveEvent(uint8_t howMany) {
  // while(TinyWireS.available()) {
  //   int data = TinyWireS.receive();
  // }
}

// ----- START -----
void setup() {
  system_state.current_state = BOOTUP;

  // Initialise the pins
  pinMode(ADCPin, INPUT);

  // Setup the I2C bus
  Wire.begin(I2C_SLAVE_ADDRESS);
  Wire.onRequest(requestEvent); 
  // Wire.onReceive(receiveEvent);
}

void loop() {
  readBatteryVoltage();
  // TinyWireS_stop_check();
}