#include <Keypad.h>
#include <LiquidCrystal.h>

#define TIME long

const int rs = 32, en = 30, d4 = 28, d5 = 26, d6 = 24, d7 = 22;
LiquidCrystal ldc(rs, en, d4, d5, d6, d7);

const byte ROWS = 4;  //four rows
const byte COLS = 4;  //four columns
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte rowPins[ROWS] = { 34, 36, 38, 40 }; /* connect to the row pinouts of the keypad */
byte colPins[COLS] = { 42, 44, 46, 48 }; /* connect to the column pinouts of the keypad */

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int ledPin[9] = { 31, 33, 35, 37, 39, 41, 43, 45, 47 };  // the number of the LED pins
int curentLed = 0;

int timers[3] = { 0, 0, 0 };
const int MAX_TIMER_TIME = 99 * 60 + 45;

bool home_print = true;
const char comp_date[] = __DATE__;
const char comp_time[] = __TIME__;

const int PIN_TO_SENSOR = 49;  // the pin that OUTPUT pin of sensor is connected to
int pinStateCurrent = LOW;    // current state of pin
int pinStatePrevious = LOW;

enum class state {
  HOME,
  MOD_ADD_DEL,
  ChooseExercise
};

state displayState = state::HOME;

class Clock {
private:
  TIME startTime = 0;
  int lastObservedSecond = 0;

public:
  Clock(TIME initialSeconds) {
    startTime = initialSeconds;
  }

  TIME getTime() {
    return startTime + (millis() / 1000);
  }

  bool secondHasChanged() {
    if (lastObservedSecond < millis() / 1000) {
      lastObservedSecond = millis() / 1000;
      return true;
    }

    return false;
  }
} clk(0);

char waitForKeypress() {
  for (;;) {
    char key = keypad.getKey();
    if (key) {
      return key;
    }
  }
}

void displayNumber(int cnt) {
  ldc.clear();
  ldc.setCursor(0, 0);
  ldc.print(cnt);
  ldc.print(" +/-");
}

int change_numbers(int cnt) {
  for (;;) {
    displayNumber(cnt);
    char key = waitForKeypress();

    switch (key) {
      case '2':
        cnt++;
        break;
      case '8':
        cnt--;
        break;
      default:
        ldc.clear();
        return cnt;
        break;
    }
  }
}

class workout {
private:
  int time = 0;
  int flotari = 0, geno = 0, abd = 0;
public:
  int index = 0;

  void add(int p, int g, int a, int t) {
    time = t;
    flotari = p;
    geno = g;
    abd = a;
    index = 1;
  }

  int pushups() {
    return flotari;
  }

  int Geno() {
    return geno;
  }

  int Abd() {
    return abd;
  }

  int Time() {
    return time;
  }
};

class vector {
private:
  int size = 0;

public:
  workout vec[10];

  void push(int p, int g, int a, int t) {
    vec[size].add(p, g, a, t);
    size++;
  }

  void pop() {
    vec[size].index = 0;
    if (size > 0) {
      size--;
    }
  }
};

vector w;

TIME time_initial() {
  int h1, h2, hh, m1, m2, mm, s1, s2, ss;

  h1 = (comp_time[0] - '0');
  h2 = comp_time[1] - '0';

  m1 = (comp_time[3] - '0');
  m2 = comp_time[4] - '0';

  s1 = (comp_time[6] - '0');
  s2 = comp_time[7] - '0';

  hh = h1 * 10 + h2;
  mm = m1 * 10 + m2;
  ss = s1 * 10 + s2;

  return 3600LL * hh + 60 * mm + ss;
}

void home() {
  TIME time = clk.getTime();
  int hh = (time / 3600) % 24;
  int mm = (time / 60) % 60;
  int ss = time % 60;

  if (clk.secondHasChanged()) {
    Serial.println(time);
    Serial.println(time_initial());
    ldc.clear();
    ldc.print(comp_date);
    ldc.setCursor(0, 1);

    if (hh < 10) {
      ldc.print(0);
    }
    ldc.print(hh);
    ldc.print(':');
    if (mm < 10) {
      ldc.print(0);
    }
    ldc.print(mm);
    ldc.print(':');
    if (ss < 10) {
      ldc.print(0);
    }
    ldc.print(ss);
  }
}

