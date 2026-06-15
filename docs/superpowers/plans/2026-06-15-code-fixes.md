# Code Fixes Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix 8 identified bugs and code quality issues from the code review: buzzer duration ignored, RTC error misused for low-food alerts, missed-feed logic returns wrong session, broken rain animation, dead code, global scope leak, and display row not cleared.

**Architecture:** Each fix is independent and isolated to its module. Fixes are ordered: alerts → feeding → display → main. No new files needed — only edits to existing ones.

**Tech Stack:** Arduino (ESP32/Uno), PlatformIO, C++

---

### Task 1: Fix `handleBuzzer()` to use the `duration` parameter

**Files:**
- Modify: `src/alerts.h:6` — change `triggerAlert` signature to store duration
- Modify: `src/alerts.cpp` — rewrite buzzer timing to use stored duration

- [ ] **Step 1: Update `alerts.h` — change `triggerAlert` to accept duration as `unsigned long`**

Replace the forward declaration area. The header already declares `void triggerAlert(int buzzer, unsigned long duration);` — no change needed to the header. Move to step 2.

- [ ] **Step 2: Update `alerts.cpp` — store duration and use it in `handleBuzzer()`**

Replace the entire file content:

```cpp
#include "alerts.h"
#include "config.h"

static unsigned long alertStartTime = 0;
static unsigned long alertDuration = 0;
static bool alertActive = false;
static int alertBuzzer = 1;

void triggerAlert(int buzzer, unsigned long duration) {
    if (!ENABLE_BUZZERS) return;

    alertBuzzer = buzzer;
    alertDuration = duration;
    alertStartTime = millis();
    alertActive = true;
}

void handleBuzzer() {
    if (!ENABLE_BUZZERS || !alertActive) return;

    unsigned long elapsed = millis() - alertStartTime;
    if (elapsed < alertDuration) {
        // Beep pattern: ON for first half of duration, OFF for second half
        unsigned long halfDuration = alertDuration / 2;
        if (elapsed < halfDuration) {
            digitalWrite(alertBuzzer == 1 ? BUZZER_1_PIN : BUZZER_2_PIN, HIGH);
        } else {
            digitalWrite(alertBuzzer == 1 ? BUZZER_1_PIN : BUZZER_2_PIN, LOW);
        }
    } else {
        digitalWrite(alertBuzzer == 1 ? BUZZO_1_PIN : BUZZER_2_PIN, LOW);
        alertActive = false;
    }
}
```

Wait — there's a typo. Let me fix that. Replace the entire `src/alerts.cpp` with:

```cpp
#include "alerts.h"
#include "config.h"

static unsigned long alertStartTime = 0;
static unsigned long alertDuration = 0;
static bool alertActive = false;
static int alertBuzzer = 1;

void triggerAlert(int buzzer, unsigned long duration) {
    if (!ENABLE_BUZZERS) return;

    alertBuzzer = buzzer;
    alertDuration = duration;
    alertStartTime = millis();
    alertActive = true;
}

void handleBuzzer() {
    if (!ENABLE_BUZZERS || !alertActive) return;

    unsigned long elapsed = millis() - alertStartTime;
    if (elapsed < alertDuration) {
        // Beep pattern: ON for first half, OFF for second half
        if (elapsed < alertDuration / 2) {
            digitalWrite(alertBuzzer == 1 ? BUZZER_1_PIN : BUZZER_2_PIN, HIGH);
        } else {
            digitalWrite(alertBuzzer == 1 ? BUZZER_1_PIN : BUZZER_2_PIN, LOW);
        }
    } else {
        digitalWrite(alertBuzzer == 1 ? BUZZER_1_PIN : BUZZER_2_PIN, LOW);
        alertActive = false;
    }
}
```

- [ ] **Step 3: Verify no other files need updating**

`triggerAlert()` callers in `main.cpp` and `feeding.cpp` already pass `(int, unsigned long)` — the signature is unchanged. No callers need updating.

- [ ] **Step 4: Commit**

```bash
git add src/alerts.cpp
git commit -m "fix: handleBuzzer now respects the duration parameter

Previously, triggerAlert() accepted a duration but handleBuzzer()
always used a hardcoded 400ms window. Now the buzzer stays ON for
the first half of the requested duration and OFF for the second half,
then deactivates."
```

---

### Task 2: Add `showError()` function and use it for low-food alert

