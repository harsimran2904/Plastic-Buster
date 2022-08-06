// Main arduino code - Primarly for movement

#include

const int frontEchoPin = 9;
const int frontTriggerPin = 13;
const int frontBackEchoPin = 12; 
const int frontBackTriggerPin = 11; 

const int leftForward = 2;
const int leftBackward = 3;
const int rightForward = 4; 
const int rightBackward = 5;


int ir_one = A1; 
int ir_two = 8; 
int is_obstacle_ir_one = LOW;
int is_obstacle_ir_two = LOW;

int buzzer = 7;


int send_data;

long randNumber;

volatile float maxtDistance = 17.00;
volatile float maxtBackDistance = 36.00;
volatile float frontDuration, frontDistanceCm, frontBackDuration, frontBackDistanceCm;
long rnum;

void setup() {
  Wire.begin(9);
  Wire.onReceive(receiveEvent);
  
  pinMode(ir_one, INPUT);
  pinMode(ir_two, INPUT);
  
  Serial.begin(9600);
  
  pinMode(frontTriggerPin, OUTPUT);
  pinMode(frontEchoPin, INPUT);
  pinMode(frontBackTriggerPin, OUTPUT);
  pinMode(frontBackEchoPin, INPUT);

  pinMode(leftForward , OUTPUT);
  pinMode(leftBackward , OUTPUT);
  pinMode(rightForward , OUTPUT);
  pinMode(rightBackward , OUTPUT);

  randomSeed(analogRead(0));
}

 void checkFrontDistance() {
    digitalWrite(frontTriggerPin, LOW);  
    delayMicroseconds(4);
    digitalWrite(frontTriggerPin, HIGH);  
    delayMicroseconds(10);
    digitalWrite(frontTriggerPin, LOW);
    frontDuration = pulseIn(frontEchoPin, HIGH); 
    frontDistanceCm = frontDuration * 10 / 292 / 2; 
    frontDistanceCm = round(frontDistanceCm);
    Serial.print("Distance_HC-SR04: ");
    Serial.print(frontDistanceCm);
    Serial.println(" cm");
  }

  void checkFrontBackDistance() {
    digitalWrite(frontBackTriggerPin, LOW);  
    delayMicroseconds(4);
    digitalWrite(frontBackTriggerPin, HIGH);  
    delayMicroseconds(10);
    digitalWrite(frontBackTriggerPin, LOW);
    frontBackDuration = pulseIn(frontBackEchoPin, HIGH); 
    frontBackDistanceCm = frontBackDuration * 10 / 292 / 2; 
    frontBackDistanceCm = round(frontBackDistanceCm);
    Serial.print("Distance_HC-SR04: ");
    Serial.print(frontBackDistanceCm);
    Serial.println(" cm");
  }

  void turnLeft(){
    Serial.println("Left   ");
    digitalWrite(leftForward , LOW);
    digitalWrite(leftBackward , HIGH);
    digitalWrite(rightForward , LOW);
    digitalWrite(rightBackward , HIGH);
    delay(400);
  }

  void turnRight(){
    Serial.println("Right.   ");
    digitalWrite(leftForward , HIGH);
    digitalWrite(leftBackward , LOW);
    digitalWrite(rightForward , HIGH);
    digitalWrite(rightBackward , LOW);
    delay(400);
  }

  void forward(){
    Serial.println("Forward.");
    digitalWrite(leftForward , LOW);
    digitalWrite(leftBackward , HIGH);
    digitalWrite(rightForward , HIGH);
    digitalWrite(rightBackward , LOW);
  }
  void backward(){
    Serial.println("Backward.");
    digitalWrite(leftForward , HIGH);
    digitalWrite(leftBackward , LOW);
    digitalWrite(rightForward , LOW);
    digitalWrite(rightBackward , HIGH);
  }
   void Stop(){
    Serial.println("Stop.");
    digitalWrite(leftForward , LOW);
    digitalWrite(leftBackward , LOW);
    digitalWrite(rightForward , LOW);
    digitalWrite(rightBackward ,LOW);
  }

  void buzzerSound(){
     tone(buzzer, 4000, 500); 
     delay(1000);
     noTone(buzzer);
    }

  void receiveEvent(int bytes) {
    send_data = Wire.read();
    Serial.print(send_data);
  }
void loop()
{

  is_obstacle_ir_one = digitalRead(ir_one);
  is_obstacle_ir_two = digitalRead(ir_two);
  
  checkFrontDistance();
  checkFrontBackDistance();
 
  randNumber = random(0, 2);

    if (frontDistanceCm == (frontBackDistanceCm - 19) and (frontDistanceCm < maxtDistance))
      {
        Stop();
        delay(500);
        backward();
        delay(500);
          switch (randNumber)
            {
              case 0:
              turnLeft();
              delay(200);
              break;
              case 1:
              turnRight(); 
              delay(200);
              break;
            }
       }
    else if (frontBackDistanceCm < maxtBackDistance)
    {
        Stop();
        delay(500);
        backward();
        delay(500);
          switch (randNumber)
            {
              case 0:
              turnLeft();
              delay(200);
              break;
              case 1:
              turnRight(); 
              delay(200);
              break;
            }
    }
    
    else if (frontDistanceCm != (frontBackDistanceCm - 19) 
    and (frontDistanceCm < maxtDistance) and (frontBackDistanceCm > maxtBackDistance))
    {
      //GET DATA
      Stop();
      delay(2000);
      if (send_data == 1) {
        Serial.println("GOT POSITIVE DATA");
        buzzerSound();
        send_data = 2;
      }
      switch (randNumber)
            {
              case 0:
              turnLeft();
              delay(200);
              break;
              case 1:
              turnRight(); 
              delay(200);
              break;
            }
      
    }
      
    else
      {
        if (is_obstacle_ir_one == LOW)
          {
            Serial.println("OBSTACLE IR ---- 1");
            turnRight();
          }
        else if (is_obstacle_ir_two == LOW)
          {
            Serial.println("OBSTACLE IR ---- 2");
            turnLeft();
          }
        else
          {
            forward();
          }
      }
    }



  
