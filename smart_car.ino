#include <Servo.h>
#include <IRremote.h>
Servo servo ;


//Pins
const int trigPin = 13;
const int echoPin = 12;

const int servoPin = 10;

const int enAPin = A3;
const int in1Pin = 7;
const int in2Pin = A2;
const int in3Pin = 4;
const int in4Pin = 2;
const int enBPin = A4;

const int receiverPin = A1;

const int redPin= A0;
const int greenPin = 6;
const int  bluePin = 5;

const int buzzer = 1;


int moving = 0;
int MSpeed = 200;
int blinking = 0;
int buzzing = 0;


IRrecv irrecv(receiverPin);
decode_results signals;


//Defs
enum Motor { LEFT, RIGHT };
void go(enum Motor m, int speed)
{
  digitalWrite (m == LEFT ? in1Pin : in3Pin , speed > 0 ? HIGH : LOW );
  digitalWrite (m == LEFT ? in2Pin : in4Pin , speed <= 0 ? HIGH : LOW );
  analogWrite(m == LEFT ? enAPin : enBPin, speed < 0 ? -speed : speed );
}


void testMotors ()
{
  static int speed[8] = { 128, 255, 128, 0 , -128, -255, -128, 0};
  go(RIGHT, 0);
  for (unsigned char i = 0 ; i < 8 ; i++)
    go(LEFT, speed[i]), delay(200);
  
  for (unsigned char i = 0 ; i < 8 ; i++)
    go(RIGHT, speed[i]), delay(200);
}


unsigned int readDistance()
{
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  unsigned long period = pulseIn(echoPin, HIGH);
  return period * 343 / 2000;
}


#define NUM_ANGLES 7
unsigned char sensorAngle[NUM_ANGLES] = {60, 70, 80, 90, 100, 110, 120};
unsigned int distance [NUM_ANGLES];


void readNextDistance()
{
  static unsigned char angleIndex = 0;
  static signed char step = 1;
  distance[angleIndex] = readDistance();
  angleIndex += step ;
  if (angleIndex == NUM_ANGLES - 1) step = -1;
  else if (angleIndex == 0) step = 1;
  servo.write(sensorAngle[angleIndex]);
}


void setColor(int redValue, int greenValue,  int blueValue) {
  analogWrite(redPin, redValue);
  analogWrite(greenPin,  greenValue);
  analogWrite(bluePin, blueValue);
}



//Setup
void setup () {
  irrecv.enableIRIn();

  pinMode(redPin,  OUTPUT);              
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  pinMode(trigPin , OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);
  pinMode(enAPin, OUTPUT);
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(in3Pin, OUTPUT);
  pinMode(in4Pin, OUTPUT);
  pinMode(enBPin, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  servo.attach(servoPin);
  servo.write(90);
  go(LEFT, 0);
  go(RIGHT, 0);

  setColor(255, 0, 0);
  delay(300);
  setColor(0, 0, 0);
  delay(100);
  setColor(255, 0, 0);

  testMotors();

  servo.write(sensorAngle[0]);
  delay(200);
  for (unsigned char i = 0 ; i < NUM_ANGLES ; i ++)
    readNextDistance(), delay (200);
}




void loop () {

  if (irrecv.decode(&signals)) {
      irrecv.resume();
      if (signals.value == 0xFFC23D) moving = moving == 2 ? 0 : 2;
      if (signals.value == 0xFF906F) moving = moving == 1 ? 2 : 1;

      if (signals.value == 0xFFE01F) MSpeed = MSpeed == 10 ? 10 : MSpeed-10;
      if (signals.value == 0xFFA857) MSpeed = MSpeed == 250 ? 250 : MSpeed+10;

      if (signals.value == 0xFF22DD) moving = moving == 3 ? 4 : 3;
      if (signals.value == 0xFF02FD) moving = moving == 5 ? 6 : 5;
  }


  readNextDistance ();
  unsigned char tooClose = 0;
  for (unsigned char i = 0 ; i < NUM_ANGLES ; i++)
    if (distance[i] < 200)
      tooClose = 1;
  
  if (tooClose && moving != 0) {
    go(LEFT, -130);
    go(RIGHT, -250);
    
    if (blinking < 5) setColor(170, 0, 255), blinking++;
    else setColor(0, 0, 0), blinking = 0;
    
    if (buzzing < 5) digitalWrite(buzzer, HIGH), buzzing++;
    else digitalWrite(buzzer, LOW), buzzing = 0;
  } 
  else if (moving == 2) {
    go(LEFT, MSpeed);
    go(RIGHT, MSpeed);
    setColor(0, 255, 0); 
  }
  else if (moving == 1) {
    go(LEFT, -MSpeed);
    go(RIGHT, -MSpeed);

    if (blinking < 5) setColor(0, 0, 255), blinking++;
    else setColor(0, 0, 0), blinking = 0;
    if (buzzing < 5) digitalWrite(buzzer, HIGH), buzzing++;
    else digitalWrite(buzzer, LOW), buzzing = 0;
  }
  else if (moving == 3) {
    go(LEFT, 0);
    go(RIGHT, MSpeed);

    setColor(255, 102, 0);
  }
  else if (moving == 4) {
    go(LEFT, -MSpeed);
    go(RIGHT, MSpeed);

    if (blinking < 5) setColor(255, 102, 0), blinking++;
    else setColor(0, 0, 0), blinking = 0;
  }
  else if (moving == 5) {
    go(LEFT, MSpeed);
    go(RIGHT, 0);

    setColor(255, 255, 0);
  }
  else if (moving == 6) {
    go(LEFT, MSpeed);
    go(RIGHT, -MSpeed);

    if (blinking < 5) setColor(255, 255, 0), blinking++;
    else setColor(0, 0, 0), blinking = 0;
  }
  else {
    go(LEFT, 0);
    go(RIGHT, 0);

    setColor(255, 0, 0);
  }


  if (moving != 1 && (!tooClose && moving != 0)) digitalWrite(buzzer, LOW);

  delay (50);
}