**Files:**
- Modify: `src/display.h:11` — add `showError()` declaration
- Modify: `src/display.cpp` — add `showError()` implementation
- Modify: `src/feeding.cpp:46-47` — replace `showRTCError()` with `showError()`
- Modify: `src/display.h` — remove `showRTCError()` misuse

- [ ] **Step 1: Add `showError()` to `display.h`**

In `src/display.h`, replace line 11 (`void showRTCError();`) with:

```cpp
void showRTCError();
void showError(const char* line1, const char* line2 = nullptr);
```

- [ ] **Step 2: Add `showError()` implementation to `display.cpp`**

Add this function after `showRTCError()` (after line 599):

```cpp
void showError(const char* line1, const char* line2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    if (line2) {
        lcd.setCursor(0, 1);
        lcd.print(line2);
    }
}
```

- [ ] **Step 3: Fix `feeding.cpp` — replace `showRTCError()` with `showError()` for low-food alert**

In `src/feeding.cpp`, line 46-47, replace:
```cpp
showRTCError(); // Using this for general hardware errors for now, or custom alert
```
with:
```cpp
showError("Food Low!", "Refill hopper");
```

- [ ] **Step 4: Commit**

```bash
git add src/display.h src/display.cpp src/feeding.cpp
git commit -m "fix: add showError() and use for low-food alert instead of showRTCError

Previously, low-food detection called showRTCError() which displayed
'RTC Error! Check Hardware' — misleading. Now uses a dedicated
showError() function with appropriate 'Food Low!' message."
```

---

### Task 3: Fix `checkMissedFeeds()` to return the first missed session

**Files:**
- Modify: `src/feeding.cpp:105-126` — fix logic to return earliest missed session

- [ ] **Step 1: Replace `checkMissedFeeds()` in `src/feeding.cpp`**

Replace lines 105-126 with:

```cpp
int checkMissedFeeds(TimeData time) {
    FeedingState last = loadState();

    bool todaySame = (last.day == (uint8_t)time.day &&
                        last.month == (uint8_t)time.month &&
                        last.year == (uint16_t)time.year);

    // If no feeding has ever been recorded (session == 255 means None),
    // return the first session that has passed.
    if (last.session == 255) {
        for (int s = 0; s < NUM_SESSIONS; s++) {
            if (time.hour > SCHEDULE[s].hour ||
                (time.hour == SCHEDULE[s].hour && time.minute >= SCHEDULE[s].minute)) {
                return s;  // Return earliest missed session
            }
        }
        return -1;
    }

    // If last feeding was on a different day, find first session that has passed
    if (!todaySame) {
        for (int s = 0; s < NUM_SESSIONS; s++) {
            if (time.hour > SCHEDULE[s].hour ||
                (time.hour == SCHEDULE[s].hour && time.minute >= SCHEDULE[s].minute)) {
                return s;  // Return earliest missed session
            }
        }
        return -1;
    }

    // Same day: find first session after the last fed session that has passed
    for (int s = last.session + 1; s < NUM_SESSIONS; s++) {
        if (time.hour > SCHEDULE[s].hour ||
            (time.hour == SCHEDULE[s].hour && time.minute >= SCHEDULE[s].minute)) {
            return s;  // Return earliest missed session after last fed
        }
    }

    return -1;
}
```

- [ ] **Step 2: Commit**

```bash
git add src/feeding.cpp
git commit -m "fix: checkMissedFeeds now returns earliest missed session

Previously, the function iterated all sessions and overwrote the
result, returning the LAST missed session instead of the FIRST.
This caused missed early feedings (e.g. Pagi at 6:00) to be skipped
when multiple sessions were missed. Now returns the earliest."
```

---

### Task 4: Fix `iRain()` broken column tracking

**Files:**
- Modify: `src/display.cpp:359-370` — fix rain animation column tracking

- [ ] **Step 1: Replace `iRain()` in `src/display.cpp`**

Replace lines 359-370 with:

```cpp
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
```

- [ ] **Step 2: Commit**

```bash
git add src/display.cpp
git commit -m "fix: iRain animation column tracking

Previously, iRain() computed 'prevC' with a formula that didn't
track the actual previous position, leaving ghost characters on the
display. Now properly tracks each raindrop's row and column,
clearing only the actual previous position."
```

---

### Task 5: Remove dead `playAlert()` forward declaration

