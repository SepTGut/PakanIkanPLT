#include "display.h"
#include "config.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long previousMillis = 0;
int displayMode = 0;

// --- Idle animation state ---
static bool     idleActive = false;
static uint8_t  idleStep = 0;
static uint8_t  idleAnimIdx = 0;
static unsigned long idleLastFrame = 0;
static unsigned long idleFrameDelay = 80;

// --- Custom character bitmaps ---
static uint8_t B_EMPTY[8]   = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static uint8_t B_L1[8]      = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F};
static uint8_t B_L2[8]      = {0x00,0x00,0x00,0x00,0x00,0x1F,0x1F,0x1F};
static uint8_t B_L3[8]      = {0x00,0x00,0x00,0x1F,0x1F,0x1F,0x1F,0x1F};
static uint8_t B_L4[8]      = {0x00,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};
static uint8_t B_FULL[8]    = {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};
static uint8_t B_CHECKER[8] = {0x15,0x0A,0x15,0x0A,0x15,0x0A,0x15,0x0A};
static uint8_t B_DOTS[8]    = {0x15,0x00,0x15,0x00,0x15,0x00,0x15,0x00};
static uint8_t B_STRIPE[8]  = {0x1F,0x00,0x1F,0x00,0x1F,0x00,0x1F,0x00};
static uint8_t B_TRI_D[8]  = {0x10,0x18,0x1C,0x1E,0x1E,0x1C,0x18,0x10};
static uint8_t B_TRI_U[8]  = {0x01,0x03,0x07,0x0F,0x0F,0x07,0x03,0x01};
static uint8_t B_CIRCLE[8] = {0x00,0x0E,0x11,0x11,0x11,0x0E,0x00,0x00};
static uint8_t B_RING[8]   = {0x0E,0x11,0x00,0x00,0x00,0x00,0x11,0x0E};
static uint8_t B_CROSS[8]  = {0x11,0x0A,0x04,0x00,0x00,0x04,0x0A,0x11};
static uint8_t B_HBAR1[8]  = {0x1F,0x00,0x00,0x00,0x00,0x00,0x00,0x1F};
static uint8_t B_HBAR2[8]  = {0x00,0x1F,0x00,0x00,0x00,0x00,0x1F,0x00};
static uint8_t B_VBAR1[8]  = {0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11};
static uint8_t B_VBAR2[8]  = {0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A};
static uint8_t B_SNAKE1[8] = {0x1F,0x01,0x01,0x01,0x01,0x01,0x01,0x01};
static uint8_t B_SNAKE2[8] = {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x1F};
static uint8_t B_SNAKE3[8] = {0x1F,0x10,0x10,0x10,0x10,0x10,0x10,0x10};
static uint8_t B_SNAKE4[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x1F};
static uint8_t B_SPARK1[8] = {0x04,0x04,0x0A,0x0A,0x11,0x11,0x00,0x00};
static uint8_t B_SPARK2[8] = {0x00,0x00,0x11,0x11,0x0A,0x0A,0x04,0x04};
static uint8_t B_WAVE1[8]  = {0x18,0x06,0x01,0x00,0x00,0x01,0x06,0x18};
static uint8_t B_WAVE2[8]  = {0x03,0x18,0x20,0x00,0x00,0x20,0x18,0x03};
static uint8_t B_STAR[8]   = {0x04,0x15,0x0E,0x1F,0x1F,0x0E,0x15,0x04};
static uint8_t B_HEART[8]  = {0x00,0x0A,0x1F,0x1F,0x1F,0x0E,0x04,0x00};
static uint8_t B_ARROW_R[8]= {0x04,0x06,0x1F,0x1F,0x1F,0x06,0x04,0x00};
static uint8_t B_ARROW_L[8]= {0x04,0x0C,0x1F,0x1F,0x1F,0x0C,0x04,0x00};
static uint8_t B_DIAMOND[8]= {0x00,0x04,0x0A,0x11,0x11,0x0A,0x04,0x00};

static unsigned long prngState = 1;
static int fastRandom(int max) {
    prngState = prngState * 1103515245UL + 12345UL;
    return (int)((prngState / 65536UL) & 0x7FFFUL) % max;
}

static void iScanBar();
static void iBlockFill();
static void iEqualizer();
static void iDiagonal();
static void iTypewriter();
static void iRain();
static void iBounce();
static void iSpinner();
static void iBoxExpand();
static void iSnake();
static void iSparkle();
static void iWave();
static void iHeartbeat();
static void iArrowMarch();
static void iDissolve();
static void iFirework();
static void iSlot();
static void iBinary();
static void iZigzag();
static void iProgress();
static void iMaze();
static void iOrbit();
static void iGlitch();
static void iCountdown();

