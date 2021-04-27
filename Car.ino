#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <dht.h>

#define S1 PB4
#define DHT11 PB5


TinyGPS gps;
SoftwareSerial ss(3, 2);
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);
dht DHT;

int lastDHT = -1;
bool newData = false;
unsigned long chars;
unsigned short sentences, failed;
float flat, flon;
unsigned long age;
float speed = 0;
float altitude = 0;
int state = 1;
int preState = 0;

void setup() {
  DDRB &= ~(1 << S1); // S1 input
  PORTB |= 1 << S1;  // Pullup
 //pinMode(S1, INPUT_PULLUP);
  Serial.begin(115200);
  ss.begin(9600);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Starting system");
}

void loop() {
  if (state == 3) {
    if (preState != state) {
      preState = state;
      ss.end();
    }
    displayDHT();
    delaySwitch();
  } else {
    if (preState == 3) {
      preState = state;
      ss.begin(9600);
    }
    while (ss.available()) { // check for gps data
      char c = ss.read();
      Serial.print(c);
      bool DataOK = gps.encode(c);
      if (DataOK) // encode gps data
      {
        switch (state) {
        case 0:
          gps.f_get_position(&flat, &flon, &age);
          displayPosition();
          break;
        case 1:
          print_date(gps);
          break;
        case 2:
          speed = gps.f_speed_kmph();
          altitude = gps.f_altitude();
          displaySpeed();
          break;
        } // case
      }   // if
      if ((PINB & (1 << S1)) == 0) {
        state++;
        if (state > 3) {
          state = 0;
        }
        delay(100);
      }
    } // while
  }
}

void displaySpeed() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Speed:  ");
  lcd.print(speed == TinyGPS::GPS_INVALID_F_SPEED ? 0.0 : speed, 2);
  lcd.setCursor(0, 1);
  lcd.print("Alt:    ");
  lcd.print(speed == TinyGPS::GPS_INVALID_F_ALTITUDE ? 0.0 : altitude, 2);
}

void delaySwitch() {
  for (int i = 0; i < 200; i++) {
    smartdelay(10);
    if (digitalRead(S1) == 0) {
      state++;
      if (state > 3) {
        state = 0;
      }
      Serial.println(state);
      delay((200 - i) * 10);
      break;
    }
  }
}
void print_date(TinyGPS &gps) {
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  lcd.clear();
  lcd.setCursor(0, 0);
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths,
                     &age);
  if (age == TinyGPS::GPS_INVALID_AGE)
    Serial.print("********** ******** ");
  else {
    char sz[16];
    sprintf(sz, "   %02d/%02d/%02d", month, day, year);
    lcd.print(sz);
    lcd.setCursor(0, 1);
    char sz_time[16];
    sprintf(sz_time, "      %02d:%02d", hour + 2, minute);
    lcd.print(sz_time);
    // Serial.println(sz);
  }
  smartdelay(0);
}

void smartdelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

void displayDHT() {
  int chk = DHT.read11(DHT11);
  Serial.println(chk);
  if (chk == 0) {
    if (lastDHT == 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Temp: ");
      float temperature = DHT.temperature;
      if (temperature < 0) {
        lcd.setCursor(8, 0);
      } else {
        lcd.setCursor(9, 0);
      }
      lcd.print(temperature);
      lcd.print((char)223);
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("Humidity: ");
      lcd.print(DHT.humidity);
      lcd.print("%");
    }
  }
  lastDHT = chk;
}

void displayPosition() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LAT:");
  lcd.setCursor(5, 0);
  lcd.print(String(flat, 6));
  lcd.setCursor(0, 1);
  lcd.print("LON:");
  lcd.setCursor(5, 1);
  lcd.print(String(flon, 6));
}
