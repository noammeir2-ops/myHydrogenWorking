#ifndef BASIC_ROUTINE_H
#define BASIC_ROUTINE_H
#include "const.h"
#include <avr/wdt.h>
#include <INA226_WE.h>
#include <Wire.h>

INA226_WE ina226 = INA226_WE(I2C_ADDRESS);   // hardware I2C (Wire) on A4/A5, shared with the MLX90614

// Put every output in a safe LOW state BEFORE switching it to OUTPUT
void init_pins(){
  const uint8_t outs[] = {SPARK_IO, CHARGE_IO, TURN_ON_OFF_CURRENT_IO, FAN_IO, VALVE_HYDRO_IO, VALVE_WATER_IO};
  for (uint8_t i = 0; i < sizeof(outs); i++){
    digitalWrite(outs[i], LOW);
    pinMode(outs[i], OUTPUT);
  }
  pinMode(IGNITION_BUTTON_IO, INPUT_PULLUP);   // button to GND (pressed = LOW)
}

// Non-blocking-ish wait that keeps the watchdog fed
void wait_ms(uint32_t ms){
  uint32_t t0 = millis();
  while (millis() - t0 < ms){
    wdt_reset();
    delay(10);
  }
}

bool init_current_sensor(){
  Wire.begin();
  if(!ina226.init()){
     Serial.println("No communication with the current sensor");
     return false;
  }
  ina226.setResistorRange(R_SHUNT_OHMS,I_RANGE_A);
  return true;
}

void TURN_OFF_CURRENT(){
   digitalWrite(TURN_ON_OFF_CURRENT_IO, LOW);
}

void TURN_ON_CURRENT(){
   digitalWrite(TURN_ON_OFF_CURRENT_IO, HIGH);
   delay(DELAY_AFTER_ON);
   Serial.println("current is on");
}

// 0 = I2C error, 1 = current OK (below limit), 2 = current too high
// NOTE: call init_current_sensor() in setup() before using this
int current_valid(){
  ina226.readAndClearFlags();
  float current = ina226.getCurrent_A();

  if (ina226.getI2cErrorCode() != 0) {
      delay(200);          // FIX: this delay was after the return (never executed)
      return 0;
  }
  read++;

  if(first_time_current && read == NUM_OF_READ){
    first_time_current = false;
    Serial.println("\nCurrent : " + String(current) + "A");
    Serial.println("Voltage : " + String(ina226.getBusVoltage_V()) + "V\n");
  }
  
  if (current < CURRENT_SYSTEM){
    return 1;
  }
  return 2;
}

bool check_water_level(){
  if (analogRead(SENSOR_WATER_IO) >= NO_DETECTION){
    return false;
  }
  return true;
}

bool init_temp_sensor() {
  Wire.begin();                  // safe to call more than once (also called in init_current_sensor)
  Wire.setClock(100000);         // 100 kHz - fine for both INA226 and MLX90614
#if defined(WIRE_HAS_TIMEOUT)
  Wire.setWireTimeout(3000, true);   // don't hang forever if the bus gets stuck
#endif
  Wire.beginTransmission(MLX_ADDR);
  if (Wire.endTransmission() != 0) {
    Serial.println("No communication with the MLX90614 (check A4/A5 wiring, address 0x5A)");
    return false;
  }
  return true;
}

