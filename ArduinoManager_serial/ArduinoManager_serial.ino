#include <Servo.h>

//#include <WiFiS3.h>

//#include <ArduinoOTA.h>
//#include "Arduino_LED_Matrix.h"
//#include "Matrix.h"
#include <SoftwareSerial.h>
#include <Wire.h>

//Blue - 9

#define rstPin 4

struct Coordinates {
  float xValue;
  float yValue;
};

#define GRBL_BAUD_RATE 115200

Servo dice_servo;  // create servo object to control a servo
Servo magnet_servo;
int magnet_servo_last_angle = 0;

int PIN_MAGNET_SERVO = 53;
int PIN_MAGNET_SIGNAL = 50;
int PIN_DICE = 41;

int PIN_BUTTON = 44;

int PIN_COLOR_RED = 8;
int PIN_COLOR_GREEN = 9;
int PIN_COLOR_BLUE = 10;


int magnet_servo_target_angle = 0;   //to which angle the servo needs to rotate to
//int magnet_speed = 0; //1 --> slowly 2 --> rapidly
float magnet_servo_start_rotation = 0;  //from which x or y to start rotate the magnet servo
int start_rotate_X_or_Y = 0; //whether to start rotate the servo when X is higher/lower than magnet_servo_start_rotation or Y 0-->X, 1-->Y
int start_rotate_lower_or_higher= 0; //whether to start rotate when X/Y is lower or higher than magnet_servo_start_rotation 0-->lower, y-->higher
bool movement_started = false;

bool skip_home = false;

String current_command;
int num_of_sub_commands = 0;
int current_command_line = 0; //one based!!!!

enum CurrentStateEnum   { IDLE = 0,
                          MOVE = 1,
                          ROTATE = 2,
                          WAIT_FOR_PLAYER = 8
                         };

int current_state = IDLE;
/*
  Serial - PC
  Serial1 - 115200 - GRBL
  Serial2 - 9600 - RGB
  Serial3 - 9600 - debug
*/



void SendStringToPC(String str)
{
  Serial.println(str);
  delay (10);
}



String GetSectionByIndex(String inputString, int index, char delimiter);
void SplitString(String inputString, char delimiter, String outputArray[], int outputSize);
void EstablishWiFiConnection();
void CloseWiFiConnection();
void SendStringToPC(String str);
bool ReceiveStringFromPC();
int ExtractNumber(String input);
void CheckIfClientConnected();
int CountCharacterOccurrences(String inputString, char targetChar);
String GetSectionBySubstring(String inputString, String subs, char delimiter = '^');
Coordinates ParseMPosString(const char* inputString);
String RemovePercent(const String &original);


int matrix_last_change = 0; //when was the last change
int matrix_icon_num = 0;

bool ReceiveStringFromPC()
{
  if (!Serial.available())
    return false;
  //current_command = Serial.readStringUntil('\n');
  current_command = Serial.readString();
  num_of_sub_commands = CountCharacterOccurrences(current_command,'^')+1;
  DebugSerialPrintln("received command (current_command):" + current_command);
  return true;
}

void SetGreenColor()
{
  SetLedColor(0,200,0);
}


void SetNoColor()
{
  SetLedColor(0,0,0);
}


void SetOrangeColor()
{
  SetLedColor(255,128,0);
}

void SetLedColor(int red,int green, int blue)
{
  DebugSerialPrintln("color: " + String (red) + " " + String(green) + " " + String (blue));
  analogWrite(PIN_COLOR_RED,red);
  analogWrite(PIN_COLOR_GREEN,green);
  analogWrite(PIN_COLOR_BLUE,blue);
}


