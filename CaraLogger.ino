#include <Adafruit_GPS.h>
#include <SPI.h>
#include <SD.h>

#define GPSSerial Serial3
#define CaraSerial Serial2    // Change this to Serial for testing and Serial2 for actual CaRA

#define ON   LOW
#define OFF  HIGH
#define power_out 10

bool GPSECHO = false;
Adafruit_GPS GPS(&GPSSerial);
uint32_t timer = millis();
char gpsdata[100];
char caradata[100];
const int chipSelect = 53;
File SDLogFile;
bool SDFileOpen = false;
int  bootlevel = 0;

int LED_RED = 22;
int LED_BLUE = 28;
int LED_GRN = 26;
int PBUT1 = 24;
int PBUT2 = 30;
int PBUT3 = 34;
int BUZ  = 32;
int PLUS5V   = 36;
int GND = 38;


void setup() {
   pinMode(LED_RED, OUTPUT);
   pinMode(LED_GRN, OUTPUT);
   pinMode(LED_BLUE, OUTPUT);
   pinMode(PBUT1, INPUT_PULLUP);
   pinMode(PBUT2, INPUT_PULLUP);
   pinMode(PBUT3, INPUT_PULLUP);
   pinMode(BUZ, OUTPUT);
   pinMode(PLUS5V, OUTPUT);
   pinMode(GND, OUTPUT);
   digitalWrite(LED_RED,OFF);
   digitalWrite(LED_GRN,OFF);
   digitalWrite(LED_BLUE,OFF);
   digitalWrite(BUZ,LOW);
   digitalWrite(PLUS5V,HIGH);
   digitalWrite(GND,LOW);

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  CaraSerial.begin(9600);
  Serial3.begin(9600);
  GPS.begin(9600);

 Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    led_red();
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  all_leds_off();
  Serial.println("card initialized.");
  delay(3000);   // Give CaRA time to boot up
}

void all_leds_off()
{
   digitalWrite(LED_RED, OFF);
   digitalWrite(LED_BLUE, OFF);
   digitalWrite(LED_GRN, OFF);
}

void led_red()
{
   all_leds_off();
   digitalWrite(LED_RED,ON);
}

void led_blue()
{
   all_leds_off();
   digitalWrite(LED_BLUE,ON);
}

void led_green()
{
   all_leds_off();
   digitalWrite(LED_GRN,ON);
}


void buzzer_on()
{
  digitalWrite(BUZ,HIGH);
}

void buzzer_off()
{
  digitalWrite(BUZ,LOW);
}


void ProcessGpsData() {
    char tmp[50];
    memset(gpsdata,'\0',sizeof(gpsdata));

    strcpy(gpsdata,"Date: ");
    sprintf(tmp,"%01d/",GPS.day);
    strcat(gpsdata,tmp);
    sprintf(tmp,"%01d/",GPS.month);
    strcat(gpsdata,tmp);
    sprintf(tmp,"20%d",GPS.year);
    strcat(gpsdata,tmp);

    strcat(gpsdata," Time: ");
    sprintf(tmp,"%01d:",GPS.hour);
    strcat(gpsdata,tmp);
    sprintf(tmp,"%01d:",GPS.minute);
    strcat(gpsdata,tmp);
    sprintf(tmp,"%01d",GPS.seconds);

    strcat(gpsdata," Loc: ");
    if (GPS.lat == 'S') {
        strcat(gpsdata,"-");
    }
    dtostrf( GPS.latitude/100, 3, 6, tmp );
    strcat(gpsdata,tmp);
    strcat(gpsdata, ",");
    if (GPS.lon == 'W') {
        strcat(gpsdata,"-");
    }
    dtostrf( GPS.longitude/100, 3, 6, tmp );
    strcat(gpsdata,tmp);
    strcat(gpsdata, ", ");

    dtostrf( GPS.speed * 1.852, 3, 1, tmp );
    strcat(gpsdata,"Speed: "); 
    strcat(gpsdata,tmp);
    strcat(gpsdata," kph");
}