static void clearIdleZone() {
    lcd.setCursor(8, 1);
    for (int i = 0; i < 8; i++) lcd.write(' ');
}

void startIdleAnimation() {
    idleActive = true;
    idleStep = 0;
    idleAnimIdx = fastRandom(24);
    idleLastFrame = 0;
    idleFrameDelay = 80 + fastRandom(100); // Slower base
    clearIdleZone();
}

void stopIdleAnimation() {
    idleActive = false;
    clearIdleZone();
}

bool isIdleAnimating() { return idleActive; }

void updateIdleAnimation() {
    if (!idleActive) return;
    unsigned long now = millis();
    if (now - idleLastFrame < idleFrameDelay) return;
    idleLastFrame = now;

    switch (idleAnimIdx) {
        case 0:  iScanBar();     break;
        case 1:  iBlockFill();   break;
        case 2:  iEqualizer();   break;
        case 3:  iBounce();      break;
        case 4:  iSpinner();     break;
        case 5:  iSnake();       break;
        case 6:  iSparkle();     break;
        case 7:  iWave();        break;
        case 8:  iProgress();    break;
        case 9:  iGlitch();      break;
        case 10: iZigzag();      break;
        case 11: iArrowMarch();  break;
        case 12: iDissolve();    break;
        case 13: iFirework();    break;
        case 14: iHeartbeat();   break;
        case 15: iMaze();        break;
        case 16: iOrbit();       break;
        case 17: iSlot();        break;
        case 18: iBinary();      break;
        case 19: iCountdown();   break;
        case 20: iRain();        break;
        case 21: iDiagonal();    break;
        case 22: iTypewriter();  break;
        case 23: iBoxExpand();   break;
    }

    idleStep++;
    if (idleStep > 25 + fastRandom(20)) { // Longer duration
        idleStep = 0;
        idleAnimIdx = fastRandom(24);
        idleFrameDelay = 60 + fastRandom(120); // Slower base
        clearIdleZone();
    }
}

static void iScanBar() {
    static int pos = 0, dir = 1, barLen = 3;
    if (idleStep == 0) { pos = 0; dir = 1; barLen = 2 + fastRandom(4); }
    lcd.createChar(0, B_FULL); lcd.createChar(1, B_EMPTY);
    lcd.setCursor(8, 1);
    for (int i = 0; i < 8; i++) lcd.write((i >= pos && i < pos + barLen) ? byte(0) : byte(1));
    pos += dir;
    if (pos <= 0 || pos + barLen >= 8) { dir = -dir; pos += dir; }
}

static void iBlockFill() {
    lcd.createChar(0, B_FULL); lcd.createChar(1, B_L3);
    lcd.setCursor(8 + fastRandom(8), 1);
    lcd.write(fastRandom(2) == 0 ? byte(0) : byte(1));
}

static void iEqualizer() {
    lcd.createChar(0, B_EMPTY); lcd.createChar(1, B_L1); lcd.createChar(2, B_L2);
    lcd.createChar(3, B_L3); lcd.createChar(4, B_L4); lcd.createChar(5, B_FULL);
    lcd.setCursor(8, 1);
    for (int c = 0; c < 8; c++) lcd.write(byte(fastRandom(6)));
}

static void iBounce() {
    static int pos = 0, vel = 1;
    if (idleStep == 0) { pos = 0; vel = 1; }
    lcd.createChar(0, B_CIRCLE);
    lcd.setCursor(8 + pos, 1); lcd.write(byte(0));
    int prev = pos - vel;
    if (prev >= 0 && prev < 8 && prev != pos) { lcd.setCursor(8 + prev, 1); lcd.write(' '); }
    pos += vel;
    if (pos <= 0 || pos >= 7) { vel = -vel; pos += vel; }
}

