// === Pin Definitions ===
#define TRIG_LEFT  11
#define ECHO_LEFT  12
#define TRIG_RIGHT 10
#define ECHO_RIGHT 9

#define IN1 5  // Left motor PWM
#define IN2 3
#define IN3 6  // Right motor PWM
#define IN4 7


int global_debug;
long vcc;
float distLeft;
float distRight;



// === Constants ===
// Distances in cm
// === Constants ===
// Distances in cm
float SPEED_MULTIPLIER = 1;
float OPTIMAL_DISTANCE = 11;
float SMALL_ERROR_THRESHOLD = 2;
float LARGE_ERROR_THRESHOLD = 6;
float TURN_THRESHOLD = 30;

// Correction PWM: when we need to steer LEFT (robot is too close to wall on right)
int STRONG_INNER_PWM = 200;
int STRONG_OUTER_PWM = 250;

int MILD_INNER_PWM = 70;
int MILD_OUTER_PWM = 255;

int STRAIGHT_LEFT_PWM = 255;
int STRAIGHT_RIGHT_PWM = 230;

// Turning PWM & duration
int TURN_INNER_PWM = 0;
int TURN_OUTER_PWM = 255;
int TURN_DURATION = 323; //800 funka nästan på 0.8 multi med 230 speed
int PRE_TURN_DELAY = 26;
int POST_TURN_DURATION = 500;


int MILD_DURATION = 90; //90
int STRONG_DURATION = 150;
int MILD_ADJUST_DURATION = 130; //150
int STRONG_ADJUST_DURATION = 200;

struct DistancePair {
  float right;
  float left;
};

void sortFloatArray(float arr[], int size) {
  for (int i = 0; i < size - 1; i++) {
    for (int j = i + 1; j < size; j++) {
      if (arr[j] < arr[i]) {
        float temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
      }
    }
  }
}



DistancePair getAverageDistances() {
  DistancePair d;
  float sumRight[5];
  float sumLeft[5];
  float totalRight = 0;
  float totalLeft = 0;

  for (int i = 0; i < 5; i++) {
    sumRight[i] = readDistance(TRIG_RIGHT, ECHO_RIGHT);
    delay(5);
    sumLeft[i] = readDistance(TRIG_LEFT, ECHO_LEFT);
    delay(5);
  }

  sortFloatArray(sumRight, 5);
  sortFloatArray(sumLeft, 5);
  
  for(int i = 1; i<5; i++) {
    totalRight += sumRight[i];
    totalLeft += sumLeft[i];
  }
  
  // Use median (middle of sorted array) for robustness against outliers
  d.right = totalRight / 4;
  d.left = totalLeft / 4;

  return d;
}








void strongCorrection( char direction) {
  if (direction == 'l') {
    setMotor(STRONG_INNER_PWM, STRONG_OUTER_PWM, 1);
    delay(STRONG_ADJUST_DURATION);
    setMotor(STRAIGHT_LEFT_PWM, STRAIGHT_RIGHT_PWM, 1);
    delay(STRONG_DURATION);
    setMotor(STRONG_OUTER_PWM, STRONG_INNER_PWM, 1);
    delay(STRONG_ADJUST_DURATION);
    setMotor(STRAIGHT_LEFT_PWM, STRAIGHT_RIGHT_PWM, 1);

    
  }
  else if (direction == 'r') {
    setMotor(STRONG_OUTER_PWM, STRONG_INNER_PWM, 1);
    delay(STRONG_ADJUST_DURATION);
    setMotor(STRAIGHT_RIGHT_PWM, STRAIGHT_LEFT_PWM, 1);
    delay(STRONG_DURATION);
    setMotor(STRONG_INNER_PWM, STRONG_OUTER_PWM, 1);
    delay(STRONG_ADJUST_DURATION);
    setMotor(STRAIGHT_LEFT_PWM, STRAIGHT_RIGHT_PWM, 1);
  }
}

