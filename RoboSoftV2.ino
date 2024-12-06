//Code for Medical Delivery Robot
/*
  Use lvgl version 8.3.11

  For correct font size find the lv_conf_8.h file in c:/Users/yourName/AppData/Local/Arduino15...
  .../packages/arduino/hardware/mbed.giga/4.2.1/libraries/Arduino_H7_Video/src
  Open the file to line 372 and change the "0" to a "1"
  Then on line 394 change the default font to the 40px version
*/

#include "Arduino_H7_Video.h"
#include "lvgl.h"
#include "Arduino_GigaDisplayTouch.h"
#include "string.h"
//#include <IRremote.h> 

//Display Stuff
Arduino_H7_Video          Display(800, 480, GigaDisplayShield);
Arduino_GigaDisplayTouch  TouchDetector;

//Global Variables
const int Door1Pin = 3;
const int Door2Pin = 4;
const int steeringServoPin = 7;
bool Door1State = true;
bool Door2State = true;
bool operating = false;
char roomNum[] = "Room 1";

//Movement Variables 
// MOTOR PINS 
const int rpwm = 5; 
const int lpwm = 6; 
// ULTRASONIC SENSORS 
int leftTrigPin = 12;   // trigger 
int leftEchoPin = 13;   // echo 
long leftDistance;  
int rightTrigPin = 9;   // trigger 
int rightEchoPin = 8;   // echo 
long rightDistance;  
int frontTrigPin = 10;  // trigger 
int frontEchoPin = 11;  // echo 
long frontDistance;  
int distanceThreshold = 12; // distance threshold in inches 
unsigned long lastSensorReadTime = 0;   // time of the last sensor reading 
const int sensorReadInterval = 250;     // interval between readings (ms)

//Movement Functions------------------------------------------------------------------------------------
// FUNCTION TO READ DISTANCE FROM A SENSOR 
long readDistance(int trigPin, int echoPin) 
  { 
    digitalWrite(trigPin, LOW); 
    delayMicroseconds(5); 
    digitalWrite(trigPin, HIGH); 
    delayMicroseconds(10); 
    digitalWrite(trigPin, LOW); 
    long duration = pulseIn(echoPin, HIGH, 30000); // timeout for safety (30ms) 

    if (duration == 0) return 999; // no echo detected, return large distance 
    return (duration / 2) / 74;    // convert duration to inches 
  } 

// FUNCTION TO UPDATE SERVO ANGLE ONLY WHEN NEEDED 
void setServoAngle(int angle) 
{ 
  static int currentAngle = -1; // track the last angle 
  if (currentAngle != angle) 
  { 
    steeringServo(angle); 
     currentAngle = angle; 
  } 
} 

// DRIVE FORWARD 
void driveForward() 
{ 
  analogWrite(rpwm, 250); //forward speed 
  analogWrite(lpwm, 0); 
} 

// TURNING RIGHT 
void turningRight() 
{ 
  Serial.println("Turning Right"); 
  setServoAngle(45); // turning right 45 degrees 
  driveForward(); 
  delay(1500); 
} 

// TURNING LEFT 
void turningLeft() 
{ 
  Serial.println("Turning Left"); 
  setServoAngle(135); // turning left 45 degrees (from 90) 
  driveForward(); 
  delay(1500); 
} 

// DRIVE STRAIGHT 
void driveStraight() 
{ 
  Serial.println("Driving Straight"); 
  setServoAngle(90); // straight position 
} 

//STOPS THE CAR 
void stopCar() 
{ 
  digitalWrite(rpwm, LOW); 
  digitalWrite(lpwm, LOW); 
  Serial.println("Car Stopped"); 
} 

// DEBUG DISTANCES 
void debugDistances() 
{ 
  Serial.print("Front: "); 
  Serial.print(frontDistance); 
  Serial.print(" in, Left: "); 
  Serial.print(leftDistance); 
  Serial.print(" in, Right: "); 
  Serial.println(rightDistance); 
}
//Door Control + Servo Functions ------------------------------------------------------------------------
void writeDoor1(int angle) 
{
  // Map angle (0-180) to pulse width (in microseconds)
  int pulseWidth = map(angle, 0, 180, 544, 2400);  // Pulse width in microseconds (servo control)
  
  // Send pulse to the servo
  digitalWrite(Door1Pin, HIGH);                // Start pulse
  delayMicroseconds(pulseWidth);               // Wait for pulse width
  digitalWrite(Door1Pin, LOW);                 // End pulse
  delay(20);                                   // Short delay for stability
}