//checks if it's a good time to start the rotation, during movement
bool ShouldPerformRotate()
{
  
  if (!(current_state & MOVE))
  {
    return true;
  }

  if (movement_started)
  {
    return true;
  }

  Coordinates coords = GetGRBLPosition();
  //SerialPrintln("X:"+String(coords.xValue)+"Y:"+String(coords.yValue));
  if (coords.xValue < 0)
    return false;
  //SerialPrintln("start_rotate_X_or_Y" + String(start_rotate_X_or_Y));
  if (start_rotate_X_or_Y==0) //X
  {
    if (start_rotate_lower_or_higher==0) //lower than
    {
        if (coords.xValue < magnet_servo_start_rotation)
        {
          movement_started = true;
          DebugSerialPrintln("start!1");
          return true;
        }
    }
    else //higher
    {
        if (coords.xValue > magnet_servo_start_rotation)
        {
          movement_started = true;
          DebugSerialPrintln("start!2");
          return true;
        }
    }
  }
  else
  { //Y
    if (start_rotate_lower_or_higher==0) //lower than
    {
        if (coords.yValue < magnet_servo_start_rotation)
        {
          movement_started = true;
          DebugSerialPrintln("start!3");
          return true;
        }

    }
    else //higher
    {
        if (coords.yValue > magnet_servo_start_rotation)
        {
          movement_started = true;
          DebugSerialPrintln("start!4");
          return true;
        }
    }
  }
  //SerialPrintln("not yet...");
  return false;
}

long last_rotated_time = 0;
void RotateMagnetServo()
{
  int magnet_servo_delay = 5;

  if (millis() < last_rotated_time + magnet_servo_delay)
    return; //not enough time has passed, nothing to do
  else
  { //do one small servo step 

    //Have we reached the target angle?
    if (magnet_servo_target_angle == magnet_servo_last_angle)
    {
      current_state &= ~ROTATE;
      DebugSerialPrintln("rotate completed");
      DebugSerialPrintln(String(magnet_servo_target_angle));
    }
    else
    {

      if (ShouldPerformRotate())
      {
        magnet_servo.write(magnet_servo_target_angle);  //XXXXXXXXXXXXXXXXXXXXXXXXXXX remove comment
        DebugSerialPrintln("!!!rotated to:"+String(magnet_servo_last_angle));
        magnet_servo_last_angle = magnet_servo_target_angle;
        last_rotated_time = millis();
        delay(1000);
      }
    }
  }
   
}

/*
  supported commands:

  Roll - roll the dices
  MagnetOff - turn off the magnet
  MagnetOn - turn on the magnet

*/

void DebugSerialPrintln(String line)
{
  Serial.println(line);
  Serial3.println(line);
}


#define CHUNK_SIZE 1000      // Define the chunk size
#define SLAVE_ADDRESS 0x10 // Change this to your slave address
#define MAX_STRING_LENGTH 50
char receivedMessage[MAX_STRING_LENGTH] = "";

String padToChars(int pad,String input) {
  while (input.length() < pad) {
    input += ' ';  // Append spaces until the string is 80 characters long
  }
  return input;
}


void OriginalSendLineToRGB(String message) 
{
  // Calculate the number of chunks
  DebugSerialPrintln("Sending to RGB:" + message);
  Serial2.println(message);
  delay (30);
  //SendStringToPC("OK");
  //DebugSerialPrintln("Sent OK to PC" );
}

//full duplex
void SendLineToRGB(String message) 
{
// Step 1: Serial2 sends 'Req' to Serial1
  String initialMessage = "Req";
  Serial2.println(initialMessage);
  Serial.print("Sent: ");
  Serial.println(initialMessage);

  // Step 3: Serial2 waits for the 'ok' message from Serial1
  String receivedMessage = "";
  while (receivedMessage.indexOf("ok") == -1) {
    if (Serial2.available() > 0) {
      receivedMessage = Serial2.readStringUntil('\n');
      Serial.print("Received: ");
      Serial.println(receivedMessage);
    }
  }

  Serial.print ("Got OK");
  // Step 4: Serial2 sends 'Hello!!!' after receiving 'ok'
  
  Serial2.println(message);
  Serial.print("Sent: ");
  Serial.println(message);
}

void MoveTo(String str)
{
   Serial1.println(str);
   DebugSerialPrintln(str);
}

