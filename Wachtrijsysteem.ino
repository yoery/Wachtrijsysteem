#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Stel het I2C-adres en LCD-specificaties in
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD-adres is 0x27, 16x2 scherm

// Definieer de pinnen
const int pirSensorPin = 4;  // PIR-sensor op digitale pin 4
const int greenLedPin = 5;   // Groene LED op pin 5
const int yellowLedPin = 9;  // Gele LED op pin 9
const int redLedPin = 10;    // Rode LED op pin 10
const int trigPin = 6;       // Ultrasone sensor Trig-pin op pin 6
const int echoPin = 7;       // Ultrasone sensor Echo-pin op pin 7

// Variabelen
int peopleCount = 0;          // Huidig aantal personen
int previousCount = -1;       // Vorige waarde van peopleCount
const int threshold = 20;     // Maximale limiet van de wachtrij

unsigned long lastPIRTime = 0; // Tijdstip van laatste PIR-detectie
unsigned long pirDelay = 2000; // Tijd tussen PIR-detecties (in milliseconden)

unsigned long lastUltrasonicTime = 0; // Tijdstip van laatste ultrasone meting
unsigned long ultrasonicInterval = 500; // Interval voor ultrasone metingen (in milliseconden)

void setup() {
  // Initialiseer het LCD-scherm
  lcd.begin();
  lcd.backlight();

  // Stel de pinmodi in
  pinMode(pirSensorPin, INPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(yellowLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Welkomstbericht op LCD
  lcd.setCursor(0, 0);
  lcd.print("Personen: ");
  lcd.setCursor(0, 1);
  lcd.print("Wachtrij start!");

  // Zorg dat alle LEDs uitstaan bij opstarten
  digitalWrite(greenLedPin, LOW);
  digitalWrite(yellowLedPin, LOW);
  digitalWrite(redLedPin, LOW);

  delay(2000); // Kort pauze om te beginnen
  lcd.clear();
}

void loop() {
  unsigned long currentTime = millis();

  // PIR-sensor: Controleer op beweging met een niet-blokkerende timer
  if (currentTime - lastPIRTime >= pirDelay) {
    int motionDetected = digitalRead(pirSensorPin);
    if (motionDetected == HIGH && peopleCount < threshold) { // Alleen verhogen als teller onder de limiet zit
      peopleCount++; // Verhoog de telling met 1
      lastPIRTime = currentTime; // Update de tijdstip van detectie
    }
  }

  // Ultrasone sensor: Meet afstand met een niet-blokkerende timer
  if (currentTime - lastUltrasonicTime >= ultrasonicInterval) {
    long distance = getUltrasonicDistance();
    if (distance > 0 && peopleCount > 0) { // Als er een afstand wordt gemeten en de teller > 0 is
      peopleCount--; // Verminder de telling met 1
    }
    lastUltrasonicTime = currentTime; // Update de tijdstip van de meting
  }

  // Bijwerklogica: alleen uitvoeren als de telling verandert
  if (peopleCount != previousCount) {
    previousCount = peopleCount; // Update de vorige telling

    // Update LCD-scherm
    lcd.setCursor(0, 0);
    lcd.print("Personen: ");
    lcd.print(peopleCount);
    lcd.setCursor(0, 1);
    lcd.print("                "); // Wis de tweede rij

    // LED-logica
    if (peopleCount < 10) {
      // Groene LED aan, andere LEDs uit
      digitalWrite(greenLedPin, HIGH);
      digitalWrite(yellowLedPin, LOW);
      digitalWrite(redLedPin, LOW);
      lcd.setCursor(0, 1);
      lcd.print("Groen licht aan");
    } else if (peopleCount == 10) {
      // Gele LED aan, andere LEDs uit
      digitalWrite(greenLedPin, LOW);
      digitalWrite(yellowLedPin, HIGH);
      digitalWrite(redLedPin, LOW);
      lcd.setCursor(0, 1);
      lcd.print("Wachtrij bijna vol");
    } else if (peopleCount >= threshold) {
      // Rode LED aan, andere LEDs uit
      digitalWrite(greenLedPin, LOW);
      digitalWrite(yellowLedPin, LOW);
      digitalWrite(redLedPin, HIGH);
      lcd.setCursor(0, 1);
      lcd.print("Wachtrij vol!");
    }
  }
}

// Functie om afstand te meten met de ultrasone sensor
long getUltrasonicDistance() {
  // Trig Pin hoog voor 10 microseconden
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Meet de duur van de echo-puls
  long duration = pulseIn(echoPin, HIGH);

  // Bereken de afstand in centimeters
  long distance = duration * 0.034 / 2; // Geluidssnelheid: 0.034 cm/us

  return distance;
}
