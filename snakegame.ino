#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);


const int encA = 32;      
const int encB = 33;      
const int encSW = 25;     
const int startBtn = 26;  
const int setBtn = 23;    

// ---- Grid dimensions ----
const int GRID_W = 16;
const int GRID_H = 2;
const int MAX_LEN = GRID_W * GRID_H;


enum Direction { UP, RIGHT, DOWN, LEFT };
Direction dir = RIGHT;


int snakeX[MAX_LEN];
int snakeY[MAX_LEN];
int snakeLen = 3;

int foodX, foodY;

bool gameOver = false;
bool paused = false;
bool gameStarted = false;

unsigned long lastMove = 0;
unsigned long moveInterval = 500; 

int score = 0;


volatile int encDelta = 0;
volatile int lastEncoded = 0;



const int TURN_THRESHOLD = 4;


byte bodyChar[8]  = {0b00000,0b01110,0b11111,0b11111,0b11111,0b11111,0b01110,0b00000};
byte foodChar[8]  = {0b00000,0b00100,0b01110,0b11111,0b11111,0b01110,0b00100,0b00000};
byte headUp[8]    = {0b00100,0b01110,0b10101,0b00100,0b00100,0b00100,0b00100,0b00000};
byte headDown[8]  = {0b00100,0b00100,0b00100,0b00100,0b10101,0b01110,0b00100,0b00000};
byte headLeft[8]  = {0b00000,0b00010,0b00110,0b11111,0b00110,0b00010,0b00000,0b00000};
byte headRight[8] = {0b00000,0b01000,0b01100,0b11111,0b01100,0b01000,0b00000,0b00000};


void IRAM_ATTR encoderISR() {
  int MSB = digitalRead(encA);
  int LSB = digitalRead(encB);
  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encDelta++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encDelta--;

  lastEncoded = encoded;
}

void placeFood() {
  bool onSnake;
  do {
    onSnake = false;
    foodX = random(0, GRID_W);
    foodY = random(0, GRID_H);
    for (int i = 0; i < snakeLen; i++) {
      if (snakeX[i] == foodX && snakeY[i] == foodY) { onSnake = true; break; }
    }
  } while (onSnake);
}

void setupGame() {
  snakeLen = 3;
  for (int i = 0; i < snakeLen; i++) {
    snakeX[i] = 4 - i;
    snakeY[i] = 0;
  }
  dir = RIGHT;
  score = 0;
  moveInterval = 500;
  gameOver = false;
  paused = false;
  placeFood();
}

void turnFromEncoder() {
  noInterrupts();
  int delta = encDelta;
  encDelta = 0;
  interrupts();

  if (delta >= TURN_THRESHOLD) {
    dir = (Direction)((dir + 1) % 4);
  } else if (delta <= -TURN_THRESHOLD) {
    dir = (Direction)((dir + 3) % 4); 
  }
}

void moveSnake() {
  int newX = snakeX[0];
  int newY = snakeY[0];

  switch (dir) {
    case UP:    newY -= 1; break;
    case DOWN:  newY += 1; break;
    case LEFT:  newX -= 1; break;
    case RIGHT: newX += 1; break;
  }

  if (newX < 0 || newX >= GRID_W || newY < 0 || newY >= GRID_H) {
    gameOver = true;
    return;
  }

  for (int i = 0; i < snakeLen; i++) {
    if (snakeX[i] == newX && snakeY[i] == newY) {
      gameOver = true;
      return;
    }
  }

  for (int i = snakeLen; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }
  snakeX[0] = newX;
  snakeY[0] = newY;

  if (newX == foodX && newY == foodY) {
    if (snakeLen < MAX_LEN) snakeLen++;
    score++;
    if (moveInterval > 150) moveInterval -= 20; 
    placeFood();
  }
}

void drawGame() {
  char row0[GRID_W + 1];
  char row1[GRID_W + 1];
  for (int i = 0; i < GRID_W; i++) { row0[i] = ' '; row1[i] = ' '; }
  row0[GRID_W] = '\0';
  row1[GRID_W] = '\0';

  if (foodY == 0) row0[foodX] = 1; else row1[foodX] = 1;

  for (int i = 1; i < snakeLen; i++) {
    if (snakeY[i] == 0) row0[snakeX[i]] = 0; else row1[snakeX[i]] = 0;
  }

  byte headChar;
  switch (dir) {
    case UP:    headChar = 2; break;
    case DOWN:  headChar = 3; break;
    case LEFT:  headChar = 4; break;
    case RIGHT: headChar = 5; break;
  }
  if (snakeY[0] == 0) row0[snakeX[0]] = headChar; else row1[snakeX[0]] = headChar;

  lcd.setCursor(0, 0);
  for (int i = 0; i < GRID_W; i++) lcd.write((byte)row0[i]);
  lcd.setCursor(0, 1);
  for (int i = 0; i < GRID_W; i++) lcd.write((byte)row1[i]);
}

void showStartScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press START to");
  lcd.setCursor(0, 1);
  lcd.print("play SNAKE!");
}

void showGameOver() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Game Over!");
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(score);
}


bool readButtonPressed(int pin, unsigned long &lastPressTime) {
  if (digitalRead(pin) == LOW && millis() - lastPressTime > 250) {
    lastPressTime = millis();
    return true;
  }
  return false;
}

unsigned long lastStartPress = 0;
unsigned long lastSWPress = 0;

void setup() {
  pinMode(encA, INPUT_PULLUP);
  pinMode(encB, INPUT_PULLUP);
  pinMode(encSW, INPUT_PULLUP);
  pinMode(startBtn, INPUT_PULLUP);
  pinMode(setBtn, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(encA), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB), encoderISR, CHANGE);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  lcd.createChar(0, bodyChar);
  lcd.createChar(1, foodChar);
  lcd.createChar(2, headUp);
  lcd.createChar(3, headDown);
  lcd.createChar(4, headLeft);
  lcd.createChar(5, headRight);

  randomSeed(analogRead(34)); 

  showStartScreen();
}

void loop() {
  if (!gameStarted) {
    if (readButtonPressed(startBtn, lastStartPress)) {
      setupGame();
      gameStarted = true;
    }
    return;
  }

  if (gameOver) {
    showGameOver();
    if (readButtonPressed(startBtn, lastStartPress)) {
      setupGame();
    }
    delay(200);
    return;
  }

  if (readButtonPressed(encSW, lastSWPress)) {
    paused = !paused;
  }

  turnFromEncoder();

  if (!paused && millis() - lastMove > moveInterval) {
    lastMove = millis();
    moveSnake();
    if (!gameOver) drawGame();
  }
}