void setup() {

  Serial.begin(9600);
  ldc.begin(16, 2);

  // set the digital pin as output:

  for (int i = 0; i < 9; i++) {

    pinMode(ledPin[i], OUTPUT);

    digitalWrite(ledPin[i], LOW);
  }

  // digitalWrite(ledPin[curentLed],HIGH);
  clk = Clock(time_initial());
  pinMode(PIN_TO_SENSOR, INPUT);
  delay(100);
}

void loop() {
  char key = keypad.getKey();

  //Serial.print("LOOP KEY:");
  //Serial.println(key);

  if (key) {
    switch (key) {
      case 'D':
        displayState = state::MOD_ADD_DEL;
        break;
      case '#':
        displayState = state::HOME;
        break;
      case 'A':
        displayState = state::ChooseExercise;
      default:
        break;
    }
  }

  switch (displayState) {
    case state::HOME:
      home();
      break;
    case state::MOD_ADD_DEL:
      modAddDel();
      Serial.println("intra aici ciclu inf");
      break;
    case state::ChooseExercise:
      chooseExercise();
    default:
      break;
  }

  // if all the leds complete do the animation and display, you did it!!
}

void modAddDel() {
  ldc.clear();
  ldc.print("Modify/Add/Del");
  configure_schedule();
}

int configure_workout(int i) {
  ldc.clear();
  ldc.print("pushups?");
  delay(2000);
  int pushups = change_numbers(w.vec[i].pushups());
  ldc.print("abds?");
  delay(2000);
  int abd = change_numbers(w.vec[i].Abd());
  ldc.print("geno?");
  delay(2000);
  int geno = change_numbers(w.vec[i].Geno());
  ldc.print("Duration?");
  delay(2000);
  int t = setTimer(w.vec[i].Time());

  w.vec[i].add(pushups, abs, geno, t);
}

void Modify() {
  ldc.clear();
  ldc.print("Modify program?");
  ldc.setCursor(0, 1);

  char key = waitForKeypress();
  int prog = key - '0';
  if (w.vec[prog].index == 0) {
    ldc.clear();
    ldc.print("Incorect");
    delay(2000);
    displayState = state::HOME;
    return;
  }

  configure_workout(prog);
}

void Add() {
  ldc.clear();
  ldc.print("pushups?");
  delay(2000);
  int pushups = change_numbers(0);
  ldc.print("abds?");
  delay(2000);
  int abd = change_numbers(0);
  ldc.print("geno?");
  delay(2000);
  int geno = change_numbers(0);
  ldc.print("Duration?");
  delay(2000);
  int t = setTimer(0);

  w.push(pushups, geno, abd, t);
  Serial.println(w.vec[0].pushups());
}

void Delete() {
  ldc.clear();
  ldc.print("Delete program?");
  ldc.setCursor(0, 1);

  w.pop();
}

void configure_schedule() {
  char key = waitForKeypress();
  if (key == '#') {
    displayState = state::HOME;
    return;
  }

  switch (key) {
    case '1':
      Modify();
      break;
    case '2':
      Add();
      break;
    case '3':
      Delete();
      break;
  }
}

void ledForward() {

  //digitalWrite(ledPin[curentLed],LOW);

  if (curentLed == 8) {
    return;
  }  // can't increase the led pos

  curentLed++;
  digitalWrite(ledPin[curentLed], HIGH);

  Serial.println(curentLed);  // debuging
}

void ledBackward() {
  digitalWrite(ledPin[curentLed], LOW);

  if (curentLed == 0) {
    return;
  }

  curentLed = (curentLed - 1);

  //digitalWrite(ledPin[curentLed], HIGH);
}

void displayTime(int time) {
  // ldc.setCursor(0, 0);
  ldc.clear();
  ldc.setCursor(0, 0);
  ldc.print(time / 60);
  ldc.print(":");
  ldc.print(time % 60);

  ldc.print(" +/-");
}