void mildCorrection( char direction, int error, long vcc) {
  //int Adjustfactor = abs(error) * 0.5;
  //int AdjustTurnFactor = abs(error) * 0.5;
  int Adjustfactor = 1;
  int AdjustTurnFactor = 1;
  if (vcc < 4800) {
    Adjustfactor = 1.2;
    AdjustTurnFactor = 1.2;
  }
  if (direction == 'l') {
    setMotor(MILD_INNER_PWM, MILD_OUTER_PWM, 1);
    delay(MILD_ADJUST_DURATION * AdjustTurnFactor);
    setMotor(STRAIGHT_LEFT_PWM, STRAIGHT_RIGHT_PWM, 1);
    delay(MILD_DURATION * Adjustfactor);
    setMotor(MILD_OUTER_PWM, MILD_INNER_PWM, 1);
    delay(MILD_ADJUST_DURATION * AdjustTurnFactor);
    setMotor(STRAIGHT_LEFT_PWM, STRAIGHT_RIGHT_PWM, 1);

    
  }
  else if (direction == 'r') {
    setMotor(MILD_OUTER_PWM, MILD_INNER_PWM, 1);
    delay(MILD_ADJUST_DURATION * AdjustTurnFactor);
    setMotor(STRAIGHT_RIGHT_PWM, STRAIGHT_LEFT_PWM, 1);
    delay(MILD_DURATION * Adjustfactor);
    setMotor(220, MILD_OUTER_PWM, 1);
    delay(MILD_ADJUST_DURATION * AdjustTurnFactor);
    setMotor(STRAIGHT_LEFT_PWM, STRAIGHT_RIGHT_PWM, 1);
  }
  setMotor(STRAIGHT_LEFT_PWM, STRAIGHT_RIGHT_PWM, 1);
}

void hardTurn(char direction, long vcc) {
  int AdjustFactor = 1;
  
  if (vcc < 4800) {
    AdjustFactor = 1.5;
  }
  delay(PRE_TURN_DELAY * AdjustFactor);
  if (direction == 'l') {
    setMotor(TURN_INNER_PWM, TURN_OUTER_PWM, 0);
    
  }
  else if (direction == 'r') {
    setMotor(TURN_OUTER_PWM, TURN_INNER_PWM, 0 );
  }
  delay(TURN_DURATION);
  setMotor(STRAIGHT_LEFT_PWM, STRAIGHT_RIGHT_PWM, 1);
  delay(POST_TURN_DURATION);
}
// === Helper Functions ===
float readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  float duration = pulseIn(echoPin, HIGH, 20000); // timeout at 20ms
  float distance = duration * 0.034 / 2;
  return distance;
}

void setMotor(int leftPWM, int rightPWM, bool scale) {
  int scaledLeftPWM;
  int scaledRightPWM;
  // Left motor forward
  if (scale == 1) {
    scaledLeftPWM = (int)(leftPWM * SPEED_MULTIPLIER);
    scaledRightPWM = (int)(rightPWM * SPEED_MULTIPLIER);
  }
  else {
    scaledLeftPWM = leftPWM;
    scaledRightPWM = rightPWM;
  }
  digitalWrite(IN2, LOW);
  analogWrite(IN1, scaledLeftPWM);
  
  // Right motor forward
  digitalWrite(IN4, LOW);
  analogWrite(IN3, scaledRightPWM);
}

void stopMotors() {
  analogWrite(IN1, 0);
  analogWrite(IN3, 0);
}



void computeDriving(long distRight, long vcc){
  int error = distRight - OPTIMAL_DISTANCE;
  if (abs(error) < SMALL_ERROR_THRESHOLD) {
    setMotor(STRAIGHT_LEFT_PWM, STRAIGHT_RIGHT_PWM, 1);
    global_debug = 0;
    return;
  }
  else if (abs(error) >= SMALL_ERROR_THRESHOLD && abs(error) < LARGE_ERROR_THRESHOLD) {
    if (error >= 0) { 
      //to far left
      mildCorrection('r', error, vcc);
      global_debug = -1;
      return;
    }
    else {
      mildCorrection('l', error, vcc);
      global_debug = 1;
      return;
    }
  }
  else if (abs(error) >= LARGE_ERROR_THRESHOLD){
    if (error >= 0) { 
      //to far left
      mildCorrection('r', error, vcc);
      global_debug = -2;
      return;
    }
    else {
       //to far right
      mildCorrection('l', error, vcc);
      global_debug = 2;
      return;
    }
  }
}

// void computeDriving(float distRight, float distLeft, long vcc) {
//   float delta = distRight - distLeft;

//   if (distRight > distLeft) {
//     mildCorrection('r', 0, vcc);
//     global_debug = -1;
//   }
//   else if (distLeft > distRight) {
//     mildCorrection('l', 0, vcc);
//     global_debug = 1;
//   }
// }

long readVcc() {
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // Wait for conversion to complete
  uint16_t result = ADC;
  long vcc = 1125300L / result; // 1.1V * 1023 * 1000
  return vcc; // in millivolts
}


