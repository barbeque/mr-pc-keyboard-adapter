#include <PS2KeyAdvanced.h> // Add via Library Manager

// data pin of the PS/2 port
#define PIN_PS2_DATA 5
// clock pin of PS/2 port, must be an interrupt-capable pin
#define PIN_PS2_INTERRUPT PD2

// the wired serial data output pin
#define PIN_PC66_OUTPUT 13

#define FUDGE 0

/// Send an individual bit
void sendBit(bool b) {
  // from https://sbeach.up.seesaa.net/image/141115_02_66SR_keywire.png
  if(b) {
    // High is 501us high, 209us low
    digitalWrite(PIN_PC66_OUTPUT, HIGH);
    delayMicroseconds(501 + FUDGE);
    digitalWrite(PIN_PC66_OUTPUT, LOW);
    delayMicroseconds(209 + FUDGE);
  }
  else {
    // Low is 149us high, 209us low
    digitalWrite(PIN_PC66_OUTPUT, HIGH);
    delayMicroseconds(149 + FUDGE);
    digitalWrite(PIN_PC66_OUTPUT, LOW);
    delayMicroseconds(209 + FUDGE);
  }

  // it seems like you want to return to high when you end
  // communication
  digitalWrite(PIN_PC66_OUTPUT, HIGH);
}

void sendHeader() {
  digitalWrite(PIN_PC66_OUTPUT, LOW);
  delayMicroseconds(13 * 191);
  digitalWrite(PIN_PC66_OUTPUT, HIGH);
  delayMicroseconds(13 * 39);

  digitalWrite(PIN_PC66_OUTPUT, LOW);
  delayMicroseconds(13 * 17);
  digitalWrite(PIN_PC66_OUTPUT, HIGH);
  // TODO: check this, i don't understand the portb shit
}

void sendByte(unsigned char ct, unsigned char dt) {
  char i;
  char cttmp, dttmp;
  char pt;
  cttmp = ct;
  dttmp = dt;

  sendHeader();
  pt = 0x00;

  for(i = 0; i < 3; ++i) {
    if((ct & 0x01) != 0x00) {
      sendBit(true);
      pt = pt ^ 0x01;
    }
    else {
      sendBit(false);
    }
    ct = ct >> 1;
  }

  for (i = 0;i < 8;i ++) {
    if ( (dt & 0x01) != 0x00 ) {
      sendBit(true);
      pt = pt ^ 0x01;
    } else {
      sendBit(false);
    }
    dt = dt >> 1;
  }

  if(pt != 0x00) {
    sendBit(true);
  }
  else {
    sendBit(false);
  }

  // inverse time
  ct = cttmp;
  dt = dttmp;
  
  for (i = 0;i < 3;i ++) {
    if ( (ct & 0x01) != 0x00 ) {
      sendBit(false);
    } else {
      sendBit(true);
    }
    ct = ct >> 1;
  }

  for (i = 0;i < 8;i ++) {
    if ( (dt & 0x01) != 0x00 ) {
      sendBit(false);
    } else {
      sendBit(true);
    }
    dt = dt >> 1;
  }

  if ( pt != 0x00 ) {
    sendBit(false);
  } else {
    sendBit(true);
  }
}

void setup() {
  pinMode(PIN_PC66_OUTPUT, OUTPUT);
  digitalWrite(PIN_PC66_OUTPUT, HIGH);
}

void loop() {
  delay(1250);
  sendByte(0x0, 0x31); // '1'
}
