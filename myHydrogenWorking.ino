#include "const.h"
#include "basic_routine.h"

// Ends a session safely (valve closed, power off), runs the fan, then returns
void end_session(const char* reason) {
  Serial.println(reason);
  reset_session();
  
  uint32_t t0 = millis();
  while (millis() - t0 < ACTIVATION_TIME) {   // fan runs for ACTIVATION_TIME
    ACTIVE_FAN();
    wdt_reset();
  }
  DEACTIVE_FAN();
  Serial.println("session is over");
}

void run_session() {

  Serial.println("start!");

  OPEN_VALVE();
  delay(100);
  TURN_ON_CURRENT();

  Serial.println("waiting for hydrogen...");
  wait_ms(HYGROGEN_TIME);

  // Reference temperature BEFORE the spark
  baseTemp = readTemperature();
  if (isnan(baseTemp)) {
    end_session("ABORT: no valid temperature before the spark");
    return;
  }
  Serial.print("Base temperature before spark: ");
  Serial.println(baseTemp);

  Serial.println("start the sparks!");
  SPARK_ON();
  time_new_session = millis();
  Serial.println("the spark has been activated");

  // Give the flame time to heat the sensor, then check it
  wait_ms(FIRE_TEMP_TIME);
  if (!detect_flame()) {
    end_session("NO FLAME detected - shutting down");
    return;
  }
  Serial.println("flame detected");

  // Keep checking the flame until the session ends
  while (millis() - time_new_session < SESSION_TIME) {
    wait_ms(CURRENT_INTERVAL_TIME);
    if (!detect_flame()) {
      end_session("FLAME LOST - shutting down");
      return;
    }
  }

  end_session("session finished");
}

void setup() {
  Serial.begin(BAUDRATE);
  delay(1000);

  init_pins();          // all outputs LOW, button as INPUT_PULLUP
  reset_session();      // safe state (after Serial.begin so its prints are visible)

  if (!init_temp_sensor()) {
    Serial.println("reset the arduino");
    while (1);
  }

  Serial.println("initialize");
  if (!AUTO_START) Serial.println("press the ignition button to start");
  //wdt_enable(WDTO_8S);
}

void loop() {
  if (AUTO_START || PRESS_BUTTON_IGNITION()) {
    run_session();
  }
}
