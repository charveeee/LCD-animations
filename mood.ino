#include <Wire.h>
#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x27, 16, 2);


const int buttonPin = 23;

byte happyFace[8] = {
  0b00000,
  0b01010,
  0b01010,
  0b00000,
  0b10001,
  0b01110,
  0b00000,
  0b00000
};

byte sadFace[8] = {
  0b00000,
  0b01010,
  0b01010,
  0b00000,
  0b00000,
  0b01110,
  0b10001,
  0b00000
};

byte surprisedFace[8] = {
  0b00000,
  0b01010,
  0b01010,
  0b00000,
  0b00100,
  0b01010,
  0b00100,
  0b00000
};

byte winkFace[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b10001,
  0b01110,
  0b00000,
  0b00000
};

const int numFaces = 4;
int currentFace = 0;
bool lastButtonState = HIGH;

void showFace(int faceIndex) {
  lcd.clear();
  lcd.setCursor(7, 0);
  lcd.write(byte(faceIndex));

  lcd.setCursor(4, 1);
  switch (faceIndex) {
    case 0: lcd.print("happy!"); break;
    case 1: lcd.print("sad..."); break;
    case 2: lcd.print("wow!"); break;
    case 3: lcd.print("wink;"); break;
  }
}

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);

  Wire.begin(21, 22); 
  lcd.init();
  lcd.backlight();

  lcd.createChar(0, happyFace);
  lcd.createChar(1, sadFace);
  lcd.createChar(2, surprisedFace);
  lcd.createChar(3, winkFace);

  showFace(currentFace);
}

void loop() {
  bool buttonState = digitalRead(buttonPin);

  if (lastButtonState == HIGH && buttonState == LOW) {
    currentFace = (currentFace + 1) % numFaces;
    showFace(currentFace);
    delay(200);
  }

  lastButtonState = buttonState;
  delay(10);
}
