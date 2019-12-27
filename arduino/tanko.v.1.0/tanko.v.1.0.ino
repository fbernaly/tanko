#include <PS2X_lib.h>  //for v1.6

/******************************************************************
 * set pins connected to PS2 controller:
 ******************************************************************/
#define PS2_DAT        12
#define PS2_CMD        11
#define PS2_SEL        10
#define PS2_CLK        13

/******************************************************************
 * set pins connected to motors:
 ******************************************************************/
#define PMWB            6
#define DIRB            7
#define PMWA            9
#define DIRA            8

/******************************************************************
 * set pins connected to leds:
 ******************************************************************/
#define LEFT_LED        5
#define RIGHT_LED       4

PS2X ps2x; // create PS2 Controller Class
int error = 0;
bool leftLed = false;
bool rightLed = false;

void setup(){
  // motors setup
  pinMode(PMWA,OUTPUT);
  pinMode(DIRA,OUTPUT);
  pinMode(PMWB,OUTPUT);
  pinMode(DIRB,OUTPUT);
  digitalWrite(PMWA, LOW);
  digitalWrite(PMWB, LOW);

  // leds setup
  pinMode(LEFT_LED,OUTPUT);
  pinMode(RIGHT_LED,OUTPUT);
  digitalWrite(LEFT_LED, LOW);
  digitalWrite(RIGHT_LED, LOW);
  
  Serial.begin(57600);
  
  delay(500);  //added delay to give wireless ps2 module some time to startup, before configuring it
   
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT);
  
  if(error == 0)
    Serial.println("Found Controller, configured successful "); 
  else if(error == 1)
    Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
  else if(error == 2)
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");
  else if(error == 3)
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");

  if(error != 0)
    return;
  
  byte type = ps2x.readType(); 
  switch(type) {
    case 0:
      Serial.println("Unknown Controller type found");
      break;
    case 1:
      Serial.println("DualShock Controller found");
      break;
    case 2:
      Serial.println("GuitarHero Controller found");
      break;
	case 3:
      Serial.println("Wireless Sony DualShock Controller found");
      break;
   }
   
   Serial.println("Ready to receive commands...");
}

