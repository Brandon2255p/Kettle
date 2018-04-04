#include <Arduino.h>
#include <Servo.h>


int PinLightSensor = 2;
int PinPumpPWM = 5;
int PinCupSensor = A0;
int PinCompleteSignal = 10;

Servo myservo;

volatile bool RunDrop = true;
volatile bool Pumping = true;

String command = "";         // a String to hold incoming data

bool commandComplete = false;  // whether the string is complete
void serialEvent();
void HandleInput();
bool signalDrop = false;
bool signalPump = false;

void StopTheDrop()
{
    RunDrop = false;
    Serial.println("S");
    signalPump = true;
}

void setup() {
    Serial.println("Starting!!");
    command.reserve(20);
    Serial.begin(9600);
    myservo.attach(9);
    myservo.writeMicroseconds(1200);                  // sets the servo position according to the scaled value
    pinMode(PinLightSensor, INPUT_PULLUP);
    pinMode(PinCupSensor, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PinLightSensor), StopTheDrop, RISING);
}

void DropBag()
{
    RunDrop = true;
    Serial.println("Dropping a bag...");
    auto StartTime_ms = millis();
    unsigned long TimeLimit_ms = 15000;

    while(RunDrop)
    {
        signalDrop = false;
        serialEvent();
        HandleInput();
        bool TimedOut = millis() - StartTime_ms > TimeLimit_ms;
        if(TimedOut)
        {
            Serial.println("Timeout!");
            break;
        }
        myservo.writeMicroseconds(1290);
        if(!RunDrop)
            break;
        delay(300);
        myservo.writeMicroseconds(1150);
        if(!RunDrop)
            break;
        delay(600);
    }
    myservo.writeMicroseconds(1200);                  // sets the servo position according to the scaled value
}

void PumpWater()
{
    signalPump = false;
    Pumping = true;
    Serial.println("Pumping water...");
    auto StartTime_ms = millis();
    unsigned long TimeLimit_ms = 15000;
    digitalWrite(PinPumpPWM, HIGH);
    while(Pumping)
    {
        serialEvent();
        HandleInput();
        delay(300);
        bool TimedOut = millis() - StartTime_ms > TimeLimit_ms;
        if(TimedOut)
        {
            Pumping = false;
            Serial.println("Timeout!");
            break;
        }
    }
    digitalWrite(PinPumpPWM, 0);
}

void loop() {
    HandleInput();
    if(signalPump)
        PumpWater();
    if(signalDrop)
        DropBag();

    if(digitalRead(PinCupSensor) == 0)
    {
        signalDrop = true;
    }
    delay(10);
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
        Serial.println("");
        commandComplete = true;
        return;
    }
    Serial.print(inChar, HEX);
    command += inChar;
  }
}

void HandleInput()
{
    if (commandComplete) {
        Serial.print("Echo:");
        Serial.println(command);
        if(command.equalsIgnoreCase("X"))
        {
            Serial.println("CANCELLING");
            RunDrop = false;
            Pumping = false;
        }
        else if(command.equals("drop"))
        {
            signalDrop = true;
        }
        else if(command.equals("pump"))
        {
            signalPump = true;
        }
        else
        {
            int servoValue = command.toInt();
            if(servoValue){
                Serial.print("SERVO:");
                Serial.println(servoValue);
                myservo.writeMicroseconds(servoValue);
            }
            else
                myservo.writeMicroseconds(1200);
        }
        commandComplete = false;
        command = "";
    }
}