bool OpenSDLogFile() {
    SDLogFile = SD.open("caralog.txt", FILE_WRITE);
    if (SDLogFile) 
        return true;
    return false;
}


void CloseSDLogFile() {
    SDLogFile.close();
}

void ShowLogfile() {
    led_blue();
    if (!SD.begin(chipSelect)) {
      Serial.println("Card failed, or not present");
      // don't do anything more:
      while (1);
    }
    Serial.println("card initialized.");
    File dataFile = SD.open("caralog.txt");

    // if the file is available, read its contents and show on serial port:
    if (dataFile) {
      while (dataFile.available()) {
        Serial.write(dataFile.read());
      }
      dataFile.close();
    }
    while ( digitalRead(PBUT1) == LOW ) {
      delay(100);    
    };
  }

  void DeleteLogfile() {
    led_blue();
    if (!SD.begin(chipSelect)) {
      Serial.println("Card failed, or not present");
      // don't do anything more:
      while (1);
    }
    Serial.println("card initialized.");

    if ( SD.exists("caralog.txt") ) {
      SD.remove("caralog.txt");
    }
    
     if ( !SD.exists("caralog.txt") ) {
      Serial.println("File 'caralog.txt' deleted from SD card");
    }
    while ( digitalRead(PBUT3) == LOW ) {
      delay(100);    
    };
  }

  void LogCaraData() {

    if (CaraSerial.available()) {
      buzzer_on();
      timer = millis(); // reset the timer
      // if the file is already opened okay, write to it:
      if (SDFileOpen ) {
        led_blue();
        char c[2];
        c[0] =  CaraSerial.read();
        c[1] = '\0';
        Serial.write(c);    // Echo on serial port
        SDLogFile.print(c);  // And write to disk
      }
      else{
        if (OpenSDLogFile() ) {
          SDFileOpen = true;
        }
      }   
    } 
    else {
          if (millis() - timer > 50) {      
            if (SDFileOpen) {
              if (GPS.fix) {
                ProcessGpsData();
              }
              else {
                strcpy(gpsdata, "NO GPS FIX!!!");
              }
              Serial.println();
              SDLogFile.println();
              Serial.println(gpsdata);
              SDLogFile.println(gpsdata);
              SDLogFile.println();
              Serial.println();
              CloseSDLogFile();
              SDFileOpen = false;
            }
          }
          if (millis() - timer > 90) { 
            timer = millis(); // reset the timer 
            if (GPS.fix) {
              led_green();
            }
            else { 
              all_leds_off();
            }
          }  
      }
     
      if (Serial.available()) {        // If anything comes in Serial (USB), we send it to the CaRA device
        CaraSerial.write(Serial.read());  
      }
       buzzer_off();
  }

void loop() {


  if (!CaraSerial.available()) {
    switch (bootlevel) {
      case 0 : CaraSerial.println("$SAC*"); // Request Crash settings
              bootlevel++; 
              break;
      case 1 : CaraSerial.println("$SGC*");  // Request Rollover settings
            bootlevel++; 
            break;
      case 2 : CaraSerial.println("$PVV*");   // Request Suppply Voltage settings
            bootlevel++; 
            break;   
      case 3 : CaraSerial.println("$PRV*");   // Request current Pitch and Roll values
            bootlevel++; 
            break;   
      case 4 : CaraSerial.println("$TST*");   // Request self test      
            bootlevel++; 
            break;   
      case 6 : CaraSerial.println("$LVC*");   // Request software version  
            bootlevel++; 
            break;
      default : break;        
    }
    if (bootlevel <= 6) {
      delay(100);
    }
  }

   
    
  // read data from the GPS in the 'main loop'
  char c = GPS.read();
  if (GPSECHO) {
    if (c) Serial.print(c);
  }

  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
      if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
        return; // we can fail to parse a sentence in which case we should just wait for another
  }

  LogCaraData();

  if (digitalRead(PBUT1) == LOW)  {
      ShowLogfile();
  }

  if (digitalRead(PBUT3) == LOW)  {
    DeleteLogfile();
  }





   
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off
 
}