**Files:**
- Modify: `src/feeding.cpp:13` — remove unused forward declaration

- [ ] **Step 1: Remove dead code from `src/feeding.cpp`**

Delete line 13:
```cpp
void playAlert(int buzzerNum, int durationMs);
```

- [ ] **Step 2: Commit**

```bash
git add src/feeding.cpp
git commit -m "chore: remove unused playAlert() forward declaration"
```

---

### Task 6: Make `state` global `static` in `feeding.cpp`

**Files:**
- Modify: `src/feeding.cpp:10` — add `static` to global variable

- [ ] **Step 1: Change `FeedingState state;` to `static FeedingState state;`**

In `src/feeding.cpp`, line 10, replace:
```cpp
FeedingState state;
```
with:
```cpp
static FeedingState state;
```

- [ ] **Step 2: Commit**

```bash
git add src/feeding.cpp
git commit -m "chore: make 'state' global static in feeding.cpp to limit scope"
```

---

### Task 7: Fix `updateDisplay()` row 1 not fully cleared on mode change

**Files:**
- Modify: `src/display.cpp:606-636` — pad row 1 time string to clear ghosts

- [ ] **Step 1: Fix `updateDisplay()` to pad row 1 output**

In `src/display.cpp`, replace lines 629-635 (the row 1 / time display block):

```cpp
    if (time.second != lastSecond) {
        lcd.setCursor(0, 1);
        char timeStr[17];
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d          ", time.hour, time.minute, time.second);
        lcd.print(timeStr);
        lastSecond = time.second;
    }
```

This pads the time string with spaces to 16 characters so any leftover characters from idle animations on row 1 are cleared.

- [ ] **Step 2: Commit**

```bash
git add src/display.cpp
git commit -m "fix: pad time string in updateDisplay to clear row 1 ghosts

Previously, row 1 only wrote the 8-character time string (HH:MM:SS),
leaving leftover characters from idle animations visible. Now pads
to 16 characters to fully clear the row."
```

---

### Task 8: Fix `lastTriggerSecond` edge case in main loop

**Files:**
- Modify: `src/main.cpp:124-141` — use minute-level tracking instead of second-level

- [ ] **Step 1: Replace the feeding trigger block in `main.cpp`**

Replace lines 124-141 with:

```cpp
    static int lastTriggerMinute = -1;
    static int lastTriggerDay = -1;
    if (currentTime.minute != lastTriggerMinute || currentTime.day != lastTriggerDay) {
        for (int s = 0; s < NUM_SESSIONS; s++) {
            if (currentTime.hour == SCHEDULE[s].hour &&
                currentTime.minute == SCHEDULE[s].minute &&
                !hasFedToday(currentTime, s)) {

                Serial.print(F("Auto feeding triggered: "));
                Serial.println(s);
                markActivity();
                triggerAlert(1, 100);
                startFeeding(JUMLAH_PAKAN);
                markFeedingComplete(currentTime, s);
                break;
            }
        }
        lastTriggerMinute = currentTime.minute;
        lastTriggerDay = currentTime.day;
    }
```

- [ ] **Step 2: Commit**

```bash
git add src/main.cpp
git commit -m "fix: use minute+day tracking for feeding trigger instead of second

Previously used lastTriggerSecond which could miss triggers if the
cached time happened to match. Now tracks by minute AND day, which
is the correct granularity for schedule-based feeding and handles
edge cases like system resets gracefully."
```

---

## Summary of Changes

| # | Fix | File(s) | Priority |
|---|-----|---------|----------|
| 1 | Buzzer duration parameter ignored | `alerts.cpp` | 🔴 High |
| 2 | `showRTCError()` misused for low-food | `display.h`, `display.cpp`, `feeding.cpp` | 🔴 High |
| 3 | `checkMissedFeeds()` returns wrong session | `feeding.cpp` | 🔴 High |
| 4 | `iRain()` broken column tracking | `display.cpp` | 🟡 Medium |
| 5 | Dead `playAlert()` forward declaration | `feeding.cpp` | 🟡 Medium |
| 6 | `state` global not `static` | `feeding.cpp` | 🟢 Low |
| 7 | Row 1 not cleared on mode change | `display.cpp` | 🟢 Low |
| 8 | `lastTriggerSecond` edge case | `main.cpp` | 🟡 Medium |
