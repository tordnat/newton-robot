#include <Arduino.h>
#include <PS4Controller.h>
#include "motor.h"

//Pins
#define BLDC_PIN 33
#define MOTOR_R_PIN0 12
#define MOTOR_R_PIN1 14
#define MOTOR_L_PIN0 27
#define MOTOR_L_PIN1 26

//Frequencies
#define DC_FREQ 20000
#define BLDC_FREQ 50

//LEDC channels esp-hal uses modulo 4 to select timers for different channels, therefore BLDC is the only odd channel 
#define BLDC_CHAN 1
#define MOTOR_L_CHAN0 2
#define MOTOR_L_CHAN1 4
#define MOTOR_R_CHAN0 6
#define MOTOR_R_CHAN1 8

//Motor structs setup
Motor motorWeapon {BLDC_PIN, 0, BLDC_CHAN, 0, BLDC_FREQ, MOTOR_TYPE_BLDC};
Motor motorRight {MOTOR_R_PIN0, MOTOR_R_PIN1, MOTOR_R_CHAN0, MOTOR_R_CHAN1, DC_FREQ, MOTOR_TYPE_DC};
Motor motorLeft {MOTOR_L_PIN0, MOTOR_L_PIN1, MOTOR_L_CHAN0, MOTOR_L_CHAN1, DC_FREQ, MOTOR_TYPE_DC};

int LStickY_offset = 0;
int RStickX_offset = 0;
int R2Value_offset = 0;

int stopped = 0;
int bldc_ramp_setpoint = 0; //This increases when button is pressed, decreases when released

void steering(Motor motorLeft, Motor motorRight, PS4Controller PS4) {
  if(PS4.isConnected()) {
    int16_t motor_speed_L = 0;
    int16_t motor_speed_R = 0;
    motor_speed_L += (PS4.LStickY() - LStickY_offset);
    motor_speed_R += (PS4.LStickY() -  LStickY_offset);
    
    if(abs(PS4.RStickX() + RStickX_offset) > 8) {
      motor_speed_L -= (PS4.RStickX() + RStickX_offset)/2; //Divide for smoother steering
      motor_speed_R += (PS4.RStickX() - RStickX_offset)/2; 
    }

    if(motor_speed_L > 127) {
      motor_speed_L = 127;
    }
    if(motor_speed_R > 127) {
      motor_speed_R = 127;
    }
    if(motor_speed_L < -127) {
      motor_speed_L = -127;
    }
    if(motor_speed_R < -127) {
      motor_speed_R = -127;
    }
    setMotorSpeed(motor_speed_L, motorLeft);
    setMotorSpeed(motor_speed_R, motorRight);

    //This must be tested!
    int bldc_max_speed = 100;
    int bldc_min_speed = 0;
    if(PS4.R2Value() > 200) {
        bldc_ramp_setpoint += 1;
    }
    if(PS4.R2Value() < 200) {
        bldc_ramp_setpoint -= 1;
    }
    if(bldc_ramp_setpoint > bldc_max_speed) {
      bldc_ramp_setpoint = bldc_max_speed;
    }
    if(bldc_ramp_setpoint < bldc_min_speed) {
      bldc_ramp_setpoint = bldc_min_speed;
    }
    
    if(PS4.L1()) {
      setMotorSpeed(-bldc_ramp_setpoint, motorWeapon); 
    } else {
      setMotorSpeed(bldc_ramp_setpoint, motorWeapon);
    }
  }
  else {
    setMotorSpeed(0, motorLeft);
    setMotorSpeed(0, motorRight);
    setMotorSpeed(0, motorWeapon);
    bldc_ramp_setpoint = 0;
  }
  //Safety
  if(PS4.Circle()) {
    setMotorSpeed(0, motorLeft);
    setMotorSpeed(0, motorRight);
    setMotorSpeed(0, motorWeapon);
    stopped = 1;
    bldc_ramp_setpoint = 0;
  }
}

void PS4_calibrate() {
  int LStickY_offset = PS4.LStickY();
  int RStickX_offset = PS4.RStickX();
  int R2Value_offset = PS4.R2Value();
}

void setup() {
  Serial.begin(9600);
  delay(10);
  motorInit(motorWeapon);
  motorInit(motorLeft);
  motorInit(motorRight);
  
  Serial.printf("Test\n");
  PS4.begin("a8:7e:ea:6e:bd:87");
  while(!PS4.isConnected()) {
    Serial.print("Connecting to PS4-controller");
  }
  delay(200);
  PS4_calibrate();
}

void loop() {
  if(stopped) {
    if(PS4.Square()) {
      stopped = 0;
    }
  } else {
    steering(motorLeft, motorRight, PS4);
  }
  

  //delay(1);
  /*
  if (PS4.isConnected()) {
    if (PS4.Square()) Serial.println("Square Button");
    if (PS4.Cross()) Serial.println("Cross Button");
    if (PS4.Circle()) Serial.println("Circle Button");
    if (PS4.Triangle()) Serial.println("Triangle Button");

    if (PS4.UpRight()) Serial.println("Up Right");
    if (PS4.DownRight()) Serial.println("Down Right");
    if (PS4.UpLeft()) Serial.println("Up Left");
    if (PS4.DownLeft()) Serial.println("Down Left");

    if (PS4.L1()) Serial.println("L1 Button");
    if (PS4.R1()) Serial.println("R1 Button");

    if (PS4.LStickX()) {
      Serial.printf("Left Stick x at %d\n", PS4.LStickX());
    }
    if (PS4.LStickY()) {
      Serial.printf("Left Stick y at %d\n", PS4.LStickY());
    }
    if (PS4.RStickX()) {
      Serial.printf("Right Stick x at %d\n", PS4.RStickX());
    }
    if (PS4.RStickY()) {
      Serial.printf("Right Stick y at %d\n", PS4.RStickY());
    }

    if (PS4.Charging()) Serial.println("The controller is charging");
    if (PS4.Audio()) Serial.println("The controller has headphones attached");
    if (PS4.Mic()) Serial.println("The controller has a mic attached");

    Serial.printf("Battery Level : %d\n", PS4.Battery());

  }else {
    Serial.printf("Controller not connected");
  }    
  Serial.printf("\n");
  */
}

