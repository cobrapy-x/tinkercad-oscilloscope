// ===== Tinkercad Oscilloscope =====
// Two 16x2 I2C LCDs stacked into one 16x4 screen
// Signal in: A0 (0-5 V)   Time/div knob: potentiometer on A1

#include <Adafruit_LiquidCrystal.h>

Adafruit_LiquidCrystal lcdTop(0);     // top LCD    (address 0x20)
Adafruit_LiquidCrystal lcdBottom(1);  // bottom LCD (address 0x21)

const int WIDTH = 16;      // 16 columns
const int LEVELS = 32;     // 4 rows x 8 dot-rows = 32 heights

int samples[WIDTH];        // one sample per column
int prevRow[WIDTH];        // where each column's dot was drawn last time

// Put something in one cell of the 16x4 screen
// row 0-1 = top LCD, row 2-3 = bottom LCD
// ch = custom character (0-7), or -1 for blank
void putCell(int col, int row, int ch) {
  if (row < 2) {
    lcdTop.setCursor(col, row);
    if (ch < 0) lcdTop.print(' ');
    else lcdTop.write(byte(ch));
  } else {
    lcdBottom.setCursor(col, row - 2);
    if (ch < 0) lcdBottom.print(' ');
    else lcdBottom.write(byte(ch));
  }
}

void setup() {
  lcdTop.begin(16, 2);
  lcdBottom.begin(16, 2);
  lcdTop.setBacklight(1);
  lcdBottom.setBacklight(1);

  // Startup message: checks that both LCDs work
  lcdTop.setCursor(0, 0);
  lcdTop.print(" OSCILLOSCOPE");
  lcdTop.setCursor(0, 1);
  lcdTop.print(" Top LCD OK");
  lcdBottom.setCursor(0, 0);
  lcdBottom.print(" Bottom LCD OK");
  lcdBottom.setCursor(0, 1);
  lcdBottom.print(" Starting...");
  delay(2000);

  lcdTop.clear();
  lcdBottom.clear();

  // Make 8 custom characters on BOTH LCDs:
  // character N = flat line at height N (0 = bottom, 7 = top of the cell)
  for (int level = 0; level < 8; level++) {
    byte pattern[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    pattern[7 - level] = B11111;
    lcdTop.createChar(level, pattern);
    lcdBottom.createChar(level, pattern);
  }

  for (int x = 0; x < WIDTH; x++) {
    prevRow[x] = -1;       // nothing drawn yet
  }

  Serial.begin(9600);      // optional: see values in Serial Monitor too
}

void loop() {
  // Time/div knob: 5 ms to 100 ms between samples
  int sampleDelay = map(analogRead(A1), 0, 1023, 5, 100);

  // 1) Capture 16 samples
  for (int x = 0; x < WIDTH; x++) {
    int reading = analogRead(A0);                      // 0 to 1023
    samples[x] = map(reading, 0, 1023, 0, LEVELS - 1); // 0 to 31
    Serial.println(reading * 5.0 / 1023.0);            // volts
    delay(sampleDelay);
  }

  // 2) Draw them (only change the cells that need changing)
  for (int x = 0; x < WIDTH; x++) {
    int y = samples[x];            // 0 = bottom, 31 = top
    int dotRow = y % 8;            // height inside the cell
    int screenRow = 3 - (y / 8);   // which of the 4 text rows

    if (prevRow[x] >= 0 && prevRow[x] != screenRow) {
      putCell(x, prevRow[x], -1);  // erase old dot
    }
    putCell(x, screenRow, dotRow); // draw new dot
    prevRow[x] = screenRow;
  }
}