void writeDoor2(int angle) 
{
  // Map angle (0-180) to pulse width (in microseconds)
  int pulseWidth = map(angle, 0, 180, 544, 2400);  // Pulse width in microseconds (servo control)
  
  // Send pulse to the servo
  digitalWrite(Door2Pin, HIGH);                // Start pulse
  delayMicroseconds(pulseWidth);               // Wait for pulse width
  digitalWrite(Door2Pin, LOW);                 // End pulse
  delay(20);                                   // Short delay for stability
}

void steeringServo(int angle) 
{
  // Map angle (0-180) to pulse width (in microseconds)
  int pulseWidth = map(angle, 0, 180, 544, 2400);  // Pulse width in microseconds (servo control)
  
  // Send pulse to the servo
  digitalWrite(steeringServoPin, HIGH);           // Start pulse
  delayMicroseconds(pulseWidth);               // Wait for pulse width
  digitalWrite(steeringServoPin, LOW);            // End pulse
  delay(20);                                   // Short delay for stability
}
//Button Press Functions ------------------------------------------------------------------------------

//Door 1 Button Press----------------------------------------------------------------------------------
static void btn_event_cb(lv_event_t * e) 
{
  #if (LVGL_VERSION_MAJOR == 9)
    lv_obj_t * btnD1 = (lv_obj_t *) lv_event_get_target(e);
  #else 
    lv_obj_t * btnD1 = lv_event_get_target(e);
  #endif

  //Servo Positions
  if (Door1State == true)
  {
    //Lock Door 1
    writeDoor1(180);
    Door1State = false;

    //Button Prompt
    lv_obj_t * prompt = lv_obj_get_child(btnD1, 0);
    lv_label_set_text_fmt(prompt, "Unlock");
    lv_obj_set_style_bg_color(btnD1, lv_color_hex(0x8d1612), LV_PART_MAIN);
  }
  else 
  {
    //Unlock Door 1
    writeDoor1(100);
    Door1State = true;

    //Button Prompt
    lv_obj_t * prompt = lv_obj_get_child(btnD1, 0);
    lv_label_set_text_fmt(prompt, "Lock");
    lv_obj_set_style_bg_color(btnD1, lv_color_hex(0x1E6B52), LV_PART_MAIN);
  }
}
//Door 2 Button Press------------------------------------------------------------------------------------
static void btn2_event_cb(lv_event_t * e) 
{
  #if (LVGL_VERSION_MAJOR == 9)
    lv_obj_t * btnD2 = (lv_obj_t *) lv_event_get_target(e);
  #else 
    lv_obj_t * btnD2 = lv_event_get_target(e);
  #endif

  //Servo Positions
  if (Door2State == true)
  {
    //Lock Door 1
    writeDoor2(180);
    Door2State = false;

    //Button Prompt
    lv_obj_t * prompt = lv_obj_get_child(btnD2, 0);
    lv_label_set_text_fmt(prompt, "Unlock");
    lv_obj_set_style_bg_color(btnD2, lv_color_hex(0x8d1612), LV_PART_MAIN);
  }
  else 
  {
    //Unlock Door 1
    writeDoor2(100);
    Door2State = true;

    //Button Prompt
    lv_obj_t * prompt = lv_obj_get_child(btnD2, 0);
    lv_label_set_text_fmt(prompt, "Lock");
    lv_obj_set_style_bg_color(btnD2, lv_color_hex(0x1E6B52), LV_PART_MAIN);
  }
}
//Room 1 Button Press------------------------------------------------------------------------------------
static void btnR1_event_cb(lv_event_t * e) 
{
  #if (LVGL_VERSION_MAJOR == 9)
    lv_obj_t * btnR1 = (lv_obj_t *) lv_event_get_target(e);
  #else 
    lv_obj_t * btnR1 = lv_event_get_target(e);
  #endif

  //If not operating
  if (operating == false)
  {
    //Now operating
    operating = true;
    roomNum[5] = '1';

    //Button Prompt
    lv_obj_t * prompt = lv_obj_get_child(btnR1, 0);
    lv_label_set_text_fmt(prompt, "Working...");
    lv_obj_set_style_bg_color(btnR1, lv_color_hex(0x1E6B52), LV_PART_MAIN);
  }
}
//Room 2 Button Press------------------------------------------------------------------------------------
static void btnR2_event_cb(lv_event_t * e) 
{
  #if (LVGL_VERSION_MAJOR == 9)
    lv_obj_t * btnR2 = (lv_obj_t *) lv_event_get_target(e);
  #else 
    lv_obj_t * btnR2 = lv_event_get_target(e);
  #endif

  //If not operating
  if (operating == false)
  {
    //Now operating
    operating = true;
    roomNum[5] = '2';

    //Button Prompt
    lv_obj_t * prompt = lv_obj_get_child(btnR2, 0);
    lv_label_set_text_fmt(prompt, "Working...");
    lv_obj_set_style_bg_color(btnR2, lv_color_hex(0x1E6B52), LV_PART_MAIN);
  }
}
//Find Room---------------------------------------------------------------------------------------------
static void findRoom(lv_event_t * e) 
{
  //#if (LVGL_VERSION_MAJOR == 9)
  //  lv_obj_t * btnR1 = (lv_obj_t *) lv_event_get_target(e);
  //#else 
  //  lv_obj_t * btnR1 = lv_event_get_target(e);
  //#endif

    while (true)
    {

      //Move Code
      // continuously check the front sensor for obstacles 
      frontDistance = readDistance(frontTrigPin, frontEchoPin); 
      if (frontDistance > distanceThreshold) 
      { 
        // No obstacle: Drive forward 
        driveStraight(); 
        driveForward(); 
      } 
      else 
      { 
        // Obstacle detected, handle turning 
        while (frontDistance <= distanceThreshold) 
        { 
          // Read left and right distances to decide the turning direction 
          leftDistance = readDistance(leftTrigPin, leftEchoPin); 
          rightDistance = readDistance(rightTrigPin, rightEchoPin); 
          if (leftDistance > rightDistance) 
          { 
            turningLeft(); 
          } 
          else 
          { 
            turningRight(); 
          } 

          // After turning, recheck the front sensor 
          driveStraight(); 
          driveForward(); 
          delay(1000); // Stabilize after the turn 
          frontDistance = readDistance(frontTrigPin, frontEchoPin); 

          // If still blocked, the loop continues and tries turning again 
        } 
    } //End While
  }

    //Button Prompt
    //lv_obj_t * prompt = lv_obj_get_child(btnR1, 0);
    //lv_label_set_text_fmt(prompt, "Go To");
    //lv_obj_set_style_bg_color(btnR1, lv_color_hex(0xFFD602), LV_PART_MAIN);

    //operating = false;
}