// Reads one 16-bit register of the MLX90614 -> degrees C, or NAN on error
float mlx_read_temp_c(uint8_t reg) {
  Wire.beginTransmission(MLX_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {     // repeated start
    return NAN;
  }
  if (Wire.requestFrom((uint8_t)MLX_ADDR, (uint8_t)3) != 3) {
    return NAN;
  }
  uint8_t lo = Wire.read();
  uint8_t hi = Wire.read();
  Wire.read();                    // PEC byte, ignored

  if (hi & 0x80) return NAN;      // MSB set = error flag from the sensor
  uint16_t raw = ((uint16_t)hi << 8) | lo;
  return raw * 0.02 - 273.15;
}

float readTemperature() {
  float temperatureC = mlx_read_temp_c(MLX_REG_TOBJ1);
  if (isnan(temperatureC)) {                 // one retry
    delay(5);
    temperatureC = mlx_read_temp_c(MLX_REG_TOBJ1);
  }

  if (first_time_temp && !isnan(temperatureC)) {
    first_time_temp = false;
    Serial.print("First Temperature : ");
    Serial.println(temperatureC);
  }

  return temperatureC;
}

// Flame detection: compares the current temperature to the baseline taken before the spark.
// Returns true if the temp rose by at least FLAME_DELTA_TEMP above baseTemp.
// (renamed from detect_drop_temp - it checks for a RISE in temperature)
bool detect_flame() {
  float tempNow = readTemperature();

  // Sensor failure (or no baseline): treat as "no flame" so the session is shut down safely
  if (isnan(tempNow) || isnan(baseTemp)) {
    Serial.println("Error: no valid temperature reading");
    return false;
  }

  float delta = tempNow - baseTemp;

  Serial.print("Temp: ");
  Serial.print(tempNow);
  Serial.print(" C | Base: ");
  Serial.print(baseTemp);
  Serial.print(" C | Delta: ");
  Serial.print(delta);
  Serial.println(" C");

  return delta >= FLAME_DELTA_TEMP;
}

void FILL_WATER(){
  digitalWrite(VALVE_WATER_IO, HIGH);
  delay(DELAY_FILL_WATER);
  digitalWrite(VALVE_WATER_IO, LOW);
  delay(DELAY_AFTER_FILL_WATER);
}

bool PRESS_BUTTON_IGNITION() {
  wdt_reset();
  // Button pressed
  if (digitalRead(IGNITION_BUTTON_IO) == LOW && check_ignit == LOW) {
    check_ignit = HIGH;
    delay(BOUNCE_TIME);
  }

  // Button released -> one full press+release
  if (digitalRead(IGNITION_BUTTON_IO) == HIGH && check_ignit == HIGH) {
    check_ignit = LOW;
    return HIGH;
  }
  return LOW;
}

void CLOSE_VALVE(){
  digitalWrite(VALVE_HYDRO_IO, LOW);
  Serial.println("valve is close ");
}

void OPEN_VALVE(){
  digitalWrite(VALVE_HYDRO_IO, HIGH);
  Serial.println("valve is open ");
}

void SPARK_ON(){
  for(int i = 0; i < NUM_OF_SPARK; i++){
    digitalWrite(CHARGE_IO,HIGH);
    Serial.println("charging started");
    delay(CHARGING_TIME);
    digitalWrite(CHARGE_IO,LOW);
    Serial.println("charging stopped");

    digitalWrite(SPARK_IO, HIGH);
    Serial.println("spark high");
    delay(SPARK_TIME);
    digitalWrite(SPARK_IO, LOW);
    Serial.println("spark low");
    delay(SPARK_SPACE_TIME);
    wdt_reset();
  }
  delay(DELAY_AFTER_SPARK);
  wdt_reset();
}

void ACTIVE_FAN() {
  static bool first_time = true;
  digitalWrite(FAN_IO, HIGH);
  delay(DELAY_FAN_ON);
  if (first_time){
    first_time = false;
    Serial.println("THE FAN IS ON");
  }
}

void DEACTIVE_FAN() {
  digitalWrite(FAN_IO, LOW);
  delay(DELAY_FAN_OFF);
}

void reset_session() {
  digitalWrite(SPARK_IO, LOW);
  digitalWrite(CHARGE_IO, LOW);
  CLOSE_VALVE();
  //delay(DELAY_BEFORE_VALVE_OFF);
  delay(1500);
  TURN_OFF_CURRENT();
  Serial.println("current is off");

  read = 0;
  average_current = 0;
  first_time_current = true;
  first_time_temp = true;
  flag_first_press = false;
  flag_new_session = true;
  flag_spark_ready = false;
  baseTemp = NAN;
  time_start = millis();
}

#endif
