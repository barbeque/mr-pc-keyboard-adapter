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

  Serial.print("Raw: ");
  Serial.println(raw, HEX);
  
  if(key >= 'A' && key <= 'Z' && !(raw & PS2_SHIFT)) {
    // default seems to be uppercase? that's weird
    key += 0x20;
  }
  if(key >= '0' && key <= '9' && (raw & PS2_SHIFT)) {
    // num row
    key -= 0x10; // except for 0x30 (0)
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
