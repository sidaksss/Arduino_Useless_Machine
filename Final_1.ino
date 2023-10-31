#include <Servo.h>
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <LiquidCrystal.h>

#define SERVO_PIN 7

// Pin-Definitionen
const int lichtschranke1 = 10;  // Dreieck
const int lichtschranke2 = 11;  // Kreis
const int lichtschranke3 = 12;  // Sechseck
const int lichtschranke4 = A2;  // Viereck
const int lichtschranke5 = A1;  // Abstandsmessung in der Klappe

LiquidCrystal lcd(6, 5, 4, 3, 2, 13);  //Pin, an dem LCD angeschlossen ist

Servo servo;

SoftwareSerial mySoftwareSerial(8, 9);  // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

// Zähvariablen
int a = 0;
int b = 0;
int c = 0;
int d = 0;
int i = 0;

int Fall;
int AnzahlVersuche = 0;

int WinkelZu = 70;
int WinkelAuf = 150;

const unsigned long maxzeit = 20000;  // Zeitmessungen
unsigned long startzeit;
unsigned long wartezeit;
unsigned long richtigzeit;

bool Ergebnis = false;         // gibt Auskunft, ob der Fall richtig erraten wurde
bool FalscheEingaben = false;  // zu viele falsche Eingaben

int Entfernung = 0;  // Abstandsmessung im Fach

void setup() {

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Guten Tag");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Moechten Sie");
  lcd.setCursor(0, 1);
  lcd.print("eine Suessigkeit");

  Serial.println("____________________");
  Serial.begin(9600);

  pinMode(lichtschranke1, INPUT);  // Lichtschranke als Eingang festlegen
  pinMode(lichtschranke2, INPUT);
  pinMode(lichtschranke3, INPUT);
  pinMode(lichtschranke4, INPUT);
  pinMode(lichtschranke5, INPUT);

  mySoftwareSerial.begin(9600);
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  // MP3-Modul-start
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
  }
  Serial.println(F("DFPlayer Mini online."));
  myDFPlayer.volume(22);  //Set volume value. From 0 to 30


  moveServo(WinkelZu);  // Ausgangszustand der Klappe

  randomSeed(analogRead(5));  // generieren eines zufälligen Falls (1-4)
  Fall = random(4) + 1;
  Serial.println((String) "Fall: " + Fall);

  lcd.clear();
  lcd.print("Fall ist ");
  lcd.print(Fall);
  delay(3000);

  startzeit = millis();
  wartezeit = millis();
}

