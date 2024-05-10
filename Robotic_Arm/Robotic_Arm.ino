//Template for PS4 Controllers
#include <Bluepad32.h>
#include <ESP32Servo.h>

// Default Angle Const
#define ANG1 150
#define ANG2 110
#define ANG3 5
#define ANG4 90
#define ANG5 90

// Constant Declaration of Servo Pins
const int servo_pin[5] = {23,22,21,19,18}; // Gripper, Gripper Chassis, First Arm, Second Arm, Base
// Define the maximum number of sets to store
const int MAX_SETS = 700;
// Multidimensional array to store sets of servo angles
int servoAngles[MAX_SETS];
// Index to keep track of the current set being stored
volatile int currentIndex = 0;
// Sets exist
bool Dataset;

//
long long current_time = 0, last_time = 0;
const int threshold = 1000;

// Angle Variables for 5 servos (Initial Angle)
int angle_1 = ANG1;
int angle_2 = ANG2;
int angle_3 = ANG3;
int current_angle_4 = ANG4;
int angle_5 = ANG5;
int angle[5];

int mode = 0;
int min_angle_4, max_angle_4, delta_angle_4 = 0;
int last_pos = 150;

// Record and Play
int action_servo, action_pos;
void Record(int*,int*);
void Play(bool);

//Controller and Servo Setup
ControllerPtr myControllers;
Servo myservo[5];

//Connection Phase
void onConnectedController(ControllerPtr per) {
  if (myControllers == nullptr) {
    myControllers = per;
    Serial.println("Declaration: " + per->getModelName() + " has connected!");
    per->setColorLED(0, 0, 255);  // Blue == Connected
  }
}

//Disconnected Phase
void onDisconnectedController(ControllerPtr per) {
  if (myControllers == per) {
    Serial.println("CALLBACK: Controller disconnected!");
    myControllers = nullptr;
  }
}

