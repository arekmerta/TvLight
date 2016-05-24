/*
   Pi Blaster parser

   This software is to parse Pi-Blaster frames received over Serial Link

   -----
   Highly experimental, under development, no liabilities, use at own risk.
   More:
   http://uczymy.edu.pl/wp/na-warsztacie/doswiadczenia-z-tasmami-led-i-raspberry-pi/
*/

#define RASPBERRY_RED 17
#define RASPBERRY_GREEN 22
#define RASPBERRY_BLUE 24

#define ARDUINO_RED 3
#define ARDUINO_GREEN 6
#define ARDUINO_BLUE 9

#define RELAY_PIN 12

#define DEBUG 1
//#define DEBUG 0

#define OVERRIDE_OFF 0
#define OVERRIDE_ON 1

#define OVERRIDE_END 255


#define BUF_SIZE 25
char buf[BUF_SIZE];
char *bufPos = buf;

int pin;
int dutyMajor;
char dutyMinor[10];

char c;
byte chrCnt = BUF_SIZE;

bool override = false;

//*************************************
void setup() {
  Serial.begin(9600);

  pinMode(ARDUINO_RED, OUTPUT);
  pinMode(ARDUINO_GREEN, OUTPUT);
  pinMode(ARDUINO_BLUE, OUTPUT);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH );
}

//*************************************
void debugMe(int pin, int pwm) {
#ifdef DEBUG
  Serial.print(":");
  Serial.print(pin);
  Serial.print(",");
  Serial.println(pwm);
#endif
}

//*************************************
void firePWM(int pin, int duty) {
  analogWrite(pin, duty);
}

//*************************************
void setPWM(int pin, float duty) {

  int pwm = (int)(255.*duty);

  if (pin == RASPBERRY_BLUE ) {
    //debugMe(pin, pwm);
    firePWM(ARDUINO_BLUE, pwm);
    
  } else if (pin == RASPBERRY_GREEN ) {
    //debugMe(pin, pwm);
    firePWM(ARDUINO_GREEN, pwm);
    
  } else if (pin == RASPBERRY_RED ) {
    //debugMe(pin, pwm);
    firePWM(ARDUINO_RED, pwm);
    
  }
}

//*************************************
void loop() {
  int res;
  while (Serial.peek()) {
    c = Serial.read();
    if (isAlphaNumeric(c) || c == '.' || c == '=') {
      *(bufPos++) = c;
      chrCnt -= 1;
    }
    if ( chrCnt == 0 ) {
      chrCnt = BUF_SIZE;
      bufPos = buf;
      continue;
    }
    if (c == '\r' || c == '\n' && ((BUF_SIZE - chrCnt) > 3) ) {
      *(bufPos) = '\0';
      if ( (buf[0] == 'o') && (buf[1] == 'v')) {
        res = sscanf(buf, "ov=%d.%s", &dutyMajor, dutyMinor + 1);
        if ( res == 2) {
          switch(dutyMajor){
            case OVERRIDE_OFF:
              //Switch off
              digitalWrite( RELAY_PIN, LOW);
              override = true; 
            break;
            case OVERRIDE_ON:
              //Switch on
              digitalWrite( RELAY_PIN, HIGH);
              override = false;
            break;
            case OVERRIDE_END:
              override = false;
            break;
            case RASPBERRY_RED:
              //Turn red 
              firePWM(ARDUINO_RED, 255);
              firePWM(ARDUINO_GREEN, 0);
              firePWM(ARDUINO_BLUE, 0);
              override = true;         
            break;
            case RASPBERRY_GREEN: 
              //Turn green
              firePWM(ARDUINO_RED, 0);
              firePWM(ARDUINO_GREEN, 255);
              firePWM(ARDUINO_BLUE, 0);
              override = true;
            break;
            case RASPBERRY_BLUE: 
              //Turn blue
              firePWM(ARDUINO_RED, 0);
              firePWM(ARDUINO_GREEN, 0);
              firePWM(ARDUINO_BLUE, 255);
              override = true;
            break;
          }
        }
      } else if( !override ) {
        res = sscanf(buf, "%d=%d.%s", &pin, &dutyMajor, dutyMinor + 1);
        if ( res ==  3) {
          dutyMinor[0] = '.';
          float duty = atof(dutyMinor) + dutyMajor;
          setPWM(pin, duty);
        }
      }
      chrCnt = BUF_SIZE;
      bufPos = buf;
    }
  }
}
