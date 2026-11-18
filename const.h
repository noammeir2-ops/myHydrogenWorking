#ifndef CONST_H          // FIX: was "#ifndef const / #define const", which turned the keyword 'const' into an empty macro
#define CONST_H
#include <Arduino.h>

/*
*==========Arduino Nano pinout====== 
 *                            _______
 *                       TXD-|       |-Vin 
 *                       RXD-|       |-Gnd  
 *                       RST-|       |-RST
 *                       GND-|       |-+5V  
 *    IGNITION_BUTTON_IO  D2-|       |-A7  
 *              SPARK_IO  D3-|       |-A6  
 *                        D4-|       |-A5   SCL current sensor (INA226)
 *                        D5-|       |-A4   SDA current sensor (INA226)
 *                        D6-|       |-A3   SENSOR_WATER_IO
 *TURN_ON_OFF_CURRENT_IO  D7-|       |-A2   SCL soft I2C (MLX90614)
 *             CHARGE_IO  D8-|       |-A1   SDA soft I2C (MLX90614)
 *                FAN_IO  D9-|       |-A0   
 *        VALVE_HYDRO_IO D10-|       |-Ref
 *        VALVE_WATER_IO D11-|       |-3.3V   
 *                       D12-|       |-D13
 *                            --USB--        
 */
#define BAUDRATE (9600)
////////////  I/O   ////////////
#define SENSOR_WATER_IO           A3
//CURRENT_SENSOR                  A4,A5 for SCL SDA

// Software I2C for the MLX90614 (SoftI2CMaster library) - must be defined BEFORE including SoftI2CMaster.h
#define SDA_PORT PORTC
#define SDA_PIN  1          // A1 = PC1
#define SCL_PORT PORTC
#define SCL_PIN  2          // A2 = PC2
#define I2C_FASTMODE 0      // MLX90614 works up to 100 kHz

// MLX90614
#define MLX_ADDR   (0x5A << 1)
#define MLX_REG_TAMB   0x06
#define MLX_REG_TOBJ1  0x07

#define IGNITION_BUTTON_IO       2  
#define SPARK_IO                 3  // the pin connected to the transistor
#define TURN_ON_OFF_CURRENT_IO   7  // transistor -> 220 V SSR -> 12V power supply
#define CHARGE_IO                8  // the charge pin is connected to the capacitor
#define FAN_IO                   9  // the FAN is switched by a transistor
#define VALVE_HYDRO_IO           11 // the VALVE is switched by a transistor
#define VALVE_WATER_IO           10 // the VALVE is switched by a transistor

// Set to true to restart a session automatically in a loop, false = one session per button press
const bool AUTO_START = true;

// global vars
uint32_t time_start = 0;
uint32_t time_start_hydro = 0;
uint32_t time_new_session = 0;

const uint32_t ACTIVATION_TIME       = 15000; // how long the fan runs after a session (ms)
const uint32_t CURRENT_INTERVAL_TIME = 1000;  // interval between flame checks during the session (ms)
const uint32_t HYGROGEN_TIME         = 15000; // hydrogen flow time before the spark (ms)
const uint32_t FIRE_TEMP_TIME        = 5000;  // time after the spark before the first flame check (ms)
const uint32_t SESSION_TIME          = 60000; // time between spark and end of the session (ms)

//////SPARK //////
const int16_t SPARK_SPACE_TIME = 1000;  // ms
const int16_t CHARGING_TIME = 300;      // the time the capacitor is charging (ms)
const int16_t SPARK_TIME = 20;          // ms - Spark time !!! keep as short as possible to avoid HV driver damage
const int16_t NUM_OF_SPARK = 2;         // number of sparks
const int16_t DELAY_AFTER_SPARK = 500;  // ms
bool flag_spark_ready = false;
//////// FAN /////////
const int16_t DELAY_FAN_ON = 100;
const int16_t DELAY_FAN_OFF = 2000;
bool flag_ready_fan = false;

//////////// WATER VALVE and SENSOR///////////
const uint16_t NO_DETECTION = 900;//in bit resolution
const uint16_t DELAY_FILL_WATER = 500;//in ms
const uint16_t DELAY_AFTER_FILL_WATER = 1000;//in ms
const uint8_t MAX_NUMBER_FILL_WATER = 10;//
int16_t counter_fill_water;//
const int16_t ERROR_WATER_DELAY = 500;

//////////// FLAME TEMPERATURE (MLX90614) ///////////
float baseTemp = NAN;                 // FIX (was missing): temperature measured right BEFORE the spark
const float FLAME_DELTA_TEMP = 2;  // FIX (was missing): flame detected if temp rose by at least this much (C) above baseTemp
bool first_time_temp = true;

/////////BUTTON IGNITION//////////
bool check_ignit = LOW;
bool buttonPressed;
bool flag_new_session = true;
bool flag_first_press = false;
const int16_t BOUNCE_TIME = 100;//ms 

///////////VALVE HYDRO ////////////
const int DELAY_BEFORE_VALVE_OFF = 1500;  // FIX: int8_t overflows at 500
const int16_t DELAY_VALVE_ON = 500;          // FIX: int8_t overflows at 500

////////current INA////////
#define I2C_ADDRESS 0x40
const int ERROR_CURRENT_DELAY = 2000;
const float R_SHUNT_OHMS = 0.00215f;
const float I_RANGE_A    = 20.0f;
const float CURRENT_SYSTEM = 3.5;
long time_read_current = 0;
bool first_time_current = true;
const int NUM_OF_READ = 1;
int read = 0;
float average_current = 0;
int current_flag = 0;

const int DELAY_AFTER_ON = 500;//ms
#endif