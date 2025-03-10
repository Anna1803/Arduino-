// wifi----------------[
#include <SoftwareSerial.h> 
#define RX 2
#define TX 3
SoftwareSerial esp8266(RX,TX); 
String AP = "No Wi-Fi For You";
String PASS = "263487519";
String API = "V2DR3QMRD6NXU8PD";
String HOST = "api.thingspeak.com";
String PORT = "80";
String tempField = "field1";
String humField = "field3";
String getData;
int countTrueCommand;
int countTimeCommand;
boolean found = false; 
//--------------------]

// tft----------------[
#define sclk 13  // Don't change
#define mosi 11  // Don't change
#define cs   10  // If cs and dc pin allocations are changed then 
#define dc   9   // comment out #define F_AS_T line in "Adafruit_ILI9341_FAST.h"
#define rst  12  // Can alternatively connect this to the Arduino reset
#define BLACK    0x0000
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5

#include <Adafruit_GFX_AS.h>     // Core graphics library
#include <Adafruit_ST7735.h>
#include <Adafruit_ILI9341_AS.h> // Fast hardware-specific library
#include <SPI.h>

#define ILI9341_GREY 0x2104 // Dark grey 16 bit colour

Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);
//--------------------]

// dht----------------[
#include <DHT.h>; //pt DHT
#define DHTPIN 7
#define DHTTYPE DHT22
int chk;
float hum;
float temp;
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
//--------------------]

// senzor praf--------[
int pin = 8;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 20000;//sampe 1s ;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
//--------------------]

float voc; // senzor voc

unsigned long sequentialstart;
unsigned long displaystart;
String displayOrdine = "T";
String identificatorOrdine = "T";
String curent;

void setup() {
  Serial.begin(9600);
  esp8266.begin(9600); // wifi
  dht.begin(); // dht
  pinMode(8,INPUT); // senzor praf
  starttime = millis();

  tft.initR(INITR_BLACKTAB); // tft
  tft.fillScreen(BLACK); // tft
  tft.setRotation(135); // tft

  for (int i = 10; i >= 1; i--) { // timp de asteptare pt senzor praf
    tft.setTextSize(3);
    tft.setTextColor(ST7735_GREEN);
    tft.setCursor(65, 50);
    tft.print(i);
    
    delay(1000);
    tft.fillScreen(ILI9341_BLACK);
  }

  sequentialstart = millis();
  sequentialstart = sequentialstart + 25000;
  displaystart = millis();
  displaystart = displaystart + 5000;
  
  sendCommand("AT", 5, "OK");
  sendCommand("AT+CWMODE=1", 5, "OK");
  sendCommand("AT+CWJAP=\"" + AP + "\",\"" + PASS +"\"", 20, "OK");
}

void loop() {  
  // dht------------------------------------[
  temp = dht.readTemperature();
  Serial.print(temp); 
  Serial.println(" grade C");
  int xpos = 40, ypos = 10, gap = 10, radius = 40;

  if ((millis() - displaystart) > 5000 && displayOrdine == "T") {
    tft.fillScreen(ILI9341_BLACK);
    curent = "unu";
    displaystart = millis();
    displayOrdine = "H";
  }
    
  if ((millis() - sequentialstart) > 25000 && identificatorOrdine == "T") {
    getData = "GET /update?api_key="+ API +"&"+ tempField +"="+String(temp);
    sendCommand("AT+CIPMUX=1",5,"OK");
    sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
    sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
    esp8266.println(getData);delay(1500);countTrueCommand++;
    sendCommand("AT+CIPCLOSE=0",5,"OK");

    sequentialstart = millis();
    identificatorOrdine = "H";
  }

  hum = dht.readHumidity();
  Serial.print(hum); 
  Serial.println(" %");

  if ((millis() - displaystart) > 5000 && displayOrdine == "H") {
    tft.fillScreen(ILI9341_BLACK);
    curent = "doi";
    displaystart = millis();
    displayOrdine = "P";
  }
  
  if ((millis() - sequentialstart) > 25000 && identificatorOrdine == "H") {
    getData = "GET /update?api_key="+ API +"&"+ humField +"="+String(hum);
    sendCommand("AT+CIPMUX=1",5,"OK");
    sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
    sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
    esp8266.println(getData);delay(1500);countTrueCommand++;
    sendCommand("AT+CIPCLOSE=0",5,"OK");

    sequentialstart = millis();
    identificatorOrdine = "P";
  }
  //---------------------------------------]
  
  // senzor praf---------------------------[
  
  duration = pulseIn(pin, LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;
  if ((millis()-starttime) > sampletime_ms)
  {
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=>100
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
    Serial.print("concentration = ");
    Serial.print(concentration);
    Serial.print(" pcs/0.01cf  -  ");
    if (concentration < 1.0) {
       Serial.println("It's a smokeless and dustless environment"); 
    }
    if (concentration > 1.0 && concentration < 20000) {
       Serial.println("It's probably only you blowing air to the sensor :)"); 
    }
    
    if (concentration > 20000 && concentration < 315000) {
       Serial.println("Smokes from matches detected!"); 
    }
    if (concentration > 315000) {
       Serial.println("Smokes from cigarettes detected! Or It might be a huge fire! Beware!"); 
    }
    
    lowpulseoccupancy = 0;
    starttime = millis();
  }

  if ((millis() - displaystart) > 5000 && displayOrdine == "P") {
    tft.fillScreen(ILI9341_BLACK);
    curent = "trei";
    displaystart = millis();
    displayOrdine = "V";
  }

  if ((millis() - sequentialstart) > 25000 && identificatorOrdine == "P") {
    getData = "GET /update?api_key="+ API +"&"+ 5 +"="+String(concentration);
    sendCommand("AT+CIPMUX=1",5,"OK");
    sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
    sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
    esp8266.println(getData);delay(1500);countTrueCommand++;
    sendCommand("AT+CIPCLOSE=0",5,"OK");

    sequentialstart = millis();
    identificatorOrdine = "V";
  }
  //-------------------------------------]

  // senzor VOC-------------------------[
  voc = analogRead(A0);
  Serial.println(voc);
  Serial.println();

  if ((millis() - displaystart) > 5000 && displayOrdine == "V") {
    tft.fillScreen(ILI9341_BLACK);
    curent = "patru";
    displaystart = millis();
    displayOrdine = "T";
  }
  
  if ((millis() - sequentialstart) > 25000 && identificatorOrdine == "V") {
    getData = "GET /update?api_key="+ API +"&"+ 7 +"="+String(voc);
    sendCommand("AT+CIPMUX=1",5,"OK");
    sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
    sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
    esp8266.println(getData);delay(1500);countTrueCommand++;
    sendCommand("AT+CIPCLOSE=0",5,"OK");

    sequentialstart = millis();
    identificatorOrdine = "T";
  }
  //------------------------------------]

  if (curent == "unu") {
    xpos = gap + ringMeter(temp, -10, 50, xpos, ypos, radius, "mA", BLUE2RED);
    tft.setTextSize(2);
    tft.setCursor(5, 90);
    tft.print("Temp.: ");
    tft.print(temp);
    tft.print("   grade C");
  }
  else if (curent == "doi") {
    xpos = gap + ringMeter(hum, 0, 100, xpos, ypos, radius, "mA", GREEN2RED);
    tft.setTextSize(2);
    tft.setCursor(1, 95);
    tft.print("Umid.: ");
    tft.print(hum);
    tft.print("%");
  }
  else if (curent == "trei") {
    xpos = gap + ringMeter(concentration, 0, 1000000, xpos, ypos, radius, "mA", GREEN2RED);
    tft.setTextSize(2);
    tft.setCursor(5, 90);
    tft.print(" Conc. praf:");
    tft.print(concentration);
    tft.print(" pcs/L");
  }
  else if (curent == "patru") {
    xpos = gap + ringMeter(voc, 0, 800, xpos, ypos, radius, "mA", GREEN2RED);
    tft.setTextSize(2);
    tft.setCursor(5, 90);
    tft.print(" Voc: ");
    tft.print(voc);
    tft.print("     ppm");
  }
  
  delay(100);
}

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime * 1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
    tft.setTextSize(2);
    tft.setTextColor(ST7735_RED);
    tft.setCursor(55, 50);
    tft.print("FAIL!");
  }
  
  found = false;
}

