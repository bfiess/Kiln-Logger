/*
  Web client
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 */

// Updates Needed!!!
// Smoothing/Averaging: throw out outliers
// speaker and beepc


#include <SPI.h>
#include <Ethernet.h>
#include "Adafruit_MAX31855.h"
#include <dht.h>
dht DHT;
#define DHT22_PIN 5
#define DO   7
#define CS   9
#define CLK  2
Adafruit_MAX31855 thermocouple(CLK, CS, DO);
#include <Wire.h>
#include <utility/Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>
//declare LCD shield
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

unsigned long PreviousMillis = 0;// For when millis goes past app 49 days. 
unsigned long PreviousMillis2 = 0;// For when millis goes past app 49 days. 
unsigned long PreviousMillis3 = 0;// For when millis goes past app 49 days. 
//unsigned long interval = 10000;  // Wait between dumps (10000 = 10 seconds)
unsigned long interval = 300000;  // Wait between dumps (5 min)
unsigned long interval2 = 5000;  // Wait between dumps (5 min)
unsigned long interval3 = 90000;  // Wait between dumps (5 min)
unsigned long intervalTime;      // Global var tracking Interval Time
unsigned long intervalTime2;      // Global var tracking Interval Time
unsigned long intervalTime3;      // Global var tracking Interval Time
int rate[11]; // Temperature samples every 1.5 minutes
int rateHolder; // rate counter
int ror=0; // Rate of Rise
int highTemp = 0; // peak temperature
int power = 0; // Set to specify infinite switch power setting
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50; 
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
IPAddress server(172,21,121,220);  // numeric IP for Google (no DNS)
//char server[] = "www.google.com";    // name address for Google (using DNS)

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(172,21,121,230);

byte subnet[] = {
  255,255,0,0};

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  lcd.begin(16,2);

  // start the Ethernet connection:
  intervalTime = millis(); 
  Ethernet.begin(mac, ip, subnet);

  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");

  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected");
    client.println( " HTTP/1.1");
    client.println( "Host: 172.21.121.220" );
    client.print(" Host: ");
    client.println(server);
    client.println( "Connection: close" );
    client.println();
    client.println();
    client.stop();
  } 
  else {
    // kf you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}

uint8_t i=0;
void loop()
{
  
   uint8_t buttons = lcd.readButtons();

  if (buttons) {
    lcd.clear();
    lcd.setCursor(0,0);
    if (buttons & BUTTON_UP) {
      power=power+1;
      lcd.print(power);
      delay(200);
    }
    if (buttons & BUTTON_DOWN) {
      power=power-1;
      lcd.print(power);
      delay(200);
    }
    if (buttons & BUTTON_LEFT) {
      lcd.print("High Reset ");
      highTemp = 0;
    }
    if (buttons & BUTTON_RIGHT) {
      lcd.print("High:");
      lcd.print(highTemp);
    }
    if (buttons & BUTTON_SELECT) {
      lcd.print(ror);
    }
  }
  
  
  unsigned long CurrentMillis = millis();
  unsigned long CurrentMillis2 = millis(); 
  unsigned long CurrentMillis3 = millis(); 
  if ( CurrentMillis < PreviousMillis ) // millis reset to zero?
  {
    intervalTime = CurrentMillis+interval;
  }
  if ( CurrentMillis2 < PreviousMillis2 ) // millis reset to zero?
  {
    intervalTime2 = CurrentMillis2+interval2;
  }
  if ( CurrentMillis3 < PreviousMillis3 ) // millis reset to zero?
  {
    intervalTime3 = CurrentMillis3+interval3;
  }
  if ( CurrentMillis3 > intervalTime3 )
  {
    intervalTime3 = CurrentMillis3 + interval3;
    Serial.print("counter: ");
    Serial.println(rateHolder);
    if ( rateHolder < 11 )
    {
    rate[rateHolder] = sampleKiln(10);
    }
    if (rateHolder >= 11)
    {
      for (int t = 0; t < 11 ; t++ )
      {
        rate[t] = rate[t+1];
      }
      rate[10] = sampleKiln(10);
      ror = (rate[10] - rate[2])*5;
    }
    for (int w = 0; w < 11 ; w++ )
      {
        Serial.print("[");
        Serial.print(w);
        Serial.print("] ");
        Serial.println(rate[w]);
      }
    rateHolder++;
    

}
  if ( CurrentMillis2 > intervalTime2 )  // Did we reach the target time yet?
  {
    intervalTime2 = CurrentMillis2 + interval2;
    lcd.setCursor(0, 0);
    lcd.print("KILN: ");
    int kilnTemp = sampleKiln(10);
    lcd.print(lcdOutput(kilnTemp));
    lcd.print(kilnTemp);
    lcd.print((char)223);
    lcd.print("F ");
    if (power < 10 ) {
      lcd.print("0");
    }lcd.print(power);
    lcd.setCursor(0, 1);
    lcd.print("ROOM: ");
    int chk = DHT.read22(DHT22_PIN);
    lcd.print(int((DHT.temperature* 9 +2)/5+32));
    lcd.print((char)223);
    lcd.print("F ");
    lcd.print(int(DHT.humidity));
    lcd.print("%");
    
  }
  if ( CurrentMillis > intervalTime )  // Did we reach the target time yet?
  {
    intervalTime = CurrentMillis + interval;



    if (client.connect(server, 80)) {
      Serial.println("connected");
      // Make a HTTP request:
      client.print("GET /~benfiess/add_data.php?");
      client.print("serial=");
      client.print("testserial");
      client.print("&temperature=");
      client.print(sampleKiln(10));
      //  client.print(thermocouple.readFarenheit());
      client.print("&room=");
      int chk = DHT.read22(DHT22_PIN);
      double thermometer = (DHT.temperature* 9 +2)/5+32;
      client.print(thermometer);
      client.print("&humidity=");
      double humidity = (DHT.humidity);
      client.print(humidity);
      client.print("&power=");
      client.print(power);
      client.print("&ror=");
      client.println(ror);
      client.println( " HTTP/1.1");
      client.println( "Host: 172.21.121.220" );
      client.print(" Host: ");
      client.println(server);
      client.println( "Connection: close" );
      client.println();
      client.println();
      client.stop();
    } 
    else {
      // kf you didn't get a connection to the server:
      Serial.println("connection failed");
    }
  }


}

double sampleKiln (int maxSamples)
{
  double kilnAvg;
  for (int y = 0; y < 5; y++)
  {

    double result = NAN;
    for (int i = 0; i < maxSamples; i++)
    {
      result = thermocouple.readFarenheit();
      if (!isnan(result)) //exit on first good reading.
        break;
      delay(10);
    }
    kilnAvg = kilnAvg + result;
    delay(10);
  }
  if ((kilnAvg/5) > highTemp) {
    highTemp = kilnAvg/5;
  }
  return kilnAvg/5;
}

String lcdOutput(int kilnTemp) 
{
  String leading = "";
  if ( kilnTemp < 100 ) { leading = "00"; }
  else if ( kilnTemp >= 100 && kilnTemp < 1000 ) { leading = "0"; }
  else if ( kilnTemp >= 1000 ) { leading = ""; }
  return leading;
}