void setup2() {
  // Initialize Serial for communication with the PC

  magnet_servo.attach(PIN_MAGNET_SERVO);
  // Initialize Serial1 for communication with the GRBL controller

  Serial.begin(9600);
  Serial1.begin(GRBL_BAUD_RATE);
  Serial2.begin(9600);  //RGB
  Serial3.begin(9600);  //debug



  // Give GRBL some time to initialize
  delay(2000);
  Home();
  // Send the homing command to GRBL
  //Serial.println("before move");
  /*for (int i = 0; i < 1 ; i++)
  {
    MoveTo("G0 x1 y1");

    wait_for_idle();
    magnet_servo.write(10);  

    MoveTo("G0 x3 y3");
    magnet_servo.write(30);  
    wait_for_idle();
    
    MoveTo("G0 x0 y0");
    magnet_servo.write(20);  
    wait_for_idle();
    
  }*/
  // Notify the user that the homing command has been sent
}

void CommunicateWithRGB()
{
  Serial2.println ("Req\n");
  DebugSerialPrintln("sent Req");
  delay (10);
  while (true)
  {
      if (Serial2.available() > 0) {  // Check if data is available to read
        int receivedByte = Serial.read();  // Read the byte from the serial buffer
        Serial.print("Received byte: ");
        Serial.println((char)receivedByte);  // Print the received byte
      }
  }

}

//Blue - 9

void setup() 
{
  Serial.begin(9600);
  Serial1.begin(115200);  //GRBL
  Serial2.begin(9600);  //RGB
  Serial3.begin(9600);  //debug

  
  digitalWrite(rstPin, HIGH);
  

  pinMode(PIN_COLOR_RED, OUTPUT);
  pinMode(PIN_COLOR_GREEN, OUTPUT);
  pinMode(PIN_COLOR_BLUE, OUTPUT);

  
  //CommunicateWithRGB();

  //delay (100000);



  SendLineToRGB("JUST_TEXT:HELLO!:0:NONE:0:0:0");

  
  magnet_servo.attach(PIN_MAGNET_SERVO);
  pinMode(PIN_MAGNET_SIGNAL, OUTPUT);
  
  pinMode(PIN_BUTTON,INPUT);
  pinMode(PIN_DICE, OUTPUT);
  SetOrangeColor();
  dice_servo.write(90);                  // sets the servo position according to the scaled value
  magnet_servo.write(1);                  // sets the servo position according to the scaled value
  magnet_servo_last_angle = 10;

  digitalWrite(PIN_MAGNET_SIGNAL,LOW);
  
  SetNoColor();
  DebugSerialPrintln("-----------DEBUG STARTED---------------");
  SendLineToRGB("JUST_TEXT:CONNECTING:0:NONE:0:0:0");
  SendLineToRGB("STARS:CONNECTED!:0:NONE:0:0:0");

  //Home();
  
  DebugSerialPrintln("Setup ended");
}

void WaitForPlayer()
{
  current_state = WAIT_FOR_PLAYER;
  SetGreenColor();
}

void RollDice()
{
  DebugSerialPrintln("start roll dice");
  digitalWrite(PIN_DICE,HIGH);
  delay(800);
  digitalWrite(PIN_DICE,LOW);
  //delay(500);
  DebugSerialPrintln("end roll dice");
}


String ReadLineFromGRBL(bool handle_alarm = true)
{
  String received_data;
  if (Serial1.available()) 
  {
    received_data = Serial1.readStringUntil('\n'); // Read a line of text until a newline character
    // Check if any data was received
    if (received_data.length() > 0) 
    {
      DebugSerialPrintln(String("DATA:") + received_data);
      if (handle_alarm && (received_data.indexOf("Alarm") >=0) || (received_data.indexOf("ALARM") >=0))
      {
        
        HandleAlarm(received_data);
        return "ALARM";
      }
      return received_data;
    }
  }
  return String();
}

void wait_for_idle()
{
  wait_for("<Idl");
}