//Setup ----------------------------------------------------------------------------------------------
void setup() 
{
  Display.begin();
  TouchDetector.begin();

  //Move Stuff
  Serial.begin(9600);  
  pinMode(leftTrigPin, OUTPUT);   // ultrasonic sensor pins
  pinMode(rightTrigPin, OUTPUT); 
  pinMode(frontTrigPin, OUTPUT); 
  pinMode(leftEchoPin, INPUT); 
  pinMode(rightEchoPin, INPUT); 
  pinMode(frontEchoPin, INPUT); 
  pinMode(rpwm, OUTPUT); // bts7960 motor driver pins
  pinMode(lpwm, OUTPUT); 

  //Door Servo Setup
  pinMode(Door1Pin, OUTPUT);
  pinMode(Door2Pin, OUTPUT);

  //Reset Doors to Unlock position
  writeDoor1(100);
  Door1State = true;
  writeDoor2(100);
  Door2State = true;

  //Screen Configuration
  lv_obj_t * screen = lv_obj_create(lv_scr_act());
  lv_obj_set_size(screen, Display.width(), Display.height());

  //Grid Layout (3col x 1row)
  static lv_coord_t col_dsc[] = { 375, 375, 375, LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = { 430, LV_GRID_TEMPLATE_LAST};

  //Grid Pointer and Description
  lv_obj_t * grid = lv_obj_create(lv_scr_act());
  lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);
  lv_obj_set_size(grid, Display.width(), Display.height());
  lv_obj_set_style_bg_color(grid, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_center(grid);

  //Object Declaration
  lv_obj_t * doorMenu;
  lv_obj_t * room1Menu;
  lv_obj_t * room2Menu;
  lv_obj_t * labelD1;
  lv_obj_t * labelD2;
  lv_obj_t * promptD1;
  lv_obj_t * promptD2;
  lv_obj_t * labelR1;
  lv_obj_t * labelR2;
  lv_obj_t * promptR1;
  lv_obj_t * promptR2;

  //Object Placement Grids-------------------------------------------------------------------------
  doorMenu = lv_obj_create(grid);
  lv_obj_set_grid_cell(doorMenu, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
  lv_obj_set_style_bg_color(doorMenu, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_flex_flow(doorMenu, LV_FLEX_FLOW_COLUMN); 

  room1Menu = lv_obj_create(grid);
  lv_obj_set_grid_cell(room1Menu, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
  lv_obj_set_style_bg_color(room1Menu, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_flex_flow(room1Menu, LV_FLEX_FLOW_COLUMN);

  room2Menu = lv_obj_create(grid);
  lv_obj_set_grid_cell(room2Menu, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
  lv_obj_set_style_bg_color(room2Menu, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_flex_flow(room2Menu, LV_FLEX_FLOW_COLUMN);

  //Door 1 Button Label---------------------------------------------------------------------------
  labelD1 = lv_label_create(doorMenu);
  lv_label_set_text(labelD1, "Door 1");
  lv_obj_set_style_text_color(doorMenu, lv_color_hex(0xffffff), LV_PART_MAIN);
  lv_obj_center(labelD1);

  //Door 1 Button
  lv_obj_t * btnD1 = lv_btn_create(doorMenu);
  lv_obj_set_size(btnD1, 330, 120);
  lv_obj_center(btnD1);
  lv_obj_set_style_bg_color(btnD1, lv_color_hex(0x1E6B52), LV_PART_MAIN);
  lv_obj_add_event_cb(btnD1, btn_event_cb, LV_EVENT_CLICKED, NULL);
  
  //Door 1 Button Prompt
  promptD1 = lv_label_create(btnD1);
  lv_label_set_text(promptD1, "Lock");
  lv_obj_center(promptD1);

  //Door 2 Button Label---------------------------------------------------------------------------
  labelD2 = lv_label_create(doorMenu);
  lv_label_set_text(labelD2, "Door 2");
  lv_obj_center(labelD2);

  //Door 2 Button
  lv_obj_t * btnD2 = lv_btn_create(doorMenu);
  lv_obj_set_size(btnD2, 330, 120);
  lv_obj_center(btnD2);
  lv_obj_set_style_bg_color(btnD2, lv_color_hex(0x1E6B52), LV_PART_MAIN);
  lv_obj_add_event_cb(btnD2, btn2_event_cb, LV_EVENT_CLICKED, NULL);

  //Door 2 Button Prompt
  promptD2 = lv_label_create(btnD2);
  lv_label_set_text(promptD2, "Lock");
  lv_obj_center(promptD2);

  //Room 1 Button Label---------------------------------------------------------------------------
  labelR1 = lv_label_create(room1Menu);
  lv_label_set_text(labelR1, "Room 1");
  lv_obj_set_style_text_color(room1Menu, lv_color_hex(0xffffff), LV_PART_MAIN);
  lv_obj_center(labelR1);

  //Room 1 Button
  lv_obj_t * btnR1 = lv_btn_create(room1Menu);
  lv_obj_set_size(btnR1, 330, 120);
  lv_obj_center(btnR1);
  lv_obj_set_style_bg_color(btnR1, lv_color_hex(0xFFD602), LV_PART_MAIN);
  lv_obj_add_event_cb(btnR1, btnR1_event_cb, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(btnR1, findRoom, LV_EVENT_RELEASED, NULL);

  //Room 1 Button Prompt
  promptR1 = lv_label_create(btnR1);
  lv_label_set_text(promptR1, "Go To");
  lv_obj_center(promptR1);

  //Room 2 Button Label---------------------------------------------------------------------------
  labelR2 = lv_label_create(room2Menu);
  lv_label_set_text(labelR2, "Room 2");
  lv_obj_set_style_text_color(room2Menu, lv_color_hex(0xffffff), LV_PART_MAIN);
  lv_obj_center(labelR2);

  //Room 2 Button
  lv_obj_t * btnR2 = lv_btn_create(room2Menu);
  lv_obj_set_size(btnR2, 330, 120);
  lv_obj_center(btnR2);
  lv_obj_set_style_bg_color(btnR2, lv_color_hex(0xFFD602), LV_PART_MAIN);
  lv_obj_add_event_cb(btnR2, btnR2_event_cb, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(btnR2, findRoom, LV_EVENT_RELEASED, NULL);

  //Room 2 Button Prompt
  promptR2 = lv_label_create(btnR2);
  lv_label_set_text(promptR2, "Go To");
  lv_obj_center(promptR2);

}

//Main -------------------------------------------------------------------------------------------------
void loop() 
{
  //Updates Screen
  lv_timer_handler();
}