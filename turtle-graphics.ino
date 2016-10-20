// Turtle Graphics for Arduboy
// https://github.com/strickyak/turtle-graphics-arduboy
//
// Copyright 2016 Henry Strickland (strickyak on github)
//
// Use with MIT License.  Is GPL compatible.

#include <EEPROM.h>
#include <Arduboy.h>
Arduboy A;

const char* InitialProgram = " 8(            8(            45L 8F     )          45R 8F     )";

const char* Splash[] = {
  "",
  "",
  "TURTLE GRAPHICS",
  "",
  "Try the red buttons",
  "first.",
  "Left&Right to navigate.",
  "Up&Down to type.",
  "Both red buttons at",
  "same time to clear.",
  "",
  "github.com/strickyak/",
  "turtle-graphics-",
  "arduboy",
  nullptr,
};

const char* Help[] = {
  "TURTLE GRAPHICS",
  "  for any number n:",
  "nF: go n pixels forward",
  "nL: turn left n degrees",
  "nR: turn right n degrees",
  "n(...): repeat n times",
  nullptr
};

struct Terminal {
  constexpr static int N = 16;
  char Alphabet[N] = " FLR()9876543210";
  constexpr static int W = 12;
  constexpr static int H = 6;
  constexpr static int M = W * H;
  constexpr static int GW = 128;
  constexpr static int GH = 64;
  int held;
  int p;
  char scr[M];
  bool a_down;
  bool b_down;

  void Tick() {
    a_down = b_down = false;
    int bs = A.buttonsState();

    // Count how long buttons are held down.
    if (bs) ++held; else held = 0;

    if (bs & A_BUTTON) {
      a_down = true;
    }
    if (bs & B_BUTTON) {
      b_down = true;
    }

    if (a_down || b_down) return;

    // Auto-repeat after 10, on even ticks.
    if (held == 1 || held > 10 && (held & 1) == 0) {
      if (bs & LEFT_BUTTON) {
        --p; if (p < 0) p = M - 1;
      }
      if (bs & RIGHT_BUTTON) {
        ++p; if (p >= M) p = 0;
      }
      char& ch = scr[p];
      if (bs & UP_BUTTON) {
        --ch; if (ch < 0) ch = N - 1;
      }
      if (bs & DOWN_BUTTON) {
        ++ch; if (ch >= N) ch = 0;
      }
      if (bs & A_BUTTON) {
        a_down = true;
      }
    }
  }

  char AtI(int i) {
    return Alphabet[scr[i]];
  }
  void PutAtI(int i, char c) {
    for (int a = 0; a < N; a++) {
      if (Alphabet[a] == c) {
        scr[i] = a; return;
      }
    }
    scr[i] = 0;
  }

  void Draw() {
    A.clear();
    for (int y = 0; y < H; y++) {
      for (int x = 0; x < W; x++) {
        A.setCursor(x * 10 + 2, y * 10 + 2);
        A.write(Alphabet[scr[x + y * W]]);
      }
    }

    int xx = p % W;
    int yy = p / W;
    A.drawFastVLine(xx * 10, yy * 10, 10, 1);
    A.drawFastHLine(xx * 10, yy * 10, 10, 1);
    A.drawFastHLine(xx * 10, 9 + yy * 10, 10, 1);
    A.drawFastVLine(9 + xx * 10, yy * 10, 10, 1);
    A.display();
  }
};

struct Turtle : public Terminal {
  int x, y;
  int d;  // direction, degrees
  int n;
  int sp;
  int counts[16];
  int openParen[16];

  void drawLineTorus(int x1, int y1, int x2, int y2, int color) {
    for (int i = -1; i < 2; i++) {
      //if (i == -1 && x1 > 0 && x2 > 0) continue;
      //if (i == 1 && x1 < GW && x2 < GW) continue;
      for (int j = -1; j < 2; j++) {
        //if (j == -1 && y1 > 0 && y2 > 0) continue;
        //if (j == 1 && y1 < GH && y2 < GH) continue;

        A.drawLine(x1 + i * GW, y1 + j * GH, x2 + i * GW, y2 + j * GH, color);
      }
    }
  }

  void DrawPath() {
    x = 64; y = 32; d = 0; n = 0; sp = 0;
    int steps = 0;
    for (int i = 0; i < M; i++) {
      if (steps > held) return;
      char ch = AtI(i);
      if ('0' <= ch && ch <= '9') {
        n = 10 * n + (ch - '0');
      } else switch (ch) {
          case 'F':
            {
              if (n < 1) n = 1;
              int x2 = (int)(n * cos(d / 180.0 * 3.14159)) + x;
              int y2 = (int)(n * sin(d / 180.0 * 3.14159)) + y;
              drawLineTorus(x, y, x2, y2, 1);
              x = x2 & (GW - 1); y = y2 & (GH - 1);
              n = 0;
              ++steps;
            }
            break;
          case 'L':
            if (n < 1) n = 1;
            d -= n;
            n = 0;
            break;
          case 'R':
            if (n < 1) n = 1;
            d += n;
            n = 0;
            break;
          case '(':
            counts[sp] = n;
            openParen[sp] = i;
            ++sp;
            n = 0;
            break;
          case ')':
            --counts[sp - 1];
            if (counts[sp - 1] < 1) {
              --sp;
            } else {
              i = openParen[sp - 1];
            }
            n = 0;
            break;
        }
    }
  }
} T;

void Scroll(const char* p[]) {
  int millis = 1000;
  while (*p) {
    for (int i = 1; i < 12; i++) {
      A.clear();
      for (int j = 0; p[j] && j < 6; j++) {
        A.setCursor(0, j * 12 - i);
        A.print(p[j]);
      }
      A.display();
      delay(millis);
      millis = 100;
      if (A.buttonsState()) return;
    }
    ++p;
  }
}

void setup() {
  A.beginNoLogo();
  A.setFrameRate(30);
#if 0
  A.setTextSize(2);
  A.setCursor(10, 0);
  A.print(" TURTLE");
  A.setCursor(10, 25);
  A.print("GRAPHICS");
  A.setTextSize(1);
  A.setCursor(0, 54);
  A.print("github.com/strickyak");
  A.display();
  delay(1000);
#endif
  Scroll(Splash);

  for (const char* p = InitialProgram; *p; p++) {
    T.PutAtI(p - InitialProgram, *p);
  }
}

char buf[2];
int i;
int startChar;
bool oldBS;
char hex[] = "0123456789ABCDEF";
int xx, yy;
int held;

void loop() {
  if (!A.nextFrame()) return;
  T.Tick();

  if (T.a_down && T.b_down) {
    // Both red butons clear the script.
    for (int i = 0; i < T.M; i++) {
      T.scr[i] = 0;
    }
    T.p = 0;
  }

  if (T.b_down) {  // DRAW PATH
    A.clear();
    T.DrawPath();
    A.display();
    return;
  }

  if (T.a_down) {  // Help
    A.clear();
    for (int i = 0; Help[i]; i++) {
      A.setCursor(0, i * 11);
      A.print(Help[i]);
    }
    A.display();
    return;
  }

  T.Draw();
}

