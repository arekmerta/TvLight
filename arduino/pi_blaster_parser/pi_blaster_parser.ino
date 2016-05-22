/*
 * Pi Blaster parser
 * 
 * This software is to parse Pi-Blaster frames received over Serial Link
 * 
 * -----
 * Highly experimental, under development, no liabilities, use at own risk
 * More:
 * http://uczymy.edu.pl/wp/na-warsztacie/doswiadczenia-z-tasmami-led-i-raspberry-pi/
 */
#include <SoftwareSerial.h>

SoftwareSerial mySerial(4, 2);

#define BUF_SIZE 20
char buf[BUF_SIZE];

#define RASPBERRY_RED 17
#define RASPBERRY_GREEN 22
#define RASPBERRY_BLUE 24

#define ARDUINO_RED 3
#define ARDUINO_GREEN 6
#define ARDUINO_BLUE 9

#define DEBUG 1
//#undef DEBU1

//int lastPWMValues[10] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
//int ssMax = 1;
//int ssMin = -1;

int pin;
int dutyMajor;
char dutyMinor[10];

char c;
char *bufPos = buf;
byte chrCnt = BUF_SIZE;


void setup() {
  Serial.begin(9600);

  //mySerial.begin(9600);
  //mySerial.println("START");

  pinMode(ARDUINO_RED, OUTPUT);
  pinMode(ARDUINO_GREEN, OUTPUT);
  pinMode(ARDUINO_BLUE, OUTPUT);

}


void displayWrite(int pin, int pwm){
  Serial.print(":");
  Serial.print(pin);
  Serial.print(",");
  Serial.println(pwm);

}


void firePWM(int pin, int duty){
  analogWrite(pin, duty);
}
void setPWM(int pin, float duty) {
  
  int pwm = (int)(255.*duty);
  
  if (pin == RASPBERRY_BLUE ) {
    //int diff = pwm - lastPWMValues[ARDUINO_BLUE];
    //if ( diff < ssMin || diff > ssMax ) {
      //Blue
      //mySerial.print("bb:");
      //displayWrite(pin,pwm);
      
      firePWM(ARDUINO_BLUE, pwm);
      //lastPWMValues[ARDUINO_BLUE] = pwm;
    //}
  } else if (pin == RASPBERRY_GREEN ) {
    //int diff = pwm - lastPWMValues[ARDUINO_GREEN];

    //if ( diff < ssMin || diff > ssMax ) {
      //Green
      //mySerial.print("gg:");
      //displayWrite(pin,pwm);
      
      firePWM(ARDUINO_GREEN, pwm);
    //  lastPWMValues[ARDUINO_GREEN] = pwm;
    //}
  } else if (pin == RASPBERRY_RED ) {
    //int diff = pwm - lastPWMValues[ARDUINO_RED];

    //if ( diff < ssMin || diff > ssMax ) {
      //Red
      //mySerial.print("rr:");
      //displayWrite(pin,pwm);
      
      firePWM(ARDUINO_RED, pwm);
    //  lastPWMValues[ARDUINO_RED] = pwm;
    //}
  }
}

void loop() {


  //if (Serial.available()) 
  {
    //Serial.println("got...");
    while (Serial.peek()) {
      //mySerial.print(";");
      c = Serial.read();
      if(isAlphaNumeric(c) || c=='.' || c=='='){
      //mySerial.print("read:");
      //mySerial.println(c);
      
      *(bufPos++) = c;
      chrCnt -= 1;
      }
      //if (chrCnt < (BUF_SIZE - 4))
        if (c == '\r' || c=='\n') {
          *(bufPos) = '\0';
          //mySerial.print(">>>");
          //mySerial.println(buf);
          
          int r = sscanf(buf, "%d=%d.%s", &pin, &dutyMajor, dutyMinor + 1);
          if ( r ==  3) {
            dutyMinor[0] = '.';
            
            float duty = atof(dutyMinor) + dutyMajor;
            
            setPWM(pin, duty);
          }
          chrCnt = 0;
        }
      if (chrCnt == 0) {
        chrCnt = BUF_SIZE;
        bufPos = buf;
        //memset(buf, 0, sizeof(buf));
      }
    }
  }
  //delay(10);
  //#endif

}