void wait_for_ok()
{
  wait_for("ok");
}

void wait_for(String str)
{
  bool done = false;
  while (!done)
  {
    Serial1.write("?\n");
    //DebugSerialPrintln("writing ? to serial");
    if (Serial1.available())
    {
      String ret_val = ReadLineFromGRBL();
      DebugSerialPrintln("got input from GRBL:" + ret_val);
      if (ret_val.startsWith(str))
      {        
        Serial.println("got idle from GRBL:" + ret_val);
        done = true;
      }        
    }
  }
  CleanupGRBLBuffer();
}

long last_pos_check = 0;
// ?<Alarm|MPos:0.000,0.000,0.000|FS:0,0|Pn:P>
Coordinates GetGRBLPosition()
{
  //SerialPrintln("Checking GRBL Position");
  int interval = 200;  //check every 200ms
  Coordinates coords;
  coords.xValue = -1;
  coords.yValue = -1;
  
  if (millis() < last_pos_check + interval)
    return coords;

  Serial1.write("?\n");
  if (Serial1.available())
  {
    String pos_str = ReadLineFromGRBL(false);
    //SerialPrintln(pos_str);
    String pos_section = GetSectionBySubstring(pos_str,"WPos:",'|');
    if (CountCharacterOccurrences(pos_section,',') == 2) //if it seems the line is complete
    {
      float x,y;
      if(ParseMPosString(pos_section.c_str(),x,y))
      {
        coords.xValue = x;
        coords.yValue = y;
      }
      return coords;
    }
  }
  

  return coords;
}

/*
void RotateMagnetServoTo(int new_angle)
{
  int delta = 1;
  int delay_ms = 4;
  if (new_angle < magnet_servo_last_angle)
  {
   for (int cur_angle = magnet_servo_last_angle ;  cur_angle >= new_angle;cur_angle -=delta)
    {
      magnet_servo.write(cur_angle);
      delay (delay_ms);
    }   
  }
  else
  {
   for (int cur_angle = magnet_servo_last_angle ; cur_angle <= new_angle;cur_angle +=delta)
    {
      magnet_servo.write(cur_angle);
      delay (delay_ms);
    }  
  }
  magnet_servo_last_angle = new_angle;
}*/

/*
void RotateMagnetServo(String str)
{
  SerialPrintln("about to rotate the magnet servo");
  int pos = ExtractNumber(str);
  SerialPrintln("about to rotate the magnet servo to: " + String(pos));
  SerialPrintln("Magnet rotate to:" + String(pos));
  RotateMagnetServoTo(pos);  
  SerialPrintln("Magnet rotated");
}*/

void WaitForOKFromGRBL()
{
    bool keep_checking = true;
    while (keep_checking)
    {
      String ret_val = ReadLineFromGRBL(false);
      if (ret_val != "")
        DebugSerialPrintln(ret_val);
      if (ret_val.startsWith("ok"))
      {
        DebugSerialPrintln("got OK from GRBL");
        return;
      }
    }
}


int UpdateGRBLState(bool handle_alarm = true)
{
  bool completed = false;

  //while (!completed)
  if (current_state & (MOVE) )
  {
    Serial1.write("?\n");

    if (Serial1.available())
    {
      String ret_val = ReadLineFromGRBL(handle_alarm);
      if (ret_val.startsWith("<Idle"))
      {        
        DebugSerialPrintln ("got idle from GRBL:" + ret_val);
        completed = true;
        CleanupGRBLBuffer();

        if (current_state & ~ROTATE)  //dont turn off move until rotate is done, to make sure rotate is fully done
        {
          DebugSerialPrintln("clearing current state");
          current_state &= ~(MOVE);
          DebugSerialPrintln("Current state = " + String(current_state));
        }

      }
      if (ret_val == "ALARM")
      {
        DebugSerialPrintln ("Encountered Alarm while waiting for GRBL");
        return -1;
      }
    }
  }
 
  //need to cleanup the rest of the messages, before the next call:
  
  //CleanupGRBLBuffer();
  //SerialPrintln("GRBL is Idle");
  return 0;
}

