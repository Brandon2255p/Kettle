#include <Arduino.h>
#include <Servo.h>


int PinLightSensor = 2;
int PinPumpPWM = 5;
int PinCupSensor = A0;
int PinLed = 12;

Servo servoBag;
Servo servoPour;

volatile bool RunDrop = true;
volatile bool Pumping = true;
volatile bool Pouring = true;

String command = "";         // a String to hold incoming data

bool commandComplete = false;  // whether the string is complete
void serialEvent();
void HandleInput();
bool signalDrop = false;
bool signalPump = false;
bool signalBoil = false;
bool signalPour = false;

void StopTheDrop()
{
    RunDrop = false;
}

void setup() {
    Serial.println("Starting!!");
    command.reserve(20);
    Serial.begin(9600);
    servoBag.attach(9);
    servoPour.attach(7);
    servoBag.writeMicroseconds(1200);                  // sets the servo position according to the scaled value
    pinMode(PinLightSensor, INPUT_PULLUP);
    pinMode(PinCupSensor, INPUT_PULLUP);
    pinMode(PinLed, OUTPUT);
    digitalWrite(PinLed, LOW);
    attachInterrupt(digitalPinToInterrupt(PinLightSensor), StopTheDrop, RISING);
}

void DropBag()
{
    digitalWrite(PinLed, HIGH);
    delay(200);
    RunDrop = true;
    Serial.println("Dropping a bag...");
    auto StartTime_ms = millis();
    unsigned long TimeLimit_ms = 15000;
    bool TimedOut = false;
    while(RunDrop)
    {
        signalDrop = false;
        serialEvent();
        HandleInput();
        TimedOut = millis() - StartTime_ms > TimeLimit_ms;
        if(TimedOut)
        {
            Serial.println("Timeout!");
            break;
        }
        servoBag.writeMicroseconds(1290);
        if(!RunDrop)
            break;
        delay(300);
        servoBag.writeMicroseconds(1150);
        if(!RunDrop)
            break;
        delay(600);
    }
    signalPump = !TimedOut;
    servoBag.writeMicroseconds(1200);                  // sets the servo position according to the scaled value
    digitalWrite(PinLed, LOW);
}

void PumpWater()
{
    signalPump = false;
    Pumping = true;
    Serial.println("Pumping water...");
    servoPour.write(170);
    auto StartTime_ms = millis();
    unsigned long TimeLimit_ms = 5000;
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
            signalBoil = true;
            Serial.println("Timeout!");
            break;
        }
    }
    digitalWrite(PinPumpPWM, 0);
}

void PourWater()
{
    Pouring = true;
    Serial.println("Pouring water...");
    servoPour.write(5);
    signalPour = false;
}



void loop() {
    HandleInput();
    if(signalDrop)
        DropBag();
    if(signalPump)
        PumpWater();
    if(signalPour)
        PourWater();

    if(signalBoil)
    {
        Serial.println("cmdboil");
        signalBoil = false;
    }

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
        commandComplete = true;
        return;
    }
    command += inChar;
  }
}

void HandleInput()
{
    if (commandComplete) {
        Serial.print("Echo:");
        Serial.println(command);
        if(command.charAt(command.length() - 1) == '\r')
        {
            Serial.println("WINDOWS END LINE");
            command.remove(command.length() - 1);
        }

        if(command.equals("cmdx"))
        {
            Serial.println("CANCELLING");
            RunDrop = false;
            Pumping = false;
            Pouring = false;
            servoPour.write(175);
        }
        else if(command.equals("cmddrop"))
        {
            Serial.println("Dropping");
            signalDrop = true;
        }
        else if(command.equals("cmdpump"))
        {
            Serial.println("Pumping");
            signalPump = true;
        }
        else if(command.equals("cmdpour"))
        {
            Serial.println("Pouring");
            signalPour = true;
        }
        else
        {
            Serial.println("UNKNOWN");
            // int servoValue = command.toInt();
            // if(servoValue){
            //     Serial.print("SERVO:");
            //     Serial.println(servoValue);
            //     servoBag.writeMicroseconds(servoValue);
            // }
            // else
            //     servoBag.writeMicroseconds(1200);
        }
        commandComplete = false;
        command = "";
    }
}
