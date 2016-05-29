/**
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

//#define DEBUG 1
//#define DEBUG 0

#define OVERRIDE_OFF 0
#define OVERRIDE_ON 1
#define OVERRIDE_END 255

#define BUF_SIZE 25

char *pDutyMinor;

/*
   Some variables
*/
char cBuf[BUF_SIZE];

char *pBufPos;

char cChar;

int  nDutyMajor;
int  nDutyMinor;

byte sChrCnt = BUF_SIZE;

bool bOverride = false;
int nRes;
int nPin;

int nZeroASCII;
int nNineASCII;

int aParseRes[3];

/*
   Setup function - called after reset
*/
void setup() {
  Serial.begin(9600);

  pDutyMinor = new char[10];

  pinMode(ARDUINO_RED, OUTPUT);
  pinMode(ARDUINO_GREEN, OUTPUT);
  pinMode(ARDUINO_BLUE, OUTPUT);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH );

  nZeroASCII = 48;
  nNineASCII = 57;

  pBufPos = cBuf;
  sChrCnt = BUF_SIZE;
}

/*
   Debugger helper
*/
void debugMe(char* name, int val) {
#ifdef DEBUG
  Serial.print("[");
  Serial.print(name);
  Serial.print("]: ");
  Serial.println(val);
#endif
}

/*
   Set pwm signal on a selected pin of specified duty
*/
void setPWM(int pin, int pwm) {
  debugMe("pin", pin);
  debugMe("pwm", pwm);

  if (pin == RASPBERRY_BLUE ) {
    //debugMe(pin, pwm);
    analogWrite(ARDUINO_BLUE, pwm);

  } else if (pin == RASPBERRY_GREEN ) {
    //debugMe(pin, pwm);
    analogWrite(ARDUINO_GREEN, pwm);

  } else if (pin == RASPBERRY_RED ) {
    //debugMe(pin, pwm);
    analogWrite(ARDUINO_RED, pwm);

  } else {
    //do nothing, unknown PIN
  }
}

/*
   8xtimes faster than sscanf (but resolution limited to 0.01 - which is enough for
   estimating PWM)
*/
int parseA(int nBufOffset, int sChrCnt, int *trailingZeros) {
  int nVal = 0;
  int nMult = 1;
  int nHits = 0;
  bool bTrailer = true;
  bool bBreak = false;
  int nTrailerZeros = 0;
  for (int i = nBufOffset; i < BUF_SIZE - sChrCnt; i++) {
    if (cBuf[i] != '.' && cBuf[i] != '=' ) {
      int valAtChar = (int)cBuf[i] - nZeroASCII;
      if ( valAtChar == 0 && bTrailer ) {
        nTrailerZeros = nTrailerZeros + 1;
        *trailingZeros = nTrailerZeros;
        //debugMe("trailingZeros", nTrailerZeros);
      } else {
        bTrailer = false;
        if (nTrailerZeros == 0)*trailingZeros = 0;
      }
      nVal = nVal * nMult + valAtChar;
      nMult = 10;
    }

    if (cBuf[i] == '.' || cBuf[i] == '=' || nVal > 100 || ( i == (BUF_SIZE - sChrCnt - 1)) ) {
      debugMe("hits", nHits);
      debugMe("val", nVal);

      aParseRes[nHits++] = nVal;

      bTrailer = true;
      nTrailerZeros = 0;
      nVal = 0;
      nMult = 1;
      bBreak = false;
      if ( nHits >= 3) {
        break;
      }

    }

  }
  debugMe("exit", nHits);
  return nHits;
}
/*
   Much faster than atof
*/
int getPWMA(int major, int minor, int trailingZeros) {
  float rank = 10.;
  if (trailingZeros > 0)
    rank = rank * (trailingZeros * 10.);
  debugMe("    rankInit", rank);
  if ( minor >= 10) rank *= 10.;
  if ( minor >= 100) rank *= 10.;
  if ( minor >= 1000) rank *= 10.;
  if ( minor >= 10000) rank *= 10.;
  debugMe("    rank", rank);
  debugMe("    minor", minor);
  debugMe("    trailingZeros", trailingZeros);
  int pwm = (int)(((float)major + ((float)minor ) / rank) * 255.);
  return pwm;
}
/*
   Loop function
*/


void loop() {


  while ( Serial.peek() ) {
    cChar = Serial.read();
    if ( (cChar >= nZeroASCII && cChar <= nNineASCII) || cChar == '.' || cChar == '=' || cChar == 'a' || cChar == 'o') {
      *(pBufPos++) = cChar;
      sChrCnt -= 1;
      debugMe("Got char", cChar);

    }
    if ( sChrCnt == 0 ) {
      pBufPos = cBuf;
      sChrCnt = BUF_SIZE;
      debugMe("OVERRUN", 0);
      //end of buffer
      continue;
    }
    if (cChar == '\r' || cChar == '\n' && ((BUF_SIZE - sChrCnt) > 3) ) {
      *(pBufPos) = '\0';

      if ( (cBuf[0] == 'a') && (cBuf[1] == 'o')) {
        int trailingZeros = 0;
        nRes = parseA(3, sChrCnt, &trailingZeros);
        if ( nRes != 2 ) {
          pBufPos = cBuf;
          sChrCnt = BUF_SIZE;
          break;
        }

        nDutyMajor = aParseRes[0];
        nDutyMinor = aParseRes[1];

        switch (nDutyMajor) {
          case OVERRIDE_OFF:
            //Switch off
            digitalWrite( RELAY_PIN, LOW);
            bOverride = true;
            break;
          case OVERRIDE_ON:
            //Switch on
            digitalWrite( RELAY_PIN, HIGH);
            bOverride = false;
            break;
          case OVERRIDE_END:
            digitalWrite( RELAY_PIN, HIGH);
            bOverride = false;
            break;
          case RASPBERRY_RED:
          case RASPBERRY_GREEN:
          case RASPBERRY_BLUE:
            //Turn red
            setPWM(nDutyMajor, nDutyMinor);
            bOverride = true;
            break;
          default:
            pBufPos = cBuf;
            sChrCnt = BUF_SIZE;
            break;
        }

      } else if ( !bOverride ) {
        int trailingZeros = 0;
        nRes = parseA(0, sChrCnt, &trailingZeros);

        if ( nRes != 3 ) {
          pBufPos = cBuf;
          sChrCnt = BUF_SIZE;
          break;
        }
        nPin       = aParseRes[0];
        nDutyMajor = aParseRes[1];
        nDutyMinor = aParseRes[2];
        setPWM(nPin, getPWMA(nDutyMajor, nDutyMinor, trailingZeros));
      }
      pBufPos = cBuf;
      sChrCnt = BUF_SIZE;
    }
  }
}
