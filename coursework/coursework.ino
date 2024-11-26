#include <Arduino.h>

const int buttonStartPin = 23;  // Кнопка 1
const int buttonConfirmPin = 22;  // Кнопка 2
const int ledPins[] = {5, 4, 19};

// Змінні
bool programRunning = false;    // Стан програми (вимкнена за замовчуванням)
int currentMode = 0;            // Поточний режим (0 = режим 1)
bool modeConfirmed = false;     // Чи підтверджений вибір режиму

unsigned long lastButtonPressTime = 0;  // Час останнього натискання кнопки
const unsigned long debounceDelay = 200; // Затримка між натисканнями (мс)
unsigned long lastBlinkTime = 0;         // Час останнього мигання
const unsigned long blinkInterval = 500; // Інтервал мигання (мс)

// Для режиму 2
unsigned long reactionTimes[100]; // Масив для збереження часу реакції (до 100 спроб)
int attemptCount = 0;             // Лічильник спроб у режимі 2

void setup();
void loop();
void startProgram();
void stopProgram();
void switchMode();
void executeMode1();
void executeMode2();
void executeMode3();
int chooseParameter(String paramName);

void setup() {
  Serial.begin(115200);

  // Налаштування пінів
  pinMode(buttonStartPin, INPUT);
  pinMode(buttonConfirmPin, INPUT);

  for (int i = 0; i < 3; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);  // Вимкнути всі світлодіоди
  }

  Serial.println("Програма готова до запуску.");
}

void loop() {
  static bool prevStartButtonState = LOW;
  static bool prevConfirmButtonState = LOW;

  bool currentStartButtonState = digitalRead(buttonStartPin);
  bool currentConfirmButtonState = digitalRead(buttonConfirmPin);

  // Кнопка 1: Увімкнення/вимкнення програми або перемикання режимів
  if (prevStartButtonState == LOW && currentStartButtonState == HIGH && millis() - lastButtonPressTime > debounceDelay) {
    lastButtonPressTime = millis();
    if (!programRunning) {
      startProgram();
    } else if (programRunning && !modeConfirmed) {
      switchMode();
    }
  }

  // Затискання кнопки 1: Вимкнення програми
  if (currentStartButtonState == HIGH) {
    unsigned long holdTime = millis();
    while (digitalRead(buttonStartPin) == HIGH) {
      if (millis() - holdTime > 1000) {  // Затримка 1 секунда
        stopProgram();
        delay(1000); // Затримка після вимкнення (1 секунда)
        return;
      }
    }
  }

  // Кнопка 2: Підтвердження вибору режиму
  if (prevConfirmButtonState == LOW && currentConfirmButtonState == HIGH && millis() - lastButtonPressTime > debounceDelay) {
    lastButtonPressTime = millis();
    if (programRunning && !modeConfirmed) {
      modeConfirmed = true;
      Serial.print("Режим ");
      Serial.print(currentMode + 1);
      Serial.println(" підтверджено.");
      digitalWrite(ledPins[currentMode], HIGH); // Увімкнути вибраний режим
      if (currentMode == 0) {
        executeMode1();
      } else if (currentMode == 1) {
        executeMode2();
      } else if (currentMode == 2) {
        executeMode3();
      }
    }
  }

  // Мигання активного світлодіода під час вибору режиму
  if (programRunning && !modeConfirmed && millis() - lastBlinkTime > blinkInterval) {
    lastBlinkTime = millis();
    static bool ledState = LOW;
    ledState = !ledState;
    digitalWrite(ledPins[currentMode], ledState);

    // Ліміт часу для вибору режиму
    if (millis() - lastButtonPressTime > 10000) {  // Ліміт 10 секунд
      Serial.println("Час вибору режиму вичерпано.");
      stopProgram();
      return;
    }
  }

  prevStartButtonState = currentStartButtonState;
  prevConfirmButtonState = currentConfirmButtonState;
}

