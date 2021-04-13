#include <PS2KeyAdvanced.h> // Add via Library Manager

// data pin of the PS/2 port
#define PIN_PS2_DATA 5
// clock pin of PS/2 port, must be an interrupt-capable pin
#define PIN_PS2_INTERRUPT PD2

// the wired serial data output pin
#define PIN_PC66_OUTPUT 13

typedef struct {
  unsigned char control;
  unsigned char key;
} KeyCode;

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
}

void sendByte(unsigned char ct, unsigned char dt) {
  // Copied from Esubi's code
  // http://sbeach.seesaa.net/article/409010371.html
  char i;
  char cttmp, dttmp;
  char pt; // parity
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

  for (i = 0;i < 8; ++i) {
    if ((dt & 0x01) != 0x00 ) {
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

  // report the logical inverse of everything that was just emitted.
  // do not update the parity bit
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

PS2KeyAdvanced keyboard;

void setup() {
  pinMode(PIN_PC66_OUTPUT, OUTPUT);
  digitalWrite(PIN_PC66_OUTPUT, HIGH);

  // init serial
  Serial.begin(9600);
  
  // init ps2 keyboard and report any errors
  keyboard.begin(PIN_PS2_DATA, PIN_PS2_INTERRUPT);
  keyboard.echo(); // send an echo command to the keyboard to see if it's there
  delay(6);
  char c = keyboard.read();
  if((c & 0xff) == PS2_KEY_ECHO || (c & 0xff) == PS2_KEY_BAT) {
    Serial.println("Found the keyboard!");
  }
  else {
    if((c & 0xff) == 0) {
      Serial.println("Keyboard not found.");
    }
    else {
      Serial.print("Invalid code returned from echo = ");
      Serial.println(c, HEX);
    }
  }
}

KeyCode translateKeycode(const uint16_t &raw) {
  unsigned char key = raw & 0xff;
  unsigned char control = 0x00;

  // TODO: CTRL table
  // TODO: ALT table

  Serial.print("Raw: 0x");
  Serial.println(raw, HEX);
  
  if(key >= 'A' && key <= 'Z' && !(raw & PS2_SHIFT)) {
    // default seems to be uppercase? that's weird
    key += 0x20;
  }
  if(key >= '0' && key <= '9' && (raw & PS2_SHIFT)) {
    // num row
    if(key == 0x30) { // '0'
      key = 0xa6;
    }
    else {
      key -= 0x10; // except for 0x30 (0). why?
    }
  }

  if(raw == 0x11e) {
    Serial.println("return shunt");
    key = 0x0d;
  }
  if(raw == 0x11f) {
    Serial.println("space shunt");
    key = 0x20;
  }
  if(raw == 0x161) { // f1
    control = 0b001;
    key = 0xf0;
  }
  if(raw == 0x162) { // f2
    control = 0b001;
    key = 0xf1;
  }
  if(raw == 0x163) { // f3
    control = 0b001;
    key = 0xf2;
  }
  if(raw == 0x164) { // f4
    control = 0b001;
    key = 0xf3;
  }
  if(raw == 0x165) { // f5
    control = 0b001;
    key = 0xf4;
  }

  if(raw == 0x11c) { // backspace
    key = 0x7f; // DEL?
  }

  // cursor keys
  if(raw == 0x115) { // left
    key = 0x1d;
  }
  else if(raw == 0x116) { // right
    key = 0x1c;
  }
  else if(raw == 0x117) { // up
    key = 0x1e;
  }
  else if(raw == 0x118) { // down 
    key = 0x1f; // TODO: Send 'game' codes too, since I bet this won't work well with games
  }

  if(raw & PS2_SHIFT) {
    // there's gotta be a better way to handle these
    switch(key) {
      case 0x3B:
        key = 0x2B; break; // '+'
      case 0x3A:
        key = 0x2A; break; // '*'
      case 0xF0:
        key = 0xF5; break; // turn F1 into F6
      case 0xF1:
        key = 0xF6; break; // turn F2 into F7
      case 0xF2:
        key = 0xF7; break; // turn F3 into F8
      case 0xF3:
        key = 0xF8; break; // turn F4 into F9
      case 0xF4:
        key = 0xF9; break; // turn F5 into F10
      case 0x5B:
        key = 0xA2; break; // turn '[' into 0xA2
      case 0x5D:
        key = 0xA3; break; // turn ']' into 0xA2
    }
  }
  else {
    // more hacks
    switch(key) {
      case 0x3c:
        key = 0x2c; break; // ,
      case 0x3d:
        key = 0x2d; break; // -
      case 0x3e:
        key = 0x2e; break; // .
      case 0x3f:
        key = 0x2f; break; // /
    }
  }

  if(raw & PS2_CTRL) {
    // Sending CTRL+A..Z - CTRL+C is necessary
    if(toupper(key) >= 'A' && toupper(key) <= 'Z') {
      key = (toupper(key) - 'A') + 1; // 000:001 for ctrl+A
    }
  }

  return KeyCode {
    control: control,
    key: key
  };
}

void loop() {
  uint16_t raw = keyboard.read();
  unsigned char asciiKey = raw & 0xff;
  bool isBreakCode = raw & PS2_BREAK;
  
  if(!isBreakCode && asciiKey != 0x00 && raw != 0x4106 && raw != 0x4107) {
    // no break codes necessary for 6601SR; just send autorepeat
    KeyCode k = translateKeycode(raw);
    
    Serial.print("Sending: ");
    Serial.print(k.control, HEX);
    Serial.print(":");
    Serial.println(k.key, HEX);
    sendByte(k.control, k.key);
  }
  // TODO: game mode packets
}