void GP_Input() {
  //Joystick Controller
  int X1 = map(myControllers->axisX(), -512, 512, 0, 255);
  int Y1 = map(myControllers->axisY(), -512, 512, 255, 0);
  int X2 = map(myControllers->axisRX(), -512, 512, 0, 255);
  int Y2 = map(myControllers->axisRY(), -512, 512, 255, 0);

  //Dpad Buttons
  bool UP = myControllers->dpad() == 1;
  bool RIGHT = myControllers->dpad() == 4;
  bool DOWN = myControllers->dpad() == 2;
  bool LEFT = myControllers->dpad() == 8;

  //Right Shape Buttons
  bool Tri = myControllers->y() == 1;
  bool Cir = myControllers->b() == 1;
  bool Cro = myControllers->a() == 1;
  bool Squ = myControllers->x() == 1;

  //Shoulder Buttons
  bool L1 = myControllers->l1();
  bool R1 = myControllers->r1();

  //Shoulder Analog Buttons
  int L2 = myControllers->brake();
  int R2 = myControllers->throttle();

  //Thumb Buttons
  bool thumb_L = myControllers->thumbL() == 1;
  bool thumb_R = myControllers->thumbR() == 1;

  //Misc Buttons
  bool System = myControllers->miscSystem();
  bool Back = myControllers->miscBack();  // Left Button
  bool Home = myControllers->miscHome();  // Right Button

  //Battery Status
  // int batt_lvl = myControllers->battery();  //0 ~ 255 (0 == unknown status; 1 == empty; 255 == full)

  // Serial.println("Left-X: " + String(X1) + " Left-Y: " + String(Y1) + " Right-X: " + String(X2) + " Right-Y: " + String(Y2));

  // Angle Variables
  int min_angle_4 = static_cast<int>(1.2*angle_3 + 84);
  int max_angle_4 = min_angle_4 + 50;
  int current_angle_4 = min_angle_4 + delta_angle_4;

  if (max_angle_4 >= 160){
    max_angle_4 = 160;
  }

  // Mode Selection
  if(Tri){
    mode = 1;
    myControllers->setColorLED(255,0,0);
  }else if(Cir){
    mode = 2;
    myControllers->setColorLED(0,255,0);
  }else if(Cro){
    mode = 3;
    myControllers->setColorLED(0,0,0);
  // Replay the Motion
  }else if(Back && Dataset){
    mode = 4;
  }

  bool Last_Squ_State = 0;
  if(Squ != Last_Squ_State){
    if(Squ){
      Dataset = 1;
      myControllers->setColorLED(0, 0, 255);
      if(current_time - last_time > threshold){
        // Record each angle of the servo
        Record(angle,servoAngles);
        last_time = current_time;
        myControllers->setRumble(0xc0, 0x80); // force, duration
      }
      
    }
  }

  // Gripper Chassis Rotation
  if((mode==1) && UP && (angle_2 < ANG2)){
    angle_2++;
  }else if((mode==1) && DOWN && (angle_2 > 0)){
    angle_2--;
  }

  // First Arm Rotation
  if((mode==2) && UP && (angle_3 < 50)){
    angle_3++;
    delay(20);
  }else if((mode==2) && DOWN && (angle_3 > ANG3)){
    angle_3--;
    delay(20);
  }

  // Second Arm Rotation
  if((mode==3) && UP && (current_angle_4 < max_angle_4)){
    delta_angle_4++;
  }else if((mode==3) && DOWN && (current_angle_4 > min_angle_4)){
    delta_angle_4--;
  }
  
  // Default Setup
  if(System){
    angle_1 = ANG1;
    angle_2 = ANG2;
    angle_3 = ANG3;
    current_angle_4 = ANG4;
    angle_5 = ANG5;
    mode = 0;
    delta_angle_4 = 0;
    currentIndex = 0;
    Dataset = 0;
    myControllers->setColorLED(0, 0, 255);
  }

  // Gripper Rotation
  if(L1){
    angle_1 = 80;;
  }else if(R1){
    angle_1 = 150;
  }

  // Base Rotation
  if(L2 && (angle_5 > 0)){
    angle_5--;
  }else if(R2 && (angle_5 < 150)){
    angle_5++;
  }

  // if(LEFT){
  //   if(current_time - last_time > threshold){
  //     Serial.println(currentIndex);
  //     last_time = current_time;
  //     for (int Play_action=0; Play_action < currentIndex; Play_action++){
  //       Serial.println(String(Play_action) + " Servo Angles: " + String(servoAngles[Play_action]));
  //     }
  //   }
  // }

  angle[0] = angle_1;
  angle[1] = angle_2;
  angle[2] = angle_3;
  angle[3] = current_angle_4;
  angle[4] = angle_5;

  if(mode != 4){
    for(unsigned char i=0; i<5;i++){ // 0 -> 255; 4 bytes = 32 ; 2^32 = 60000++
      myservo[i].write(angle[i]);
    }
    // for(int i=0; i<5;i++){
    //   Serial.print(angle[i]);
    //   Serial.print(" ");
    // }
  }else if(mode == 4){
    myControllers->setColorLED(90,98,180);
    Play(Home);
    myControllers->setRumble(0xc0, 0x80); // force, duration
  }
  delay(10);
}

void setup() {
  for(int i=0; i<5;i++){
    myservo[i].attach(servo_pin[i]);
  }

  Serial.begin(115200);
  // Setup the Bluepad32 callbacks
  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys();
}

void loop() {
  BP32.update();
  if (myControllers && myControllers->isConnected()) {
    GP_Input();
    current_time = millis();
  }
}

void Play(bool STOP){
  for (unsigned char Play_action=0; Play_action < currentIndex; Play_action++){
    action_servo = servoAngles[Play_action] / 1000; 
    action_pos = servoAngles[Play_action] % 1000;
    // Cancel Replay
    if(STOP){
      Serial.println("STOP");
      mode = 0;
      myControllers->setColorLED(0, 0, 255);
      break;
    }
    myservo[action_servo].write(action_pos);
    delay(250);
  }
}

void Record(int ser_angle[],int servoAngles_record[]){
  for(unsigned char i = 4;i > -1;i--){
    servoAngles_record[currentIndex] = ser_angle[i] + (i*1000);
    currentIndex++;
    Serial.println(i);
  }
}