void setup() {

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Sensor pins
  pinMode(TRIG_LEFT, OUTPUT);
  pinMode(ECHO_LEFT, INPUT);
  pinMode(TRIG_RIGHT, OUTPUT);
  pinMode(ECHO_RIGHT, INPUT);
  // Motor pins
  Serial.begin(9600);
  delay(5000);
  long vcc = 0;
  while (vcc <= 4000) {
    vcc = readVcc();
    distLeft = readDistance(TRIG_LEFT, ECHO_LEFT);
    delay(5);
    distRight = readDistance(TRIG_RIGHT, ECHO_RIGHT);
    Serial.print(vcc);
    Serial.print(" distLeft: ");
    Serial.print(distLeft);
    Serial.print(" distRight: ");
    Serial.println(distRight);
    delay(250);
    
  }


  

  // Initialize motors stopped
  
}


void updateVariable(String input) {

input.remove(0,1);
int spaceIndex = input.indexOf(' ');
if (spaceIndex > 0) {
  String varName = input.substring(0, spaceIndex);
  String varVal = input.substring(spaceIndex + 1);
  int value = varVal.toInt();
  Serial.println(varName);
  Serial.println(value);
}

}

void loop() {
  vcc = readVcc();
  DistancePair avg = getAverageDistances();
  distRight = avg.right;
  distLeft = avg.left;

  
  while((distRight > 60 || distLeft > 100) || (distRight > 30 && distLeft > 30) || (distLeft < 1 || distRight < 1)) {
    setMotor(STRAIGHT_LEFT_PWM, STRAIGHT_RIGHT_PWM, 1);
    delay(20);
    Serial.print("Bad measuremnets detected! R:  ");
    Serial.print(distRight);
    Serial.print(" L: ");
    Serial.println(distLeft);
    avg = getAverageDistances();
    distRight = avg.right;
    distLeft = avg.left;

  }
  // Detect a right turn (open space on right, wall still on left)
  if (distRight > TURN_THRESHOLD) {
    hardTurn('r', vcc);
    global_debug = 3;
  }
  else if (distLeft > TURN_THRESHOLD) {
    hardTurn('l', vcc);
    global_debug = -3;
  }
  else {
    computeDriving(distRight, vcc);
  }

  
  Serial.print(global_debug);
  Serial.print(",");
  Serial.print(vcc);
  Serial.print(",");
  Serial.print(distRight);
  Serial.print(",");
  Serial.print(distLeft);
  Serial.print("\n");
  

  delay(20); // small delay to avoid rapid jitter
}




// void loop() {
//   float distRight = readDistance(TRIG_RIGHT, ECHO_RIGHT);
//   float distLeft = readDistance(TRIG_LEFT, ECHO_LEFT);
//   long vcc = readVcc();
//   Serial.print(global_debug);
//   Serial.print(",");
//   Serial.print(vcc);
//   Serial.print(",");
//   Serial.print(distRight);
//   Serial.print(",");
//   Serial.print(distLeft);
//   Serial.print("\n");
  
  
//   if (Serial.available()) {
//     String input = Serial.readStringUntil('\n');  // Read until newline

//     input.trim();  // Remove any whitespace/newlines

//     // Ignore HM-10 status messages
//     if (input == "OK+CONN" || input == "OK+LOST" || input == "CONN") {
//       return;
//     }
//     else if (input.startsWith("!")) {
//       Serial.println("Utrosptecken upptäckt!");
//       updateVariable(input);

//     }
//     else if (input == "l") {
//       Serial.println("Left command received");
//       // Add your left-turn logic here
//       hardTurn('l');  // Example action
//     }
//     else if (input == "r") {
//       Serial.println("Right command received");
//       // Add your right-turn logic here
//       hardTurn('r');   // Example action
//     }
//     else if (input == "s") {
//       Serial.println("Stop command received");
//       // Add your right-turn logic here
//       stopMotors();
//       }   // Example action
//     else if (input == "S") {
//       Serial.println("Straight command received");
//       // Add your right-turn logic here
//       setMotor(STRAIGHT_LEFT_PWM, STRAIGHT_RIGHT_PWM, 1);
//     }
//     else {
//       Serial.print("Unknown commandddd: ");
//       Serial.println(input);
//     }
//   }
    

    

//   //Optional: add a small delay to avoid spamming the CPU
//   delay(50);
// }

  
  