static void iSpinner() {
    static uint8_t frames[4][8] = {
        {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x1F},
        {0x00,0x00,0x00,0x1F,0x1F,0x00,0x00,0x00},
        {0x1F,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
        {0x00,0x00,0x00,0x1F,0x1F,0x00,0x00,0x00},
    };
    static int col = 11;
    if (idleStep == 0) col = 8 + fastRandom(6) + 1;
    uint8_t tmp[8]; memcpy(tmp, frames[idleStep % 4], 8);
    lcd.createChar(0, tmp);
    lcd.setCursor(col, 1); lcd.write(byte(0));
}

static void iSnake() {
    static int pos = 0, dir = 1;
    if (idleStep == 0) { pos = 0; dir = 1; }
    lcd.createChar(0, B_SNAKE1); lcd.createChar(1, B_SNAKE3);
    lcd.setCursor(8 + pos, 1);
    lcd.write(dir > 0 ? byte(0) : byte(1));
    int tail = pos - dir;
    if (tail >= 0 && tail < 8) { lcd.setCursor(8 + tail, 1); lcd.write(' '); }
    pos += dir;
    if (pos >= 8 || pos < 0) { dir = -dir; pos += dir; }
}

static void iSparkle() {
    lcd.createChar(0, B_SPARK1); lcd.createChar(1, B_SPARK2); lcd.createChar(2, B_STAR);
    lcd.setCursor(8 + fastRandom(8), 1);
    lcd.write(byte(fastRandom(3)));
    if (fastRandom(3) == 0) { lcd.setCursor(8 + fastRandom(8), 1); lcd.write(' '); }
}

static void iWave() {
    lcd.createChar(0, B_WAVE1); lcd.createChar(1, B_WAVE2);
    lcd.setCursor(8, 1);
    for (int c = 0; c < 8; c++) lcd.write(byte((c + idleStep) % 2 == 0 ? 0 : 1));
}

static void iProgress() {
    static int pos = 0, dir = 1;
    if (idleStep == 0) { pos = 0; dir = 1; }
    lcd.createChar(0, B_L1); lcd.createChar(1, B_L2); lcd.createChar(2, B_L3);
    lcd.createChar(3, B_L4); lcd.createChar(4, B_FULL);
    lcd.setCursor(8, 1);
    for (int c = 0; c < 8; c++) lcd.write(byte(c <= pos ? min(c / 2, 4) : 0));
    pos += dir;
    if (pos >= 7 || pos <= 0) { dir = -dir; pos += dir; }
}

static void iGlitch() {
    lcd.createChar(0, B_CHECKER); lcd.createChar(1, B_DOTS);
    lcd.createChar(2, B_STRIPE); lcd.createChar(3, B_FULL);
    lcd.setCursor(8, 1);
    for (int c = 0; c < 8; c++) lcd.write(byte(fastRandom(4)));
}

static void iZigzag() {
    lcd.createChar(0, B_TRI_D); lcd.createChar(1, B_TRI_U);
    lcd.setCursor(8, 1);
    for (int c = 0; c < 8; c++) lcd.write(byte((c + idleStep) % 4 < 2 ? 0 : 1));
}

static void iArrowMarch() {
    static int pos = 0, dir = 1;
    if (idleStep == 0) { pos = 0; dir = 1; }
    lcd.createChar(0, B_ARROW_R); lcd.createChar(1, B_ARROW_L);
    lcd.setCursor(8 + pos, 1);
    lcd.write(dir > 0 ? byte(0) : byte(1));
    int tail = pos - dir;
    if (tail >= 0 && tail < 8) { lcd.setCursor(8 + tail, 1); lcd.write(' '); }
    pos += dir;
    if (pos >= 8 || pos < 0) { dir = -dir; pos += dir; }
}

static void iDissolve() {
    static bool filled = false;
    if (idleStep == 0) filled = false;
    if (!filled) {
        lcd.createChar(0, B_FULL);
        lcd.setCursor(8, 1);
        for (int c = 0; c < 8; c++) lcd.write(byte(0));
        filled = true;
    } else {
        lcd.createChar(0, B_CHECKER);
        lcd.setCursor(8, 1);
        for (int c = 0; c < 8; c++) lcd.write(byte(0));
        lcd.setCursor(8 + fastRandom(8), 1); lcd.write(' ');
    }
}

static void iFirework() {
    static int phase = 0;
    static int cx = 11;
    if (idleStep == 0) { phase = 0; cx = 8 + fastRandom(6) + 1; }
    lcd.createChar(0, B_SPARK1); lcd.createChar(1, B_SPARK2); lcd.createChar(2, B_STAR);
    if (phase == 0) {
        lcd.setCursor(cx, 1); lcd.write(byte(2));
    } else if (phase < 4) {
        int bx[4] = {cx-1, cx, cx+1, cx};
        for (int i = 0; i < 4; i++) {
            if (bx[i] >= 8 && bx[i] < 16) {
                lcd.setCursor(bx[i], 1);
                lcd.write(byte(phase % 2 == 0 ? 0 : 1));
            }
        }
    } else {
        for (int dx = -1; dx <= 1; dx++) {
            int c = cx + dx;
            if (c >= 8 && c < 16) { lcd.setCursor(c, 1); lcd.write(' '); }
        }
    }
    phase++;
    if (phase > 5) phase = 0;
}

static void iHeartbeat() {
    static int beat = 0;
    if (idleStep == 0) beat = 0;
    lcd.createChar(0, B_HEART);
    int col = 11;
    if (beat % 3 == 0) { lcd.setCursor(col, 1); lcd.write(byte(0)); }
    else if (beat % 3 == 1) { lcd.setCursor(col, 1); lcd.write(byte(0)); }
    else { lcd.setCursor(col, 1); lcd.write(' '); }
    beat++;
}

static void iMaze() {
    static int s = 0;
    if (idleStep == 0) s = 0;
    lcd.createChar(0, B_HBAR1); lcd.createChar(1, B_VBAR1);
    if (s < 2) {
        lcd.setCursor(8 + s * 4, 1); lcd.write(byte(0));
    } else {
        lcd.setCursor(8 + (s - 2) * 3 + 1, 1); lcd.write(byte(1));
    }
    s++;
    if (s > 4) s = 0;
}

static void iOrbit() {
    static int cx = 11, angle = 0;
    if (idleStep == 0) { cx = 8 + fastRandom(6) + 1; angle = 0; }
    lcd.createChar(0, B_CIRCLE);
    static int prevCol = -1;
    int col = cx;
    if (angle < 2) col = cx - 1 + angle;
    else if (angle < 5) col = cx + (angle - 2);
    else col = cx + (7 - angle);
    if (prevCol >= 8 && prevCol < 16 && prevCol != col) { lcd.setCursor(prevCol, 1); lcd.write(' '); }
    if (col >= 8 && col < 16) { lcd.setCursor(col, 1); lcd.write(byte(0)); }
    prevCol = col;
    angle = (angle + 1) % 8;
}

static void iSlot() {
    static int col = 10;
    if (idleStep == 0) col = 8 + fastRandom(5) + 1;
    lcd.createChar(0, B_DIAMOND); lcd.createChar(1, B_HEART); lcd.createChar(2, B_STAR);
    lcd.createChar(3, B_CIRCLE); lcd.createChar(4, B_CROSS);
    for (int s = 0; s < 3 && col + s < 16; s++) { lcd.setCursor(col + s, 1); lcd.write(byte(fastRandom(5))); }
}

static void iBinary() {
    static int val = 0;
    if (idleStep == 0) val = 0;
    lcd.createChar(0, B_FULL); lcd.createChar(1, B_EMPTY);
    lcd.setCursor(8, 1);
    for (int b = 0; b < 8; b++) lcd.write((val & (1 << ((7-b)%4))) ? byte(0) : byte(1));
    val = (val + 1) & 0xF;
}

static void iCountdown() {
    static int num = 5;
    if (idleStep == 0) num = 5;
    lcd.createChar(0, B_FULL); lcd.createChar(1, B_L4); lcd.createChar(2, B_L3);
    lcd.createChar(3, B_L2); lcd.createChar(4, B_L1);
    lcd.setCursor(8, 1);
    for (int c = 0; c < 8; c++) lcd.write(byte(c < num * 2 && num > 0 ? (num > 4 ? 0 : (5-num)) : 0));
    num--;
    if (num < 0) num = 5;
}

static void iRain() {
    static int cols[4];
    static int rows[4];
    static bool inited = false;
    if (!inited) {
        for (int i = 0; i < 4; i++) {
            cols[i] = 8 + fastRandom(8);
            rows[i] = fastRandom(2);
        }
        inited = true;
    }
    lcd.createChar(0, B_L1);
    for (int i = 0; i < 4; i++) {
        // Clear previous position
        lcd.setCursor(cols[i], rows[i]);
        lcd.write(' ');
        // Move raindrop down (wrap around)
        rows[i]++;
        if (rows[i] > 1) {
            rows[i] = 0;
            cols[i] = 8 + fastRandom(8);
        }
        // Draw at new position
        lcd.setCursor(cols[i], rows[i]);
        lcd.write(byte(0));
    }
}

static void iDiagonal() {
    static int d = 0;
    if (idleStep == 0) d = 0;
    lcd.createChar(0, B_FULL); lcd.createChar(1, B_CHECKER);
    lcd.setCursor(8, 1);
    for (int c = 0; c < 8; c++) lcd.write(byte(c == d ? 0 : (c == d+1 ? 1 : 0)));
    d++;
    if (d > 7) d = 0;
}

static void iTypewriter() {
    static int idx = 0;
    static const char* txt = "SetGT";
    static int col = 8;
    if (idleStep == 0) { idx = 0; col = 8 + fastRandom(11); }
    if (idx < 5 && col + idx < 16) {
        lcd.setCursor(col + idx, 1); lcd.print(txt[idx]); idx++;
    }
}

static void iBoxExpand() {
    static int phase = 0, dir = 1;
    if (idleStep == 0) { phase = 0; dir = 1; }
    lcd.createChar(0, B_FULL); lcd.createChar(1, B_HBAR1); lcd.createChar(2, B_HBAR2);
    lcd.createChar(3, B_VBAR1);
    clearIdleZone();
    if (phase == 0) {
        lcd.setCursor(11, 1); lcd.write(byte(0));
    } else if (phase == 1) {
        for (int c = 9; c <= 14; c++) { lcd.setCursor(c, 1); lcd.write(byte(1)); }
        lcd.setCursor(9, 1); lcd.write(byte(3));
        lcd.setCursor(14, 1); lcd.write(byte(3));
    } else {
        for (int c = 8; c <= 15; c++) { lcd.setCursor(c, 1); lcd.write(byte(0)); }
    }
    phase += dir;
    if (phase >= 3 || phase <= 0) { dir = -dir; phase += dir; }
}

static void animScanBar() {
    lcd.createChar(0, B_FULL); lcd.createChar(1, B_EMPTY);
    int row = fastRandom(2), barLen = 4 + fastRandom(5), startCol = fastRandom(16 - barLen);
    lcd.setCursor(0, row);
    for (int i = 0; i < 16; i++) lcd.write((i >= startCol && i < startCol + barLen) ? byte(0) : byte(1));
    delay(150 + fastRandom(250));
    lcd.setCursor(0, row); for (int i = 0; i < 16; i++) lcd.write(' ');
}

static void animBlockFill() {
    lcd.createChar(0, B_FULL); lcd.createChar(1, B_L3);
    for (int i = 0; i < 6 + fastRandom(10); i++) { lcd.setCursor(fastRandom(16), fastRandom(2)); lcd.write(fastRandom(2)==0?byte(0):byte(1)); delay(60+fastRandom(120)); }
}

static void animEqualizer() {
    lcd.createChar(0,B_EMPTY); lcd.createChar(1,B_L1); lcd.createChar(2,B_L2); lcd.createChar(3,B_L3); lcd.createChar(4,B_L4); lcd.createChar(5,B_FULL);
    for (int r=0;r<2;r++) { lcd.setCursor(0,r); for (int c=0;c<16;c++) lcd.write(byte(1+fastRandom(6))); }
    delay(300+fastRandom(400));
    for (int r=0;r<2;r++) { lcd.setCursor(0,r); for (int c=0;c<16;c++) lcd.write(byte(fastRandom(3))); }
    delay(200+fastRandom(300));
}

static void animDiagonal() {
    lcd.createChar(0,B_FULL); lcd.createChar(1,B_CHECKER);
    for (int p=0;p<2+fastRandom(3);p++) { int dir=fastRandom(2); for (int d=-1;d<17;d++) { for (int r=0;r<2;r++) for (int c=0;c<16;c++) if((dir?(r*2+c):(c-r))==d){lcd.setCursor(c,r);lcd.write(byte(fastRandom(2)));} delay(40); } }
}

static void animTypewriter(const char* text) {
    int len=strlen(text), row=fastRandom(2), sc=fastRandom(16-min(len,16));
    for (int i=0;i<min(len,16-sc);i++) { lcd.setCursor(sc+i,row); lcd.print(text[i]); delay(100+fastRandom(150)); }
    delay(400);
    for (int i=sc;i<sc+min(len,16-sc);i++) { lcd.setCursor(i,row); lcd.print(' '); delay(60); }
}

static void animRain() {
    lcd.createChar(0,B_L1); int col[8]; for(int i=0;i<8;i++) col[i]=fastRandom(16);
    for(int step=0;step<8;step++){for(int i=0;i<8;i++){lcd.setCursor(col[i],(step+i)%2);lcd.write(byte(0));}delay(120+fastRandom(150));for(int i=0;i<8;i++){lcd.setCursor(col[i],(step+i)%2);lcd.write(' ');}}
}

static void animBounce() {
    lcd.createChar(0,B_CIRCLE); int pos=fastRandom(14)+1,vel=(fastRandom(2)==0)?1:-1;
    for(int step=0;step<20;step++){int row=(step/3)%2;lcd.setCursor(pos,row);lcd.write(byte(0));delay(100);lcd.setCursor(pos,row);lcd.write(' ');pos+=vel;if(pos<=0||pos>=15){vel=-vel;pos+=vel;}}
}

static void animSpinner() {
    uint8_t frames[4][8]={{0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x1F},{0x00,0x00,0x00,0x1F,0x1F,0x00,0x00,0x00},{0x1F,0x10,0x10,0x10,0x10,0x10,0x10,0x10},{0x00,0x00,0x00,0x1F,0x1F,0x00,0x00,0x00}};
    int row=fastRandom(2),col=fastRandom(14)+1;
    for(int i=0;i<12;i++){uint8_t tmp[8];memcpy(tmp,frames[i%4],8);lcd.createChar(0,tmp);lcd.setCursor(col,row);lcd.write(byte(0));delay(120+fastRandom(100));}
    lcd.setCursor(col,row);lcd.write(' ');
}

static void animBoxExpand() {
    lcd.createChar(0,B_FULL);lcd.createChar(1,B_HBAR1);lcd.createChar(2,B_HBAR2);lcd.createChar(3,B_VBAR1);lcd.createChar(4,B_VBAR2);
    for(int ph=0;ph<3;ph++){lcd.clear();if(ph==0){lcd.setCursor(7,0);lcd.write(byte(0));lcd.setCursor(7,1);lcd.write(byte(0));}else if(ph==1){for(int c=5;c<=10;c++){lcd.setCursor(c,0);lcd.write(byte(1));}for(int c=5;c<=10;c++){lcd.setCursor(c,1);lcd.write(byte(2));}lcd.setCursor(5,0);lcd.write(byte(3));lcd.setCursor(10,0);lcd.write(byte(3));lcd.setCursor(5,1);lcd.write(byte(4));lcd.setCursor(10,1);lcd.write(byte(4));}else{for(int c=3;c<=12;c++)for(int r=0;r<2;r++){lcd.setCursor(c,r);lcd.write(byte(0));}}delay(250+fastRandom(250));}
    for(int ph=2;ph>=0;ph--){lcd.clear();if(ph==0){lcd.setCursor(7,0);lcd.write(byte(0));lcd.setCursor(7,1);lcd.write(byte(0));}else if(ph==1){for(int c=5;c<=10;c++){lcd.setCursor(c,0);lcd.write(byte(1));}for(int c=5;c<=10;c++){lcd.setCursor(c,1);lcd.write(byte(2));}lcd.setCursor(5,0);lcd.write(byte(3));lcd.setCursor(10,0);lcd.write(byte(3));lcd.setCursor(5,1);lcd.write(byte(4));lcd.setCursor(10,1);lcd.write(byte(4));}else{for(int c=3;c<=12;c++)for(int r=0;r<2;r++){lcd.setCursor(c,r);lcd.write(byte(0));}}delay(250+fastRandom(250));}
    lcd.clear();
}

static void animSnake() {
    lcd.createChar(0,B_SNAKE1);lcd.createChar(1,B_SNAKE2);lcd.createChar(2,B_SNAKE3);lcd.createChar(3,B_SNAKE4);
    int row=fastRandom(2);
    for(int c=0;c<16;c++){lcd.setCursor(c,row);lcd.write(byte(0));delay(80);lcd.setCursor(c,row);lcd.write(' ');}
    for(int c=15;c>=0;c--){lcd.setCursor(c,row);lcd.write(byte(2));delay(80);lcd.setCursor(c,row);lcd.write(' ');}
}

static void animSparkle() {
    lcd.createChar(0,B_SPARK1);lcd.createChar(1,B_SPARK2);lcd.createChar(2,B_STAR);
    for(int i=0;i<8+fastRandom(8);i++){int c=fastRandom(16),r=fastRandom(2);lcd.createChar(i%3,(i%3==0)?B_SPARK1:((i%3==1)?B_SPARK2:B_STAR));lcd.setCursor(c,r);lcd.write(byte(i%3));delay(70+fastRandom(120));lcd.setCursor(c,r);lcd.write(' ');}
}

static void animWave() {
    lcd.createChar(0,B_WAVE1);lcd.createChar(1,B_WAVE2);
    for(int ph=0;ph<4;ph++){for(int r=0;r<2;r++){lcd.setCursor(0,r);for(int c=0;c<16;c++)lcd.write(byte((c+ph+r)%2==0?0:1));}delay(200+fastRandom(200));}
}

static void animHeartbeat() {
    lcd.createChar(0,B_HEART); int col=fastRandom(14)+1;
    for(int b=0;b<3;b++){lcd.setCursor(col,0);lcd.write(byte(0));lcd.setCursor(col,1);lcd.write(' ');delay(250);lcd.setCursor(col,1);lcd.write(byte(0));delay(250);lcd.setCursor(col,0);lcd.write(' ');lcd.setCursor(col,1);lcd.write(' ');delay(200);}
}

static void animArrowMarch() {
    lcd.createChar(0,B_ARROW_R);lcd.createChar(1,B_ARROW_L); int row=fastRandom(2);
    for(int c=0;c<16;c++){lcd.setCursor(c,row);lcd.write(byte(0));delay(70);lcd.setCursor(c,row);lcd.write(' ');}
    for(int c=15;c>=0;c--){lcd.setCursor(c,row);lcd.write(byte(1));delay(70);lcd.setCursor(c,row);lcd.write(' ');}
}

static void animDissolve() {
    lcd.createChar(0,B_FULL);lcd.createChar(1,B_CHECKER);
    for(int r=0;r<2;r++){lcd.setCursor(0,r);for(int c=0;c<16;c++)lcd.write(byte(0));}delay(250);
    for(int r=0;r<2;r++){lcd.setCursor(0,r);for(int c=0;c<16;c++)lcd.write(byte(1));}delay(250);
    int order[32];for(int i=0;i<32;i++)order[i]=i;for(int i=31;i>0;i--){int j=fastRandom(i+1);int tmp=order[i];order[i]=order[j];order[j]=tmp;}
    for(int i=0;i<32;i++){lcd.setCursor(order[i]%16,order[i]/16);lcd.write(' ');delay(40+fastRandom(60));}
}

static void animFirework() {
    lcd.createChar(0,B_SPARK1);lcd.createChar(1,B_SPARK2);lcd.createChar(2,B_STAR);
    int cx=fastRandom(12)+2;
    for(int r=1;r>=0;r--){lcd.setCursor(cx,r);lcd.write(byte(2));delay(120);lcd.setCursor(cx,r);lcd.write(' ');}
    int bx[6]={cx-2,cx-1,cx,cx+1,cx+2,cx},by[6]={0,1,0,1,0,1};
    for(int f=0;f<3;f++){for(int i=0;i<6;i++)if(bx[i]>=0&&bx[i]<16){lcd.createChar(i%3,(f%2==0)?B_SPARK1:B_SPARK2);lcd.setCursor(bx[i],by[i]);lcd.write(byte(i%3));}delay(150);}
    for(int i=0;i<6;i++)if(bx[i]>=0&&bx[i]<16){lcd.setCursor(bx[i],by[i]);lcd.write(' ');}
}

static void animSlotMachine() {
    lcd.createChar(0,B_DIAMOND);lcd.createChar(1,B_HEART);lcd.createChar(2,B_STAR);lcd.createChar(3,B_CIRCLE);lcd.createChar(4,B_CROSS);
    int row=fastRandom(2),col=fastRandom(13)+1;
    for(int sp=0;sp<12;sp++){for(int s=0;s<3;s++){lcd.setCursor(col+s,row);lcd.write(byte(fastRandom(5)));}delay(80+fastRandom(100));}
    int fin=fastRandom(5);for(int s=0;s<3;s++){lcd.setCursor(col+s,row);lcd.write(byte(fin));}delay(400);
    for(int s=0;s<3;s++){lcd.setCursor(col+s,row);lcd.write(' ');}
}

static void animBinaryCounter() {
    lcd.createChar(0,B_FULL);lcd.createChar(1,B_EMPTY); int row=fastRandom(2);
    for(int v=0;v<16;v++){lcd.setCursor(0,row);for(int b=0;b<16;b++)lcd.write((v&(1<<((15-b)%4)))?byte(0):byte(1));delay(100+fastRandom(120));}
    lcd.setCursor(0,row);for(int i=0;i<16;i++)lcd.write(' ');
}

static void animZigzag() {
    lcd.createChar(0,B_TRI_D);lcd.createChar(1,B_TRI_U);
    for(int o=0;o<4;o++){for(int r=0;r<2;r++){lcd.setCursor(0,r);for(int c=0;c<16;c++)lcd.write((c+o+r)%4<2?byte(0):byte(1));}delay(150+fastRandom(200));}
}

static void animProgress() {
    lcd.createChar(0,B_L1);lcd.createChar(1,B_L2);lcd.createChar(2,B_L3);lcd.createChar(3,B_L4);lcd.createChar(4,B_FULL);
    int row=fastRandom(2);
    for(int c=0;c<16;c++){for(int lv=0;lv<5;lv++){lcd.setCursor(c,row);lcd.write(byte(lv));delay(30);}}delay(300);
    for(int c=15;c>=0;c--){lcd.setCursor(c,row);lcd.write(' ');delay(40);}
}

static void animMaze() {
    lcd.createChar(0,B_VBAR1);lcd.createChar(1,B_HBAR1);
    for(int c=2;c<16;c+=3){int r=fastRandom(2),h=1+fastRandom(2);for(int i=0;i<h&&(r+i)<2;i++){lcd.setCursor(c,r+i);lcd.write(byte(0));delay(80);}}
    for(int r=0;r<2;r++){for(int c=0;c<16;c+=4){int w=2+fastRandom(3);for(int i=0;i<w&&(c+i)<16;i++){lcd.setCursor(c+i,r);lcd.write(byte(1));delay(60);}}}delay(400);lcd.clear();
}

static void animOrbit() {
    lcd.createChar(0,B_CIRCLE);lcd.createChar(1,B_RING);
    int cx=fastRandom(12)+2,cy=fastRandom(2);lcd.setCursor(cx,cy);lcd.write(byte(1));
    int o[8][2]={{cx-1,cy},{cx-1,cy-1},{cx,cy-1},{cx+1,cy-1},{cx+1,cy},{cx+1,cy+1},{cx,cy+1},{cx-1,cy+1}};
    for(int l=0;l<2;l++)for(int i=0;i<8;i++){int oc=o[i][0],or_=o[i][1];if(oc>=0&&oc<16&&or_>=0&&or_<2){lcd.setCursor(oc,or_);lcd.write(byte(0));}delay(100);if(oc>=0&&oc<16&&or_>=0&&or_<2){lcd.setCursor(oc,or_);lcd.write(' ');}}
    lcd.setCursor(cx,cy);lcd.write(' ');
}

static void animGlitch() {
    lcd.createChar(0,B_CHECKER);lcd.createChar(1,B_DOTS);lcd.createChar(2,B_STRIPE);lcd.createChar(3,B_FULL);
    for(int f=0;f<6;f++){for(int r=0;r<2;r++){lcd.setCursor(0,r);for(int c=0;c<16;c++)lcd.write(byte(fastRandom(4)));}delay(70+fastRandom(120));}lcd.clear();
}

static void animCountdown() {
    lcd.createChar(0,B_FULL);lcd.createChar(1,B_L4);lcd.createChar(2,B_L3);lcd.createChar(3,B_L2);lcd.createChar(4,B_L1);
    int row=fastRandom(2);
    for(int n=5;n>=0;n--){lcd.setCursor(0,row);for(int c=0;c<16;c++)lcd.write(byte(c<n*3&&n>0?(n>4?0:(5-n)):0));delay(250);}
    lcd.setCursor(0,row);for(int c=0;c<16;c++)lcd.write(' ');
}

static void playRandomAnimation() {
    prngState = (unsigned long)micros() + 1UL;
    switch (fastRandom(24)) {
        case 0: animScanBar(); break; case 1: animBlockFill(); break; case 2: animEqualizer(); break;
        case 3: animDiagonal(); break; case 4: animTypewriter("SetGT"); break; case 5: animRain(); break;
        case 6: animBounce(); break; case 7: animSpinner(); break; case 8: animBoxExpand(); break;
        case 9: animSnake(); break; case 10: animSparkle(); break; case 11: animWave(); break;
        case 12: animHeartbeat(); break; case 13: animArrowMarch(); break; case 14: animDissolve(); break;
        case 15: animFirework(); break; case 16: animSlotMachine(); break; case 17: animBinaryCounter(); break;
        case 18: animZigzag(); break; case 19: animProgress(); break; case 20: animMaze(); break;
        case 21: animOrbit(); break; case 22: animGlitch(); break; case 23: animCountdown(); break;
    }
}

void showCopyright() {
    lcd.clear(); lcd.setCursor(4, 0); lcd.print("Made By"); lcd.setCursor(5, 1); lcd.print("SetGT");
}

void playCopyrightAnimation() {
    lcd.noBacklight(); 
    delay(200); 
    int rounds = 3 + fastRandom(3);
    for (int i = 0; i < rounds; i++) playRandomAnimation();
    lcd.backlight(); 
    lcd.clear(); 
    showCopyright(); 
    delay(1500); 
    lcd.clear();
}

void showRTCError() {
    lcd.clear(); lcd.setCursor(0, 0); lcd.print("RTC Error!"); lcd.setCursor(0, 1); lcd.print("Check Hardware");
    Serial.println("RTC Error! Check Hardware (display message)");
}

void showError(const char* line1, const char* line2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    if (line2) {
        lcd.setCursor(0, 1);
        lcd.print(line2);
    }
}

void initDisplay() { 
    lcd.init(); 
    lcd.backlight(); 
}

void updateDisplay(const TimeData& time) {
    unsigned long currentMillis = millis();
    static int lastSecond = -1, lastMode = -1;

    if (currentMillis - previousMillis >= DISPLAY_INTERVAL) {
        previousMillis = currentMillis; displayMode++;
        if (displayMode > NUM_SESSIONS) displayMode = 0;
    }

    if (displayMode != lastMode) {
        lcd.setCursor(0, 0);
        char tempBuffer[20], line1Buffer[17];
        if (displayMode == 0) {
            snprintf(tempBuffer, sizeof(tempBuffer), "%.3s,%02d/%02d/%d", time.dayName, time.day, time.month, time.year);
        } else {
            int si = displayMode - 1;
            snprintf(tempBuffer, sizeof(tempBuffer), "%s: %02d:%02d", SCHEDULE[si].label, SCHEDULE[si].hour, SCHEDULE[si].minute);
        }
        snprintf(line1Buffer, sizeof(line1Buffer), "%-16s", tempBuffer);
        lcd.print(line1Buffer);
        lastMode = displayMode;
    }

    if (time.second != lastSecond) {
        lcd.setCursor(0, 1);
        char timeStr[9];
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", time.hour, time.minute, time.second);
        lcd.print(timeStr);
        lastSecond = time.second;
    }
}
