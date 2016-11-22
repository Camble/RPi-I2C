#include <TinyWireS.h>
#define I2C_SLAVE_ADDRESS 0x4
#define MAX_TASKS 4
#define CHECK_STATE_INTERVAL 500
#define CHECK_STATE_DELAY 5000
#define BATTERY_READ_INTERVAL 2500


typedef void(*TaskFunction)(); // Function pointer for tasks

bool activityLed = true;
uint8_t LEDPin = 1; // Onboard LED
uint8_t ADCPin = 2; // ADC0             (?)
uint8_t SwitchPin = 3; // Power switch   (?)

uint16_t vIndex = 1;
uint16_t voltages[5] = { 0 };

uint16_t data = 0;

// --------------- STATE ---------------
typedef enum state {BOOTUP, RUNNING, SHUTDOWN} State;

typedef struct {
  State current_state;
  uint16_t battery_voltage;
} SystemState;

SystemState system_state;

// --------------- TASKS ---------------
typedef struct {
  TaskFunction func;
  int16_t count;
  uint16_t max_count;
  uint16_t delay_millis;
  uint16_t previous_millis;
} Task;

Task all_tasks[MAX_TASKS];
volatile int num_tasks = 0;

int createTask(TaskFunction function, int delay, int start_delay, int repeat) {
  if (num_tasks == MAX_TASKS) { // Too many tasks?
    // Find one which is complete and overwrite it
    for (int i = 0; i < num_tasks; i++) {
      if (all_tasks[i].count >= all_tasks[i].max_count) {
        all_tasks[i].func = function;
        all_tasks[i].max_count = repeat;
        all_tasks[i].count = 0;
        all_tasks[i].delay_millis = delay;
        all_tasks[i].previous_millis = millis() + start_delay;
      }
    }
    return 0; // No available free tasks to overwrite
  }
  else {
    // Or add a new task
    all_tasks[num_tasks].func = function;
    all_tasks[num_tasks].max_count = repeat;
    all_tasks[num_tasks].count = 0;
    all_tasks[num_tasks].delay_millis = delay;
    all_tasks[num_tasks].previous_millis = millis() + start_delay;
  }
  num_tasks++;
  return 1; // Successful
}

void ExecuteTasks() {
  if(num_tasks == 0) return;
  for (int i = 0; i < num_tasks; i++) {
    // If max_count has been reached, skip the task
    if ((all_tasks[i].count >= all_tasks[i].max_count) && (all_tasks[i].max_count > -1)) {
      break;
    }
    // If the delay has elapsed
    if (all_tasks[i].previous_millis + all_tasks[i].delay_millis >= millis()) {
      // Reset the elapsed time
      all_tasks[i].previous_millis = millis();
      // Don't bother to count for infinite tasks
      if (all_tasks[i].max_count > -1) { all_tasks[i].count++; }
      // Run the task
      all_tasks[i].func();
    }
  }
}

// --------------- FUNCTIONS ---------------
/* Flashes the activity LED */
// TODO re-impliment this, tws_delay may cause problems
void flashLed(unsigned int delay, unsigned int n) {
  if (activityLed == true) {
    for (int i = 0; i <= n; i++) {
      digitalWrite(LEDPin, HIGH);
      tws_delay(delay);
      digitalWrite(LEDPin, LOW);
    }
  }
}

/* Reads the pin voltage and stores the
   average of the last 5 reads in SystemState.battery_voltage */
void readBatteryVoltage() {
  // Read the voltage
  flashLed(300, 1);
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

/* Checks the state of the power switch */
void checkState() {
  int switch_state = digitalRead(SwitchPin);
  if (switch_state == 1) { // subject to change (inverse)
    system_state.current_state = SHUTDOWN;
  }
  else {
    system_state.current_state = RUNNING;
  }
}

// --------------- I2C ---------------
void tws_requestEvent() {
  // Somehow writes the SystemState struct to the I2C bus

  char lo = system_state.battery_voltage & 0xFF;
  char hi = system_state.battery_voltage >> 8;
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

void tws_receiveEvent(uint8_t howMany) {
  // Can take instruction from the I2C master python script
  // eg. change polling frequency of battery Reads
  // eg. enable/disable power switch
  while(TinyWireS.available()) {
    flashLed(150, 2);
    data = TinyWireS.receive();
  }
}

void setup() {
  system_state.current_state = BOOTUP;

  // Setup the I2C bus
  TinyWireS.begin(I2C_SLAVE_ADDRESS);
  TinyWireS.onReceive(tws_receiveEvent);
  TinyWireS.onRequest(tws_requestEvent);

  // Setup the pins
  pinMode(LEDPin, OUTPUT);
  pinMode(ADCPin, INPUT);
  pinMode(SwitchPin, INPUT);

  // Turn of the activity LED
  digitalWrite(LEDPin, LOW);

  // TODO Add keep alive digitalWrite();

  // Create some tasks
  int task_result = createTask(readBatteryVoltage, BATTERY_READ_INTERVAL, 0, -1);
  int task_result2 = createTask(checkState, CHECK_STATE_INTERVAL, CHECK_STATE_DELAY, -1);
  if (task_result + task_result2 < 2) {
    flashLed(100, 10);
  }
}

void loop() {
  ExecuteTasks();
  TinyWireS_stop_check();
}
