#include <stdio.h>
#include <float.h>
void mapps(float val, float *mins, float *maxx) {
    if(val > *maxx) {
        *maxx = val;
    }
    if(val < *mins) {
        *mins = val;
    }
}
float map(float val, float in_min, float in_max, float out_min, float out_max) {
    return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int main(){
    float mins = FLT_MAX, maxx = FLT_MIN;
    mapps(5,&mins,&maxx);
    mapps(10,&mins,&maxx);
    float an = map(10,mins,maxx ,0,360);
    printf("%f , %f , %f",an,mins,maxx);
}
----
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <float.h>

const int MPU6050_ADDR_1 = 0x68;  // Default I2C address of MPU6050
Adafruit_MPU6050 gyro1;
Adafruit_MPU6050 gyro2;
Adafruit_MPU6050 gyro3;

float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
float roll1, pitch1, yaw1;  // Sensor 1 values
float roll2, pitch2, yaw2;  // Sensor 2 values
float roll3, pitch3, yaw3;  // Sensor 3 values
float mins_pitch_pinky = FLT_MAX, maxx_pitch_pinky = FLT_MIN;  // V
float mins_roll_pinky = FLT_MAX, maxx_roll_pinky = FLT_MIN;  // V
float mins_yaw_pinky = FLT_MAX, maxx_yaw_pinky = FLT_MIN;  // V
float mins_pitch_pinky = FLT_MAX, maxx_pitch_pinky = FLT_MIN;  // V
float elapsedTime, currentTime, previousTime;
String uploadPRY;

const float RAD_TO_DEG = 57.2958;  // Conversion factor for radians to degrees

// Mapping function to scale values
float map(float val, float in_min, float in_max, float out_min, float out_max) {
    return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Update the min/max values for dynamic scaling
void mapps(float val, float *mins, float *maxx) {
    if (val > *maxx) {
        *maxx = val;
    }
    if (val < *mins) {
        *mins = val;
    }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(15, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);

  gyro1.begin();
  gyro2.begin();
  gyro3.begin();

  digitalWrite(15, HIGH);
  digitalWrite(13, HIGH);
  digitalWrite(12, HIGH);
}

void loop() {
  // Read values from the 3 sensors
  digitalWrite(15, LOW);
  getValues(gyro1, 1, &pitch1, &roll1, &yaw1);
  digitalWrite(15, HIGH);
  delay(1000);
  
  digitalWrite(13, LOW);
  getValues(gyro2, 2, &pitch2, &roll2, &yaw2);
  digitalWrite(13, HIGH);
  delay(1000);

  digitalWrite(12, LOW);
  getValues(gyro3, 3, &pitch3, &roll3, &yaw3);
  digitalWrite(12, HIGH);
  delay(1000);
  
  // Apply dynamic scaling to all 9 values
  mapValues();
  
  // Print mapped values
  printMappedValues();
}

void getValues(Adafruit_MPU6050 &mpu, int n, float *pitch, float *roll, float *yaw)
{
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;
  float rx = g.gyro.x;
  float ry = g.gyro.y;
  float rz = g.gyro.z;
  
  accToDisp(ax, ay, az, rx, ry, rz, pitch, roll, yaw);
}

void accToDisp(float ax, float ay, float az , float rx, float ry, float rz, float *pitch, float *roll, float *yaw) {
  accAngleX = (atan(ay / sqrt(pow(ax, 2) + pow(az, 2))) * RAD_TO_DEG);
  accAngleY = (atan(-1 * ax / sqrt(pow(ay, 2) + pow(az, 2))) * RAD_TO_DEG);

  previousTime = currentTime;        // Previous time is stored before the actual time read
  currentTime = millis();            // Current time actual time read
  elapsedTime = (currentTime - previousTime) / 1000; 
  *yaw = rz * elapsedTime * RAD_TO_DEG;  // Convert yaw rate (rz) from radians to degrees
  elapsedTime -= 0.25;
  
  gyroAngleX = rx * elapsedTime * RAD_TO_DEG;  // Pitch
  gyroAngleY = ry * elapsedTime * RAD_TO_DEG;  // Roll
  
  // Complementary filter - combine accelerometer and gyro angle values
  *roll = 0.96 * gyroAngleX + 0.04 * accAngleX;
  *pitch = 0.96 * gyroAngleY + 0.04 * accAngleY;

  // Update mins and maxx for scaling
  mapps(*pitch, &mins, &maxx);
  mapps(*roll, &mins, &maxx);
  mapps(*yaw, &mins, &maxx);
}

// Map all the pitch, roll, yaw values to a new range (0 to 360 degrees)
void mapValues() {
  pitch1 = map(pitch1, mins, maxx, 0, 360);
  roll1  = map(roll1, mins, maxx, 0, 360);
  yaw1   = map(yaw1, mins, maxx, 0, 360);

  pitch2 = map(pitch2, mins, maxx, 0, 360);
  roll2  = map(roll2, mins, maxx, 0, 360);
  yaw2   = map(yaw2, mins, maxx, 0, 360);

  pitch3 = map(pitch3, mins, maxx, 0, 360);
  roll3  = map(roll3, mins, maxx, 0, 360);
  yaw3   = map(yaw3, mins, maxx, 0, 360);
}

void printMappedValues() {
  Serial.println("Mapped Values:");
  
  Serial.print("Sensor 1 - Pitch: ");
  Serial.print(pitch1);
  Serial.print(", Roll: ");
  Serial.print(roll1);
  Serial.print(", Yaw: ");
  Serial.println(yaw1);

  Serial.print("Sensor 2 - Pitch: ");
  Serial.print(pitch2);
  Serial.print(", Roll: ");
  Serial.print(roll2);
  Serial.print(", Yaw: ");
  Serial.println(yaw2);

  Serial.print("Sensor 3 - Pitch: ");
  Serial.print(pitch3);
  Serial.print(", Roll: ");
  Serial.print(roll3);
  Serial.print(", Yaw: ");
  Serial.println(yaw3);
}