void loop() {
  /* You must Read Gamepad to get new values and set vibration values
     ps2x.read_gamepad(small motor on/off, larger motor strenght from 0-255)
     if you don't enable the rumble, use ps2x.read_gamepad(); with no values
     You should call this at least once a second
   */  
  if(error == 1) //skip loop if no controller found
    return; 
  
  digitalWrite(PMWA, LOW);
  digitalWrite(PMWB, LOW);

  ps2x.read_gamepad(); //read controller and set large motor to spin at 'vibrate' speed
  
  if(ps2x.Button(PSB_START))         //will be TRUE as long as button is pressed
    Serial.println("Start is being held");
  if(ps2x.Button(PSB_SELECT))
    Serial.println("Select is being held");      

  if(ps2x.Button(PSB_PAD_UP)) {      //will be TRUE as long as button is pressed
    Serial.println("UP pressed");
    moveForward();
  } else if(ps2x.Button(PSB_PAD_DOWN)) {
    Serial.println("DOWN pressed");
    moveBackward();
  } else if(ps2x.Button(PSB_PAD_RIGHT)) {
    Serial.println("RIGHT pressed");
    turnRightFowward();
  } else if(ps2x.Button(PSB_PAD_LEFT)) {
    Serial.println("LEFT pressed");
    turnLeftFowward();
  } else if(ps2x.Button(PSB_TRIANGLE)) {
    Serial.println("TRIANGLE pressed");
    rotateRight();
  } else if(ps2x.Button(PSB_CROSS)) {
    Serial.println("CROSS pressed");
    rotateLeft();
  } else if(ps2x.Button(PSB_SQUARE)) {
    Serial.println("SQUARE pressed");
    turnRightBackward();
  } else if(ps2x.Button(PSB_CIRCLE)) {
    Serial.println("CIRCLE pressed");
    turnLeftBackward();
  } else if(ps2x.Button(PSB_L1) || ps2x.Button(PSB_R1)) {
    // L stick controls motor A
    if(ps2x.Button(PSB_L1)) {
      int lx = ps2x.Analog(PSS_LX);
      int ly = ps2x.Analog(PSS_LY);
    
      if (ly < 50) {
        digitalWrite(PMWA, HIGH);
        digitalWrite(DIRA, HIGH);
      } else if (ly > 200) {
        digitalWrite(PMWA, HIGH);
        digitalWrite(DIRA, LOW);
      }
      
      Serial.print("L Stick Values: ");
      Serial.print(lx);
      Serial.print(", ");
      Serial.println(ly);
    } else {
      digitalWrite(PMWA, LOW);
    }
    
    // R stick controls motor B
    if(ps2x.Button(PSB_R1)) {
      int rx = ps2x.Analog(PSS_RX);
      int ry = ps2x.Analog(PSS_RY);
      
      if (ry < 50) {
        digitalWrite(PMWB, HIGH);
        digitalWrite(DIRB, HIGH);
      } else if (ry > 200) {
        digitalWrite(PMWB, HIGH);
        digitalWrite(DIRB, LOW);
      }
     
      Serial.print("R Stick Values: ");
      Serial.print(rx); 
      Serial.print(", ");
      Serial.println(ry);
    } else {
      digitalWrite(PMWB, LOW);
    }
  }
  
  if(ps2x.ButtonPressed(PSB_L2)) {
    Serial.println("L2 just pressed");
    toogleLeftLed();
  }
  
  if(ps2x.ButtonPressed(PSB_R2)) {
    Serial.println("R2 just pressed");
    toogleRightLed();
  }

  delay(50);  
}

void moveForward() {
  digitalWrite(PMWA, HIGH);
  digitalWrite(PMWB, HIGH);
  digitalWrite(DIRA, HIGH);
  digitalWrite(DIRB, HIGH);
}

void moveBackward() {
  digitalWrite(PMWA, HIGH);
  digitalWrite(PMWB, HIGH);
  digitalWrite(DIRA, LOW);
  digitalWrite(DIRB, LOW);
}

void rotateRight() {
  digitalWrite(PMWA, HIGH);
  digitalWrite(PMWB, HIGH);
  digitalWrite(DIRA, HIGH);
  digitalWrite(DIRB, LOW);
}

void rotateLeft() {
  digitalWrite(PMWA, HIGH);
  digitalWrite(PMWB, HIGH);
  digitalWrite(DIRA, LOW);
  digitalWrite(DIRB, HIGH);
}

void turnRightFowward() {
  digitalWrite(PMWA, HIGH);
  digitalWrite(PMWB, LOW);
  digitalWrite(DIRA, HIGH);
  digitalWrite(DIRB, HIGH);
}

void turnLeftFowward() {
  digitalWrite(PMWA, LOW);
  digitalWrite(PMWB, HIGH);
  digitalWrite(DIRA, HIGH);
  digitalWrite(DIRB, HIGH);
}

void turnRightBackward() {
  digitalWrite(PMWA, LOW);
  digitalWrite(PMWB, HIGH);
  digitalWrite(DIRA, HIGH);
  digitalWrite(DIRB, LOW);
}

void turnLeftBackward() {
  digitalWrite(PMWA, HIGH);
  digitalWrite(PMWB, LOW);
  digitalWrite(DIRA, LOW);
  digitalWrite(DIRB, HIGH);
}

void toogleRightLed() {
  rightLed = !rightLed;
  digitalWrite(RIGHT_LED, rightLed == true ? HIGH : LOW);
}

void toogleLeftLed() {
  leftLed = !leftLed;
  digitalWrite(LEFT_LED, leftLed == true ? HIGH : LOW);
}