void loop() {

  if (Ergebnis == false) {

    if (digitalRead(lichtschranke1) == 0) {
      i = i + 1;
      a = a + 1;
      Serial.println((String) "Anzahl Gegenstände IR1: " + i);
      delay(100);
      startzeit = millis();
      lcd.setCursor(0, 0);
      lcd.print("Dreieck");
      lcd.setCursor(0, 1);
      lcd.print(i);
      delay(2000);
      lcd.clear();
    }
    if (digitalRead(lichtschranke2) == 0) {
      i = i + 1;
      b = b + 1;
      Serial.println((String) "Anzahl Gegenstände IR2: " + i);
      delay(100);
      startzeit = millis();
      lcd.setCursor(0, 0);
      lcd.print("Kreis");
      lcd.setCursor(0, 1);
      lcd.print(i);
      delay(2000);
      lcd.clear();
    }
    if (digitalRead(lichtschranke3) == 0) {
      i = i + 1;
      c = c + 1;
      Serial.println((String) "Anzahl Gegenstände IR3: " + i);
      delay(100);
      startzeit = millis();
      lcd.setCursor(0, 0);
      lcd.print("Sechseck");
      lcd.setCursor(0, 1);
      lcd.print(i);
      delay(2000);
      lcd.clear();
    }
    if (analogRead(lichtschranke4) >= 30 && analogRead(lichtschranke4) <= 230) {
      i = i + 1;
      d = d + 1;
      Serial.println((String) "Anzahl Gegenstände IR4: " + i);
      delay(100);
      startzeit = millis();
      lcd.setCursor(0, 0);
      lcd.print("Viereck");
      lcd.setCursor(0, 1);
      lcd.print(i);
      delay(2000);
      lcd.clear();
    }

    // wenn nach 40 Sekunden kein stein eingeworfen wird, werden die Zähler zurückgesetzt
    if (millis() - startzeit > 40000 && i <= 3) {
      ResetVariablen();
      Fall = random(4) + 1;
      Serial.println("Variablen zurückgesetzt");
      Serial.println((String) "aktueller Fall: " + Fall);
      delay(50);
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Guten Tag");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Moechten Sie");
      lcd.setCursor(0, 1);
      lcd.print("eine Suessigkeit");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("aktueller Fall");
      lcd.setCursor(0, 1);
      lcd.print("ist ");
      lcd.print(Fall);
      delay(3000);
      lcd.clear();

      Ergebnis = false;
    }

    if (i >= 4) {  // Überprüfen, ob 4 Gegenstände erkannt wurden
      delay(1000);
      Serial.println("4 Gegenstände erkannt");

      // Fall 1: wenn in 3 und 4 etwas reinfällt geht die tür auf
      if (Fall == 1) {
        if (c >= 1 && d >= 1) {
          RichtigeEingabe();
        } else {
          FalscheEingabe();
        }
      }

      // Fall 2: wenn nur in 4 etwas reinfällt
      if (Fall == 2) {
        if (a == 0 && b == 0 && c == 0 && d >= 1) {
          RichtigeEingabe();
        } else {
          FalscheEingabe();
        }
      }

      // Fall 3: wenn in 1,2 und 3 etwas reinfällt
      if (Fall == 3) {
        if (a >= 1 && b >= 1 && c >= 1 && d == 0) {
          RichtigeEingabe();
        } else {
          FalscheEingabe();
        }
      }

      // Fall 4: wenn in 2 und 4 etwas reinfällt
      if (Fall == 4) {
        if (a == 0 && b >= 1 && c == 0 && d >= 1) {
          RichtigeEingabe();
        } else {
          FalscheEingabe();
        }
      }

      // Zurücksetzen der Variabler bzw. Zähler
      ResetVariablen();

      Serial.println((String) "Anzahl Gegenstände: " + i);
    }

  } else {  // Ergebnis wurde richig erraten
    Serial.println("Antwort richtig");
    delay(1000);
    richtigzeit = millis();
    if (servo.read() != WinkelAuf) {
      moveServo(WinkelAuf);
      Serial.println("Klappe auf");
      delay(50);
    }

    while (richtigzeit - millis() < 20000) {
      Serial.println("in der while");
      Entfernung = analogRead(lichtschranke5);
      Serial.println((String) "Entfernung: " + Entfernung);
      if (Entfernung <= 200) {  // Erkennung, ob etwas in das Fach greift
        Serial.println("Sound jz bitte abspielen");
        myDFPlayer.play(1);
        delay(10000);
        if (servo.read() != WinkelZu) {
          moveServo(WinkelZu);
        }
        delay(2000);
        Ergebnis = false;
      }
    }

    // ist glaube egal, aber erstmal noch im code
    if (millis() - wartezeit > 40000) {  // wenn nach x Sekunden nichts weiter passiert, geht es wieder in den Ausgangszustand
      Fall = random(4) + 1;
      Serial.println((String) "Neuer Fal: " + Fall);
      Ergebnis = false;
    }

    if (FalscheEingaben == true) {  // Anzahl max Versuche überschritten
      Serial.println("Zu viele falsche Eingaben");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Und du bist");
      lcd.setCursor(0, 1);
      lcd.print("raus (5)");
      delay(3000);
      lcd.clear();
      delay(2000);

      AnzahlVersuche = 0;
      ResetVariablen();
      FalscheEingaben = false;
      Ergebnis = false;
    }
  }
}



void moveServo(int angle) {
  servo.attach(SERVO_PIN);
  servo.write(angle);
  delay(1000);
  servo.detach();
}

void RichtigeEingabe() {
  delay(1000);
  lcd.begin(16, 2);
  lcd.print("Richtig!");
  delay(1000);
  lcd.clear();
  wartezeit = millis();

  Ergebnis = true;
}

void FalscheEingabe() {
  AnzahlVersuche++;
  Serial.println((String) "Anzahl Versuche: " + AnzahlVersuche);
  delay(1000);
  lcd.begin(16, 2);
  lcd.print("Falsch!");
  delay(1000);
  lcd.clear();
  Serial.println("Klappe bleibt zu");
  if (servo.read() != WinkelZu) {
    moveServo(WinkelZu);
  }

  if (AnzahlVersuche == 1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Versuche etwas");
    lcd.setCursor(0, 1);
    lcd.print("anderes (1)");
    delay(3000);
    lcd.clear();
  }

  if (AnzahlVersuche == 2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Das war es");
    lcd.setCursor(0, 1);
    lcd.print("leider nicht (2)");
    delay(3000);
    lcd.clear();
  }

  if (AnzahlVersuche == 3) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Denk noch mal");
    lcd.setCursor(0, 1);
    lcd.print("scharf nach (3)");
    delay(3000);
    lcd.clear();
  }

  if (AnzahlVersuche == 4) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Versuche etwas");
    lcd.setCursor(0, 1);
    lcd.print("anderes");
    delay(3000);
    lcd.clear();
  }

  if (AnzahlVersuche > 4) {
    FalscheEingaben = true;
  }

  Ergebnis = false;
}

void ResetVariablen() {
  i = 0;
  a = 0;
  b = 0;
  c = 0;
  d = 0;
}
