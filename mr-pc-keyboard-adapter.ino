#include <PS2KeyAdvanced.h> // Add via Library Manager

// data pin of the PS/2 port
#define PIN_PS2_DATA 5
// clock pin of PS/2 port, must be an interrupt-capable pin
#define PIN_PS2_INTERRUPT PD2

// the wired serial data output pin
#define PIN_PC66_OUTPUT 13

// Signals sent in "game" mode
enum GamePacket {
  GAME_SPACE = 0x1,
  GAME_RESERVED = 0x2,
  GAME_LEFT = 0x4,
  GAME_RIGHT = 0x8,
  GAME_DOWN = 0x10,
  GAME_UP = 0x20,
  GAME_STOP = 0x40,
  GAME_SHIFT = 0x80
}

/// Send an individual bit
void sendBit(bool b) {

  // from https://sbeach.up.seesaa.net/image/141115_02_66SR_keywire.png
  if(b) {
    // High is 501us high, 209us low
    digitalWrite(PIN_PC66_OUTPUT, HIGH);
    delayMicroseconds(501);
    digitalWrite(PIN_PC66_OUTPUT, LOW);
    delayMicroseconds(209);
  }
  else {
    // Low is 149us high, 209us low
    digitalWrite(PIN_PC66_OUTPUT, HIGH);
    delayMicroseconds(149);
    digitalWrite(PIN_PC66_OUTPUT, LOW);
    delayMicroseconds(209);
  }

  // it seems like you want to return to high when you end
  // communication
}

/// Send from bit 7 .. bit 0
void sendByteLSBFirst(unsigned char b) {
  // may be a good candidate to unroll, if the compiler can't
  for(unsigned int i = 0x1; i <= 0x80; i <<= 1) {
    sendBit((b & i) != 0);
  }
}

void sendWiredHeader() {
  // header 1: pull the line down for 2495us
  digitalWrite(PIN_PC66_OUTPUT, LOW);
  delayMicroseconds(2495);
  // header 2: 501 high 209 low (a logic "1")
  sendBit(true);
}

void sendGamePacket(const GamePacket& g) {
  sendBit(false);
  sendBit(true);
  sendBit(false);
  sendByteLSBFirst((unsigned char)g);
}

void setup() {
  Serial.begin(9600);

  pinMode(PIN_PC66_OUTPUT, OUTPUT);
  digitalWrite(PIN_PC66_OUTPUT, HIGH);
}

void loop() {
  // http://sbeach.seesaa.net/article/408970013.html
  // minimum interval between datagrams is 4657us
  delay(250); // just hang around awhile

  // send the code for F5
  sendWiredHeader();
  // F5 - 001:F4
  sendBit(true);
  sendBit(false);
  sendBit(false); // remember, LSB first!  

  sendByteLSBFirst(0xF4);

  // reset the line to high
  digitalWrite(PIN_PC66_OUTPUT, HIGH);
}
