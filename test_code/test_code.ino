
//wi-fi esp-----
#include <SoftwareSerial.h>
#define RX 10
#define TX 11
SoftwareSerial esp8266(RX,TX); 

String AP = "AndroidAP1DDF";       // CHANGE ME
String PASS = "wzzu5744"; // CHANGE ME
String API = "3CFSQKW4X09T0DWL";   // CHANGE ME
String HOST = "api.thingspeak.com";
String PORT = "80";
//String field = "field1";
int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
int valSensor = 1;
 
//dht22--------
#include <DHT.h>
#include "DHT.h"
#define dht_pin 2
DHT dhtObject;
  
//DSM501A-----
int pin=8;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 20000;//sampe 1s ;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

//MQ135
int gas_sensor = A0; //Sensor pin 
float a= 5.1419;
float b= -0.3345;
float R0= 4.56; //din codul anterior

//mq7
int CO_sensor = A1; //Sensor pin 
float a7 = -0.6527; //Slope 
float b7 = 1.30; //Y-Intercept 
float R07 = 0.32; //Sensor Resistance

//mq2
int LPG_sensor= A2; //Sensor pin 
float a2 = 4.66921; //Slope 
float b2 = -0.31503; //Y-Intercept 
float R02 = 1.15; //Sensor Resistance


void setup() {
  Serial.begin(9600);
  Serial.println("Connecting..");
  esp8266.begin(115200);

  dhtObject.setup(2);

  pinMode(gas_sensor, INPUT); //Set gas sensor mq135 as input
  pinMode(A1,INPUT);//set A1 mq7
  pinMode(A2,INPUT); //set A2 mq2


  pinMode(8,INPUT); // senzor praf
  starttime = millis();
  for (int i = 10; i >= 1; i--) { // timp de asteptare pt senzor praf
    
    delay(1000);
  }


  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
  Serial.println("Connected");
}
void loop() {
 //valSensor = getSensorData();
 //String getData = "GET /update?api_key="+ API +"&"+ field +"="+String(valSensor);

 //praf----
  duration = pulseIn(pin, LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;
   if ((millis()-starttime) > sampletime_ms)
  {
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=>100
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
    Serial.print("concentration = ");
    Serial.print(concentration);
    Serial.print(" µg/m3  -  ");
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
  //mq135  
   float sensor_volt; //Define variable for sensor voltage 
  float RS_gas; //Define variable for sensor resistance  
  float ratio; //Define variable for ratio
  float sensorValue = analogRead(gas_sensor); //Read analog values of sensor  
  sensor_volt = sensorValue*(5.0/1023.0); //Convert analog values to voltage 
    RS_gas = ((5.0*10.0)/sensor_volt)-10.0; //Get value of RS in a gas
  ratio = RS_gas/R0;  // Get ratio RS_gas/RS_air
  double ppm_log = (log10(ratio)-b)/a; //Get ppm value in linear scale according to the the ratio value  
  double ppm = pow(10, ppm_log); //Convert ppm value to log scale 
  Serial.print("CO2 PPM = ");
  Serial.println(ppm);

//mq7
   float sensor_volt7; //Define variable for sensor voltage 
  float RS_gas7; //Define variable for sensor resistance  
  float ratio7; //Define variable for ratio
  float sensorValue7 = analogRead(CO_sensor); //Read analog values of sensor  
  sensor_volt7 = sensorValue7*(5.0/1023.0); //Convert analog values to voltage 
  RS_gas7 = ((5.0*10.0)/sensor_volt7)-10.0; //Get value of RS in a gas
  ratio7 = RS_gas7/R07;  // Get ratio RS_gas/RS_air
  double ppm_log7 = (log10(ratio7)-b7)/a7; //Get ppm value in linear scale according to the the ratio value  
  double ppm7 = pow(10, ppm_log7); //Convert ppm value to log scale 
  Serial.print("CO PPM = ");
  Serial.println(ppm7);

//mq2
   float sensor_volt2; //Define variable for sensor voltage 
  float RS_gas2; //Define variable for sensor resistance  
  float ratio2; //Define variable for ratio
  float sensorValue2 = analogRead(LPG_sensor); //Read analog values of sensor  
  sensor_volt2 = sensorValue2*(5.0/1023.0); //Convert analog values to voltage 
  RS_gas2 = ((5.0*10.0)/sensor_volt2)-10.0; //Get value of RS in a gas
  ratio2 = RS_gas2/R02;  // Get ratio RS_gas/RS_air
  double ppm_log2 = (log10(ratio2)-b2)/a2; //Get ppm value in linear scale according to the the ratio value  
  double ppm2 = pow(10, ppm_log2); //Convert ppm value to log scale 
  Serial.print("LPG PPM = ");
  Serial.println(ppm2);



 String getData = "GET /update?api_key="+ API +"&field1="+getTemperatureValue()+"&field2="+getHumidityValue()+"&field3="+String(concentration)+"&field4="+String(ppm)+"&field5="+String(ppm7)+"&field6="+String(ppm2);
 sendCommand("AT+CIPMUX=1",5,"OK");
 sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
 sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
 esp8266.println(getData);
 delay(1500);
 countTrueCommand++;
 sendCommand("AT+CIPCLOSE=0",5,"OK");

}
//int getSensorData()
//{return random(1000); // Replace with }

String getTemperatureValue(){

   float temperature = dhtObject.getTemperature();
   Serial.print(" Temperature(C)= ");
   Serial.println(temperature); 
   return String(temperature); 
}

String getHumidityValue(){

   float humidity = dhtObject.getHumidity();
   Serial.print(" Humidity in %= ");
   Serial.println(humidity);
   return String(humidity); 
}

void sendCommand(String command, int maxTime, char readReplay[]) 
{
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
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
  }
  
  found = false;
 }