// #########################################################################
//  Draw the meter on the screen, returns x coord of righthand side
// #########################################################################
int ringMeter(int value, int vmin, int vmax, int x, int y, int r, char *units, byte scheme)
{
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option
  
  x += r; y += r;   // Calculate coords of centre of ring

  int w = r / 4;    // Width of outer ring is 1/4 of radius
  
  int angle = 150;  // Half the sweep angle of meter (300 degrees)

  int text_colour = 0; // To hold the text colour

  int v = map(value, vmin, vmax, -angle, angle); // Map the value to an angle v

  byte seg = 5; // Segments are 5 degrees wide = 60 segments for 300 degrees
  byte inc = 5; // Draw segments every 5 degrees, increase to 10 for segmented ring

  // Draw colour blocks every inc degrees
  for (int i = -angle; i < angle; i += inc) {

    // Choose colour from scheme
    int colour = 0;
    switch (scheme) {
      case 0: colour = ILI9341_RED; break; // Fixed colour
      case 1: colour = ILI9341_GREEN; break; // Fixed colour
      case 2: colour = ILI9341_BLUE; break; // Fixed colour
      case 3: colour = rainbow(map(i, -angle, angle, 0, 127)); break; // Full spectrum blue to red
      case 4: colour = rainbow(map(i, -angle, angle, 63, 127)); break; // Green to red (high temperature etc)
      case 5: colour = rainbow(map(i, -angle, angle, 127, 63)); break; // Red to green (low battery etc)
      default: colour = ILI9341_BLUE; break; // Fixed colour
    }

    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (r - w) + x;
    uint16_t y0 = sy * (r - w) + y;
    uint16_t x1 = sx * r + x;
    uint16_t y1 = sy * r + y;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * 0.0174532925);
    float sy2 = sin((i + seg - 90) * 0.0174532925);
    int x2 = sx2 * (r - w) + x;
    int y2 = sy2 * (r - w) + y;
    int x3 = sx2 * r + x;
    int y3 = sy2 * r + y;

    if (i < v) { // Fill in coloured segments with 2 triangles
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
      text_colour = colour; // Save the last colour drawn
    }
    else // Fill in blank segments
    {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, ILI9341_GREY);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, ILI9341_GREY);
    }
  }

  // Convert value to a string
  char buf[10];
  byte len = 4; if (value > 999) len = 5;
  dtostrf(value, len, 0, buf);

  // Set the text colour to default
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  // Uncomment next line to set the text colour to the last segment value!
  // tft.setTextColor(text_colour, ILI9341_BLACK);

  // Calculate and return right hand side x coordinate
  return x + r;
}

// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
unsigned int rainbow(byte value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  byte red = 0; // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;// Green is the middle 6 bits
  byte blue = 0; // Blue is the bottom 5 bits

  byte quadrant = value / 32;

  if (quadrant == 0) {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}