int setTimer(int timerNumber) {
  int crt = timerNumber;
  delay(200);

  displayTime(crt);
  for (;;) {
    char key = keypad.getKey();
    if (key == '2') {
      if (crt < MAX_TIMER_TIME) {
        crt += 15;
      }
      displayTime(crt);
    } else if (key == '8') {
      if (crt > 0) {
        crt -= 15;
      }
      displayTime(crt);
    } else if (key) {
      ldc.clear();
      ldc.setCursor(0, 0);
      return crt;
    }
  }
}

void chooseExercise() {
  ldc.clear();
  ldc.print("Workout now 1");
  ldc.setCursor(0, 1);
  ldc.print("Select Workout 2");
  delay(2000);

  char key = waitForKeypress();

  if (key == '#') {
    displayState = state::HOME;
    return;
  }

  if (key == '1') {
    ldc.clear();
    ldc.print("Timed workout");
    delay(2000);
    ldc.clear();
    ldc.print("Select Time");
    delay(2000);

    int time = setTimer(0);
    int a[3] = { 0, 0, 0 };
    startTime(time, a, -1);
  } else if (key == '2') {
    hitWorkout();
  }
}

void hitWorkout() {
  ldc.clear();
  ldc.print("Select workout");
  delay(2000);
  char key = waitForKeypress();

  if (key == '#') {
    displayState = state::HOME;
    return;
  } else {
    int prog = key - '0';

    if (w.vec[prog].index == 0) {
      ldc.clear();
      ldc.print("Doesn't exist");
      delay(2000);
      displayState = state::HOME;
      return;
    } else {
      hitExercise(prog);
    }
  }
}

void hitExercise(int i) {
  ldc.clear();
  ldc.print("Ready?");
  ldc.setCursor(0, 1);
  ldc.print("Press start!");
  delay(2000);

  char key = waitForKeypress();

  if (key == '#') {
    displayState = state::HOME;
    return;
  }

  int time = w.vec[i].Time();
  int ex[3] = { w.vec[i].pushups(), w.vec[i].Geno(), w.vec[i].Abd() };

  startTime(time, ex, 3);
}


// void stopTime() {
//   for (;;) {
//     char key = keypad.getKey();
//     if (key == '#') {
//       displayState = state::HOME;
//       return;
//     }

//     if (key == '*') {
//       return;
//     }
//   }
// }

void startTime(int time, int ex[], int s) {
  char type_ex[3][20] = { "pushups ",
                          "abdomene ",
                          "genoflexiuni " };

  displayTime(time);
  ldc.setCursor(0, 1);
  int pos = 0;

  if (s != -1) {
    ldc.print(type_ex[pos]);
    ldc.print(ex[pos]);
  }

  delay(1000);

  for (;;) {
    char key = keypad.getKey();
    Serial.println(key);

    if (key == '#') {
      displayState = state::HOME;
      return;
    } else if (key == '0') {
      pos++;
    }

    if (pos == s) {
      ldc.clear();
      ldc.print("GOOD JOB!");
      delay(2000);
      return;
    }
    
    bool move = Detect_Move();

    if (move) {
      time--;
    }

    displayTime(time);
    ldc.setCursor(0, 1);

    if (s != -1) {
      ldc.print(type_ex[pos]);
      ldc.print(ex[pos]);
    }

    delay(1000);

    if (time == 0) {
      ldc.clear();
      ldc.print("GOOD JOB!");
      delay(2000);
      return;
    }
  }
}

bool Detect_Move() {
  pinStatePrevious = pinStateCurrent;
  pinStateCurrent = digitalRead(PIN_TO_SENSOR);

  if (pinStatePrevious == LOW && pinStateCurrent == HIGH) {
    Serial.println("Motion detected!");
    return true;
  } else if (pinStatePrevious == HIGH && pinStateCurrent == LOW) {
    Serial.println("Motion stopped!");
    return false;
  }
}

// void beginSetTimer() {
//   ldc.print("Which timer?");
//   ldc.setCursor(0, 1);
//   ldc.print("1 2 3");

//   for (;;) {

//     char key = keypad.getKey();
//     Serial.println(key);
//     if (key) {
//       setTimer(key - '1'); // THIS CAUSES AN ERROR FOR THE MOMENT
//       return;
//     }
//   }
// }