void Home()
{
  if (skip_home)
    return;
  DebugSerialPrintln("Home is starting!!!!");
  
  DebugSerialPrintln("Turnning on Homing");
  Serial1.println ("$21=1");
  wait_for_ok();
  Serial1.println ("$22=1");
  wait_for_ok();

  DebugSerialPrintln("Sending Soft Reset");
  char myChar = 24; // ASCII value of 24
  Serial1.write(myChar);
  wait_for_ok();
  
  DebugSerialPrintln("Sending $X");
  Serial1.println ("$X");
  wait_for_ok();
  
  DebugSerialPrintln("Sending $H");
  Serial1.println ("$H");
  wait_for_ok();
  DebugSerialPrintln("G92 X0 Y0 Z0");
  Serial1.println ("G92 X0 Y0 Z0");
  wait_for_ok();
  delay(10);

  DebugSerialPrintln("Turning off homing");
  Serial1.println("$22=0");
  wait_for_ok();
  Serial1.println("$21=0");         
  wait_for_ok();
}

void MagnetOff()
{
  DebugSerialPrintln("turning magnet off");
  digitalWrite(PIN_MAGNET_SIGNAL,LOW);
  delay(100);
}

void MagnetOn()
{
  DebugSerialPrintln("turning magnet on");
  digitalWrite(PIN_MAGNET_SIGNAL,HIGH);
  delay(300);
}

/*
String GetGRBLCommand(String str)
{
  int colonPosition = str.indexOf(':');

  // Check if "GRBL" is found at the beginning of the string
  if (colonPosition != -1 && str.startsWith("GRBL")) 
  {
    // Use substring to get the part of the original string after ":"
    String substring = str.substring(colonPosition + 1);
    SerialPrintln("extracted: " + substring);
    return substring;
  }
  else
    return String();
}*/

bool CleanupGRBLBuffer()
{
  bool ret_val = true;
  while (Serial1.available()) 
  {
    String received_data = Serial1.readStringUntil('\n');
    //SerialPrintln(received_data);
    if (ret_val && (received_data.indexOf("Alarm") >=0) || (received_data.indexOf("ALARM") >=0))
    {
      DebugSerialPrintln("Encountered alarm during buffer cleanup");
      HandleAlarm(received_data);
      ret_val = false;
    }
    //SerialPrintln("in loop, from GRBL" + received_data);
  }
  return ret_val;
}

void HandleAlarm(String received_data)
{
  //matrix.loadFrame(alarm[0]);
  DebugSerialPrintln("ALARM: " + received_data);
  delay (100);
  //digitalWrite(rstPin, LOW);  //if a reset is required - use this one:
  //Home();
}


void ExecuteGRBLCommand(String GRBLCommand)
{
  Serial1.println(GRBLCommand);
  DebugSerialPrintln("Sent command to GRBL:" + GRBLCommand);
  //ret_val = WaitUntilGRBLIdle();
}


bool CheckAlarm()
{
  Serial1.write("?\n");
  delay(40);
  return CleanupGRBLBuffer();
}



bool in_alarm = false;

void CheckAlarmRequest()
{
  if (in_alarm)
  {
      SendStringToPC("ALARM");
  }
  else
  {
    SendStringToPC("OK");
  }
}



