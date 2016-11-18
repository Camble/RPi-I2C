#include <TinyWireS.h>
// --------------- CONSTANTS ---------------
#define I2C_SLAVE_ADDRESS 0x4

// --------------- TYPEDEF ---------------
typedef void(*task_function)(void *arg); // Function pointer for tasks
typedef enum {BOOTUP, STABLE, SHUTDOWN} State;




State current_state = BOOTUP;

// --------------- TASKS ---------------
struct Task {
  void *func;

  uint16_t startDelaySecs;
  uint16_t delayMillis;
};

//struct Task Tasks[1];

int createTask(task_function function, int delay) {

  return -1;
}
// --------------- STATIC VARIABLES ---------------
bool activityLed = true;
unsigned int pin = 1; // ADC0

unsigned int LEDPin = 1; // Onboard LED
unsigned int data = 0;
unsigned int vIndex = 1;
unsigned int voltages[6] = { 0 }; // 0 = average of 1-5

unsigned long previousRead = 0; // millis() of the previous analogRead()

void setup() {
  // createTask(readBatteryVoltage) // checks the battery voltage
  // createTask(checkUserState) // checks the state of the power switch (user input)


  TinyWireS.begin(I2C_SLAVE_ADDRESS);
  TinyWireS.onReceive(receiveEvent);
  TinyWireS.onRequest(requestEvent);
  pinMode(LEDPin, OUTPUT);
  digitalWrite(LEDPin, LOW);
}

void loop() {
  TinyWireS_stop_check();
  readVoltage(pin);
}
// --------------- FUNCTIONS ---------------
void flashLed(unsigned int delay, unsigned int n) {
  if (activityLed == true) {
    for (int i = 0; i <= n; i++) {
      digitalWrite(LEDPin, HIGH);
      tws_delay(delay);
      digitalWrite(LEDPin, LOW);
    }
  }
}

// Reads the pin voltage if one second has passed
// Averages the last 5 voltages
// Stores average in voltages[0]
void readVoltage(unsigned int p) {
  unsigned long dt = millis() - previousRead;
  if (dt >= 1000) {
    previousRead = millis();
    // Read the voltage
    flashLed(300, 1);
    voltages[vIndex] = analogRead(p);
    vIndex++;
    if (vIndex > 5) {
      vIndex = 1;
    }
    // Total the values
    int sum = 0;
    for (int i = 1; i < 6; i++) {
      sum += voltages[i];
    }
    // Re-calculate the average
    voltages[0] = sum / 5;
  }
}

// --------------- I2C ---------------
void requestEvent() {
  // Writes the current State and battery information to the I2C bus

  voltages[0] = 65535;
  char lo = voltages[0] & 0xFF;
  char hi = voltages[0] >> 8;
  TinyWireS.send(lo);
  TinyWireS.send(hi);
  /*
  uint8_t data[2] = { lo, hi };
  for (int i = 0; i <= 2; i++) {
    TinyWireS.send(data[i]);
    flashLed(100, 2);
  }
  */
}

void receiveEvent(uint8_t howMany) {
  // Can take instruction from the I2C master python script
  // eg. change polling frequency of battery Reads
  // eg. enable/disable power switch
  while(TinyWireS.available()) {
    flashLed(150, 2);
    data = TinyWireS.receive();
  }
}
