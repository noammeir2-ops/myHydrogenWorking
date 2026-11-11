#include "const.h"
#include "basic_routine.h"



void setup() {
  reset_session();
  int current_flag = 10;
  pinMode(VALVE_HYDRO_IO, OUTPUT);          // the valve

  pinMode(TURN_ON_OFF_CURRENT_IO, OUTPUT);           // Power relay
  pinMode(CHARGE_IO,OUTPUT);
  pinMode(SPARK_IO,HIGH);

  Serial.begin(BAUDRATE);         
  delay(1000);
  Serial.println("initialize");
  //wdt_enable(WDTO_8S);
}

void loop() {
  Serial.println("start!");
  //Serial.flush();
  delay(40);
  OPEN_VALVE();
  delay(100);
  TURN_ON_CURRENT();
  delay(5000);
  //current_flag = current_valid();
  Serial.println("current_flag " + current_flag);
  Serial.println("");
  Serial.println("start the sparks!");
  Serial.println(""); 

  SPARK_ON();

  delay(5000);
  CLOSE_VALVE();
  delay(1000);
  TURN_OFF_CURRENT();
  delay(100);
  
  delay(7000);
  Serial.println("session is over");
  }