//%Magnet:30:X:150%
void GetMoveAndRotateParams(String move_command, String magnet_line)
{
  Coordinates coords = ExtractXYValues(move_command);
  String section_5 = GetSectionByIndex(magnet_line,5,':');

  //DebugSerialPrintln(String("section_5 = ") + section_5);
  

  magnet_servo_start_rotation = section_5.toFloat();  
  start_rotate_X_or_Y = GetSectionByIndex(magnet_line,4,':') == "X" ? 0:1;

  if (start_rotate_X_or_Y == 0)
  {
    start_rotate_lower_or_higher = coords.xValue > magnet_servo_start_rotation ? 1:0;
  }
  else
  {
    start_rotate_lower_or_higher = coords.yValue > magnet_servo_start_rotation ? 1:0;
  }
  /*
  DebugSerialPrintln(String("coords.xValue = ")+String(coords.xValue));
  
  DebugSerialPrintln(String("magnet_servo_start_rotation2 = ")+String(magnet_servo_start_rotation));
  
  DebugSerialPrintln(String("start_rotate_X_or_Y = ")+String(start_rotate_X_or_Y));
  
  DebugSerialPrintln(String("start_rotate_lower_or_higher = ")+String(start_rotate_lower_or_higher));*/
  
}

void ReadCommandLine(int command_line)
{
  String sub_command = GetSectionByIndex(current_command,command_line,'^');
  sub_command = RemovePercent (sub_command);

  String move_command;
  DebugSerialPrintln ("subcommand("+String(command_line)+"):" + sub_command);

  String move_line = GetSectionBySubstring(sub_command, "Move:",'%');
  if (move_line != "")
  {
    current_state = MOVE;
    move_command = GetSectionByIndex(move_line,2,':');
    ExecuteGRBLCommand(move_command);
  }

  String magnet_line = GetSectionBySubstring(sub_command, "Magnet:",'%');
  
  if (magnet_line != "")
  {
   // String magnet_speed_str = GetSectionByIndex(magnet_line,2,':');
    String target_angle_str = GetSectionByIndex(magnet_line,3,':');
    
    //magnet_speed = magnet_speed_str.toInt();
    //DebugSerialPrintln("magnet speed!!!!!!!!!!!!!!" + String (magnet_speed));

    magnet_servo_target_angle = target_angle_str.toInt();
  
    current_state |= ROTATE;
    
    movement_started = false;

    if (CountCharacterOccurrences(magnet_line,':') > 1)
    {
        GetMoveAndRotateParams(move_command, magnet_line);
    }
  }
  if (sub_command.indexOf("MagnetOn") >= 0)
    MagnetOn();
  if (sub_command.indexOf("MagnetOff") >= 0)
    MagnetOff();
  if (sub_command.indexOf("Home") == 0)
    Home();
  if (sub_command.indexOf("Roll") == 0)
    RollDice();
  if (sub_command.indexOf("RGB") >= 0)
  {
    int colonIndex = sub_command.indexOf(':');
    String rgb_command = sub_command.substring(colonIndex + 1);
    SendLineToRGB(rgb_command);
  }
  if (sub_command.indexOf("WaitForPlayer") >= 0)
    WaitForPlayer();
}

void HandleWaitForPlayer()
{
  //DebugSerialPrintln("Handle wait for player");
  int buttonState = digitalRead(PIN_BUTTON);
  if (buttonState == HIGH) 
  {
    DebugSerialPrintln("Button Pressed");
    current_state = IDLE;
    current_command = "";
    current_command_line = 0;
    SetNoColor();
    SendStringToPC("OK");
    DebugSerialPrintln("sent OK1");
  }
  else
  {
    //SerialPrintln("button is low");
  }
}

int last_state= 0;

void loop2() { //dummy
  // Check if there is data available from GRBL
  if (current_command == "" && current_state == IDLE)   //in idle, check if new command arrived 
  {   
    //DebugSerialPrintln("check if new str is available " + String(current_state));
    if (ReceiveStringFromPC())
    { //if a new line received

        current_command_line =0;
        magnet_servo_start_rotation = 0;
    }
  }
  if (current_command != "" && current_state == IDLE && current_command_line == num_of_sub_commands)  //reached the end of the command
  { //once the command is done - send that everything is ok, ready for next command
    DebugSerialPrintln("Command is done, ready for next command");
    SendStringToPC("OK");
    current_command = "";
    current_command_line = 0;
    DebugSerialPrintln("sent OK2");

  }
  if (current_command != "" && (current_state == IDLE) && current_command_line < num_of_sub_commands)  //if there are still lines to execute
  {
      
      current_command_line +=1;
      DebugSerialPrintln("Execute line number:" + String(current_command_line));
      ReadCommandLine(current_command_line);
      delay(7);
      return;
  }

  if (current_state & (MOVE))
  {
    //SerialPrintln("Current State = " + String (current_state));
    UpdateGRBLState();
  }
  //continue to handle current line
  if (current_state & ROTATE)
  {
    RotateMagnetServo();
  }
}