// Функція запуску програми
void startProgram() {
  programRunning = true;
  modeConfirmed = false;
  currentMode = 0; // Початковий режим - 1
  Serial.println("Програма увімкнена. Виберіть режим.");

  // Увімкнути всі світлодіоди на 1 секунду
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPins[i], HIGH);
  }
  delay(1000);

  // Вимкнути всі, окрім першого
  for (int i = 1; i < 3; i++) {
    digitalWrite(ledPins[i], LOW);
  }
}

// Функція вимкнення програми
void stopProgram() {
  programRunning = false;
  modeConfirmed = false;
  Serial.println("Програма вимкнена.");

  // Очистити результати для режиму 2
  attemptCount = 0;
  for (int i = 0; i < 100; i++) {
    reactionTimes[i] = 0;
  }

  // Вивести результати для режиму 2
  if (attemptCount > 0) {
    Serial.println("Результати часу реакції:");
    unsigned long totalTime = 0;
    for (int i = 0; i < attemptCount; i++) {
      Serial.print("Спроба ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(reactionTimes[i]);
      Serial.println(" мс");
      totalTime += reactionTimes[i];
    }
    Serial.print("Середній час реакції: ");
    Serial.print(totalTime / attemptCount);
    Serial.println(" мс");
  }

  // Увімкнути всі світлодіоди на 1 секунду
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPins[i], HIGH);
  }
  delay(1000);

  // Вимкнути всі світлодіоди
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPins[i], LOW);
  }
}

// Функція перемикання режимів
void switchMode() {
  // Вимкнути поточний світлодіод
  if (currentMode >= 0 && currentMode < 3) {
    digitalWrite(ledPins[currentMode], LOW);
  }

  // Переключити на наступний режим
  currentMode = (currentMode + 1) % 3;

  // Почати мигання нового світлодіода
  Serial.print("Режим переключено на: ");
  Serial.println(currentMode + 1);
}

// Реалізація режиму 1
void executeMode1() {
  digitalWrite(ledPins[0], HIGH);
  delay(1000);
  digitalWrite(ledPins[0], LOW);

  Serial.println("Приготуйтесь");
  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPins[0], HIGH);
    digitalWrite(ledPins[2], HIGH);
    delay(250);
    digitalWrite(ledPins[0], LOW);
    digitalWrite(ledPins[2], LOW);
    delay(250);
  }

  unsigned long randomDelay = random(3000, 6000);
  delay(randomDelay);

  Serial.println("Натисніть кнопку!");
  digitalWrite(ledPins[1], HIGH);
  
  unsigned long startTime = millis();
  bool timeout = false;
  
  while (digitalRead(buttonConfirmPin) == LOW) {
    if (millis() - startTime > 5000) {
      timeout = true;
      break;
    }
  }

  digitalWrite(ledPins[1], LOW);
  
  if (timeout) {
    Serial.println("Час вичерпано. Програма завершується.");
    stopProgram();
    return;
  }

  unsigned long reactionTime = millis() - startTime;
  Serial.print("Ваш час реакції: ");
  Serial.print(reactionTime);
  Serial.println(" мс");

  stopProgram();
}

