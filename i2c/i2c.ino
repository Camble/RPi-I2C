#include <DigiKeyboard.h>
#include <TinyWireS.h>
#define I2C_SLAVE_ADDRESS 0x4
#define MAX_TASKS 4
#define CHECK_STATE_INTERVAL 500
#define CHECK_STATE_DELAY 5000
#define BATTERY_READ_INTERVAL 2000
#define BATTERY_READ_DELAY 0

typedef void(*TaskFunction)(); // Function pointer

bool activityLed = true;
int LEDPin = 1;
int ADCPin = A1;
int SwitchPin = PB3;
int AlivePin = PB4;

uint16_t vIndex = 1;
uint16_t voltages[5] = { 0 };

uint16_t data = 0;

// ----- STATE -----
typedef enum state {BOOTUP, RUNNING, SHUTDOWN} State;

typedef struct {
  State current_state;
  uint16_t battery_voltage;
} SystemState;

SystemState system_state;

// ----- TASKS -----
typedef struct {
  TaskFunction func;
  int count;
  uint16_t max_count;
  uint16_t delay_millis;
  uint64_t previous_millis;
} Task;

Task all_tasks[MAX_TASKS];
volatile uint8_t num_tasks = 0;

int createTask(TaskFunction function, int delay, int start_delay, int repeat) {
  if (num_tasks == MAX_TASKS) { // Too many tasks?
    // Find one which is complete & overwrite it
    for (int i = 0; i < num_tasks; i++) {
      if (all_tasks[i].count >= all_tasks[i].max_count) {
        all_tasks[i].func = function;
        all_tasks[i].max_count = repeat;
        all_tasks[i].count = 0;
        all_tasks[i].delay_millis = delay;
        all_tasks[i].previous_millis = millis() + start_delay;
        return 1; // Success
      }
    }
    return 0; // Failure
  }
  else {
    // Or add a new task
    all_tasks[num_tasks].func = function;
    all_tasks[num_tasks].max_count = repeat;
    all_tasks[num_tasks].count = 0;
    all_tasks[num_tasks].delay_millis = delay;
    all_tasks[num_tasks].previous_millis = millis() + start_delay;
  }
  num_tasks += 1;
  return 1; // Success
}

void ExecuteTasks() {
  if (num_tasks == 0) { return; }
  for (int i = 0; i <= num_tasks; i++) {
    // If max_count is reached, skip
    if (all_tasks[i].count == all_tasks[i].max_count) {
      break;
    }
    if (all_tasks[i].previous_millis + all_tasks[i].delay_millis <= millis()) {
      // Run the task
      all_tasks[i].func();
      // Reset the elapsed time
      all_tasks[i].previous_millis = millis();
      // Don't count infinite tasks
      if (all_tasks[i].max_count > -1) { all_tasks[i].count++; }
    }
  }
}

// ----- FUNCTIONS -----
/* Flashes the activity LED */
// TODO re-impliment this if tws_delay causes problems
void flashLed(unsigned int delay, unsigned int n) {
  if (activityLed == true) {
    for (int i = 0; i <= n; i++) {
      digitalWrite(LEDPin, HIGH);
      tws_delay(delay);
      digitalWrite(LEDPin, LOW);
    }
  }
}

/* Reads the pin voltage and stores
 * the average of thelast 5 reads in
 * SystemState.battery_voltage
 */
void readBatteryVoltage() {
  // Read the voltage
  DigiKeyboard.println("Reading battery...");
  voltages[vIndex] = analogRead(ADCPin);
  vIndex++;
  if (vIndex > 4) {
    vIndex = 0;
  }
  // Total the values
  int sum = 0;
  for (int i = 1; i < 5; i++) {
    sum += voltages[i];
  }
  // Re-calculate the average
  system_state.battery_voltage = sum / 5;
}

/* Checks the state of the power switch
 */
void checkState() {
  DigiKeyboard.println("Checking switch...");

  int switch_state = digitalRead(SwitchPin);
  if (switch_state == 1) { // subject to change (inverse)
    system_state.current_state = SHUTDOWN;
  }
  else {
    system_state.current_state = RUNNING;
  }
}

// ----- I2C -----
/* Writes the SystemState struct to the I2C bus */
void tws_requestEvent() {
  // Copy the system_state struct into a byte array
  void* p = &system_state;
  uint8_t buffer[sizeof(SystemState)];
  memcpy(buffer, p, sizeof(SystemState));

  // Write buffer to I2C
  for (int i = 0; i < sizeof(buffer); i++) {
    TinyWireS.send(buffer[i]);
    flashLed(75, 1);
  }
}
/* Used to take instructions from the I2C master python script
 * eg. change polling frequency of battery Reads
 * eg. enable/disable power switch
 */
void tws_receiveEvent(uint8_t howMany) {
  while(TinyWireS.available()) {
    flashLed(150, 2);
    data = TinyWireS.receive();
  }
}

// ----- START -----
void setup() {
  DigiKeyboard.println("Running...");
  system_state.current_state = BOOTUP;

  // Initialise the pins
  pinMode(LEDPin, OUTPUT);
  digitalWrite(LEDPin, LOW);
  pinMode(ADCPin, INPUT);
  pinMode(SwitchPin, INPUT);
  //pinMode(AlivePin, OUTPUT);
  //digitalWrite(AlivePin, HIGH);
  DigiKeyboard.println("Pins OK");

  // Create some tasks
  int task_result = createTask(readBatteryVoltage, BATTERY_READ_INTERVAL, BATTERY_READ_DELAY, -1);
  int task_result2 = createTask(checkState, CHECK_STATE_INTERVAL, CHECK_STATE_DELAY, -1);
  if (task_result + task_result2 == 2) {
    DigiKeyboard.println("Tasks OK");
  }
  else {
    DigiKeyboard.println("Problem creating one or more tasks!");
  }

  // Setup the I2C bus
  TinyWireS.begin(I2C_SLAVE_ADDRESS);
  TinyWireS.onReceive(tws_receiveEvent);
  TinyWireS.onRequest(tws_requestEvent);
  DigiKeyboard.println("I2C OK");
}

void loop() {
  ExecuteTasks();
  TinyWireS_stop_check();
}