void TestRGB()
{
  while (true)
  {
    SendLineToRGB("JUST_TEXT:CONNECTING:0:NONE:0:1:2");
    delay (4000);
    SendLineToRGB("STARS:CONNECTED!:0:NONE:0:3:4");
    delay (4000);
    SendLineToRGB("RAND_DOTS:CONNECTED!:ALON:NONE:0:3:4");
    delay (4000);
    SendLineToRGB("SNAKE:CONNECTED!:ALON fds dsaf dsf dsa fds111122:NONE:0:5:6");
    delay (4000);
    SendLineToRGB("BOUNCING:CONNECTED!:ALON ALON AALON ALON:NONE:0:5:6");
    delay (4000);
    SendLineToRGB("RAND_DOTS:BLACK:0:The board is patiently waiting your next move!:0:4:3");
    delay (4000);
    SendLineToRGB("CHECKERS:BLACK:0:Answer the call!:0:2:3");
    delay (4000);
    SendLineToRGB("BOUNCING:BLACK:0:Your turn to shine - make it count!:0:1:6");
    delay (4000);
    SendLineToRGB("RAND_DOTS:BLACK:0:Take your time, we're not going anywhere!:0:2:2");
    delay (4000);
    SendLineToRGB("SNAKE:BLACK:0:Roll and rock!:0:4:3");
    delay (4000);
    SendLineToRGB("STARS:BLACK:0:Your move, backgammon master!:0:6:3");
    delay (4000);
    SendLineToRGB("CHECKERS:BLACK:0:Think you can beat the machine's next move?:0:5:6");
    delay (4000);
    SendLineToRGB("CHECKERS:BLACK:0:Time to show the computer's boss - your move:0:3:3");
    delay (4000);
    SendLineToRGB("CHECKERS:BLACK:0:Yalla!:0:5:5");
    delay (4000);
    SendLineToRGB("CHECKERS:BLACK:0:The game's in your hands now - what's your play?:0:2:2");
    delay (4000);
  }
}

void loop() //original loop
{

  //TestRGB();

  //CheckIfClientConnected();
  
  if (current_state == WAIT_FOR_PLAYER)
  {
    HandleWaitForPlayer();
  }

  if (current_command == "" && current_state == IDLE)   //in idle, check if new command arrived 
  {   
    //DebugSerialPrintln("check if new str is available " + String(current_state));
    if (ReceiveStringFromPC())
    { //if a new line received

        current_command_line =0;
        magnet_servo_start_rotation = 0;
    }
  }

  if (current_command != "" && current_state == IDLE && current_command_line == num_of_sub_commands)  //reached the end of the command
  { //once the command is done - send that everything is ok, ready for next command
    DebugSerialPrintln("Command is done, ready for next command");
    SendStringToPC("OK");
    current_command = "";
    current_command_line = 0;
    DebugSerialPrintln("sent OK2");

  }

  if (current_command != "" && (current_state == IDLE) && current_command_line < num_of_sub_commands)  //if there are still lines to execute
  {
      
      current_command_line +=1;
      DebugSerialPrintln("Execute line number:" + String(current_command_line));
      ReadCommandLine(current_command_line);
      delay(7);
      return;
  }
  
  //continue to handle current line
  if (current_state & ROTATE)
  {
    RotateMagnetServo();
  }
  if (current_state & (MOVE))
  {
    //SerialPrintln("Current State = " + String (current_state));
    UpdateGRBLState();
  }
  delay(7);
}