// Реалізація режиму 2
void executeMode2() {
  attemptCount = 0;
  digitalWrite(ledPins[1], LOW);

  while (true) {
    Serial.println("Приготуйтесь");
    for (int i = 0; i < 4; i++) {
      digitalWrite(ledPins[0], HIGH);
      digitalWrite(ledPins[2], HIGH);
      delay(250);
      digitalWrite(ledPins[0], LOW);
      digitalWrite(ledPins[2], LOW);
      delay(250);
    }

    unsigned long randomDelay = random(3000, 6000);
    delay(randomDelay);

    Serial.println("Натисніть кнопку!");
    digitalWrite(ledPins[1], HIGH);

    unsigned long startTime = millis();
    bool timeout = false;
    bool stopByButton = false;

    while (digitalRead(buttonConfirmPin) == LOW) {
      if (millis() - startTime > 5000) {
        timeout = true;
        break;
      }

      // Перевірка натискання кнопки 1 для завершення програми
      if (digitalRead(buttonStartPin) == HIGH) {
        unsigned long holdTime = millis();
        while (digitalRead(buttonStartPin) == HIGH) {
          if (millis() - holdTime > 1000) {  // Затримка 1 секунда
            stopByButton = true;
            break;
          }
        }
      }

      if (stopByButton) {
        break;
      }
    }

    digitalWrite(ledPins[1], LOW);

    unsigned long reactionTime = millis() - startTime;

    if (timeout) {
      Serial.println("Час вичерпано. Програма завершується.");
      break;
    }

    if (stopByButton) {
      Serial.println("Програму завершено користувачем. Результати:");
      reactionTimes[attemptCount++] = reactionTime;
      break;
    }

    reactionTimes[attemptCount++] = reactionTime;

    Serial.print("Спроба ");
    Serial.print(attemptCount);
    Serial.print(": ");
    Serial.print(reactionTime);
    Serial.println(" мс");

    delay(1000);
  }

  // Виведення результатів
  Serial.println("Результати часу реакції:");
  unsigned long totalTime = 0;
  for (int i = 0; i < attemptCount; i++) {
    Serial.print("Спроба ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(reactionTimes[i]);
    Serial.println(" мс");
    totalTime += reactionTimes[i];
  }
  Serial.print("Середній час реакції: ");
  Serial.print(totalTime / attemptCount);
  Serial.println(" мс");

  stopProgram();
}

// Додано перевірку ліміту часу і передачу ходу в режимі 3
void executeMode3() {
  int players = 1, rounds = 1;
  int scores[3] = {0, 0, 0};
  unsigned long reactionTimes[3][3] = {0};

  Serial.println("Виберіть кількість гравців (1-3):");
  players = chooseParameter("гравців");
  if (players == -1) return;  // Якщо вибір не зроблено

  Serial.println("Виберіть кількість раундів (1-3):");
  rounds = chooseParameter("раундів");
  if (rounds == -1) return;  // Якщо вибір не зроблено

  for (int i = 0; i < 3; i++) digitalWrite(ledPins[i], HIGH);
  delay(1000);
  for (int i = 0; i < 3; i++) digitalWrite(ledPins[i], LOW);
  delay(500);

  for (int round = 0; round < rounds; round++) {
    Serial.print("Раунд ");
    Serial.println(round + 1);

    for (int player = 0; player < players; player++) {
      Serial.print("Гравець ");
      Serial.println(player + 1);

      for (int i = 0; i < 4; i++) {
        digitalWrite(ledPins[(player + 1) % 3], HIGH);
        digitalWrite(ledPins[(player + 2) % 3], HIGH);
        delay(250);
        digitalWrite(ledPins[(player + 1) % 3], LOW);
        digitalWrite(ledPins[(player + 2) % 3], LOW);
        delay(250);
      }

      unsigned long randomDelay = random(2000, 5000);
      delay(randomDelay);

      digitalWrite(ledPins[player], HIGH);
      Serial.println("Натисніть кнопку!");

      unsigned long startTime = millis();
      bool timeout = false;

      while (digitalRead(buttonConfirmPin) == LOW) {
        if (millis() - startTime > 5000) {
          timeout = true;
          break;
        }

        // Перевірка затискання кнопки 1 для завершення програми в будь-який момент
        if (digitalRead(buttonStartPin) == HIGH) {
          unsigned long holdTime = millis();
          while (digitalRead(buttonStartPin) == HIGH) {
            if (millis() - holdTime > 1000) {
              stopProgram();
              delay(1000);
              return;
            }
          }
        }
      }

      digitalWrite(ledPins[player], LOW);

      if (timeout) {
        Serial.println("Час вичерпано. Передача ходу іншому гравцю.");
        reactionTimes[player][round] = 5000;
        scores[player] += 5000;
      } else {
        unsigned long reactionTime = millis() - startTime;
        reactionTimes[player][round] = reactionTime;
        scores[player] += reactionTime;

        Serial.print("Час реакції гравця ");
        Serial.print(player + 1);
        Serial.print(": ");
        Serial.print(reactionTime);
        Serial.println(" мс");
      }

      for (int i = 0; i < 3; i++) digitalWrite(ledPins[i], HIGH);
      delay(1000);
      for (int i = 0; i < 3; i++) digitalWrite(ledPins[i], LOW);
      delay(500);
    }
  }

  if (players == 1) {
    Serial.println("\nРезультати гравця:");
    for (int round = 0; round < rounds; round++) {
      Serial.print("  Раунд ");
      Serial.print(round + 1);
      Serial.print(": ");
      Serial.print(reactionTimes[0][round]);
      Serial.println(" мс");
    }
    Serial.print("Середній час: ");
    Serial.print(scores[0] / rounds);
    Serial.println(" мс");
    stopProgram();
    return;
  }

  int rankings[3] = {0, 1, 2};
  for (int i = 0; i < players - 1; i++) {
    for (int j = i + 1; j < players; j++) {
      if (scores[rankings[i]] > scores[rankings[j]]) {
        int temp = rankings[i];
        rankings[i] = rankings[j];
        rankings[j] = temp;
      }
    }
  }

  for (int cycle = 0; cycle < 3; cycle++) {
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPins[i], HIGH);
      delay(150);
      digitalWrite(ledPins[i], LOW);
    }
  }
  delay(1000);

  for (int i = 0; i < players; i++) {
    if (players == 2 && i == 1) break;
    delay(2000);
    Serial.print("Місце ");
    Serial.print(i + 1);
    Serial.print(": Гравець ");
    Serial.println(rankings[i] + 1);

    for (int j = 0; j < 4; j++) {
      digitalWrite(ledPins[rankings[i]], HIGH);
      delay(250);
      digitalWrite(ledPins[rankings[i]], LOW);
      delay(250);
    }
  }

  Serial.println("\nРезультати гравців:");
  for (int player = 0; player < players; player++) {
    Serial.print("Гравець ");
    Serial.print(player + 1);
    Serial.print(" (Середній час: ");
    Serial.print(scores[player] / rounds);
    Serial.println(" мс):");

    for (int round = 0; round < rounds; round++) {
      Serial.print("  Раунд ");
      Serial.print(round + 1);
      Serial.print(": ");
      Serial.print(reactionTimes[player][round]);
      Serial.println(" мс");
    }
    Serial.println();
  }

  stopProgram();
}

int chooseParameter(String paramName) {
  int param = 1;
  unsigned long startTime = millis();

  while (true) {
    for (int i = 0; i < param; i++) {
      digitalWrite(ledPins[i], HIGH);
    }
    delay(300);
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPins[i], LOW);
    }
    delay(300);

    if (digitalRead(buttonStartPin) == HIGH) {
      param = (param % 3) + 1;
      delay(300);
    }

    if (digitalRead(buttonConfirmPin) == HIGH) {
      Serial.print("Вибрано ");
      Serial.print(param);
      Serial.print(" ");
      Serial.println(paramName);
      delay(500);
      break;
    }

    if (millis() - startTime > 10000) {  
      Serial.println("Час вибору параметра вичерпано.");
      stopProgram();
      return -1; 
    }

    if (digitalRead(buttonStartPin) == HIGH) {
      unsigned long holdTime = millis();
      while (digitalRead(buttonStartPin) == HIGH) {
        if (millis() - holdTime > 1000) {
          stopProgram();
          delay(1000);
          return -1; 
        }
      }
    }
  }
  return param;
}