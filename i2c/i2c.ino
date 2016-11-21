#include <TinyWireS.h>
#define I2C_SLAVE_ADDRESS 0x4
#define MAX_TASKS 4

typedef enum state {BOOTUP, STABLE, SHUTDOWN} State;
typedef void(*TaskFunction)(); // Function pointer for tasks

bool activityLed = true;
uint8_t ADCpin = 1; // ADC0
uint8_t LEDPin = 1; // Onboard LED

uint16_t vIndex = 1;
uint16_t voltages[6] = { 0 }; // [0] = latest average

uint16_t data = 0;

// --------------- TASKS ---------------
typedef struct {
  //void *arg;
  TaskFunction func;
  int16_t count;
  uint16_t max_count;
  uint16_t delay_millis;
  uint16_t previous_millis;
} Task;

Task all_tasks[MAX_TASKS];
volatile int num_tasks = 0;

int createTask(TaskFunction function, int delay, int start_delay, int repeat) {
  int current_task = num_tasks;
  if (num_tasks == MAX_TASKS) {
    for (int i = 0; i < num_tasks; i++) {
      if (all_tasks[i].count >= all_tasks[i].max_count) {
        //all_tasks[i].arg = arg;
        all_tasks[i].func = function;
        all_tasks[i].max_count = repeat;
        all_tasks[i].count = 0;
        all_tasks[i].delay_millis = delay;
        all_tasks[i].previous_millis = millis() + start_delay;
      }
    }
    return -1;
  }
  else {
    //all_tasks[current_task].arg = arg;
    all_tasks[current_task].func = function;
    all_tasks[current_task].max_count = repeat;
    all_tasks[current_task].count = 0;
    all_tasks[current_task].delay_millis = delay;
    all_tasks[current_task].previous_millis = millis() + start_delay;
  }
  num_tasks++;
  return 0;
}

void ExecuteTasks() {
  if(num_tasks == 0) return;
  for (int i = 0; i < num_tasks; i++) {
    if (all_tasks[i].count >= all_tasks[i].max_count) return;
    if (all_tasks[i].previous_millis + all_tasks[i].delay_millis >= millis()) {
      all_tasks[i].previous_millis = millis();
      all_tasks[i].func();
    }
  }
}

// --------------- FUNCTIONS ---------------
/* Flashes th activity LED */
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
   average of the last 5 reads in voltages[0] */
void readBatteryVoltage() {
  // Read the voltage
  flashLed(300, 1);
  voltages[vIndex] = analogRead(ADCpin);
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

void checkUserState() {
}

// --------------- I2C ---------------
void tws_requestEvent() {
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
  State current_state = BOOTUP;

  // Setup the I2C bus
  TinyWireS.begin(I2C_SLAVE_ADDRESS);
  TinyWireS.onReceive(tws_receiveEvent);
  TinyWireS.onRequest(tws_requestEvent);

  // Setup the pins
  pinMode(LEDPin, OUTPUT);
  pinMode(ADCpin, INPUT);

  // Turn of the activity LED
  digitalWrite(LEDPin, LOW);

  // Create some tasks
  int result = createTask(readBatteryVoltage, 1000, 0, -1);
  result = createTask(checkUserState, 1000, 500, -1);
}

void loop() {
  ExecuteTasks();
  TinyWireS_stop_check();
}
