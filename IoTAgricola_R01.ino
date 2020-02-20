#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <SoftwareSerial.h>

#define RX 10
#define TX 11

// Digital Pin DHT11 sensor
#define DHTPIN 2

// Sensor Type
#define DHTTYPE DHT11
 
// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

//HumidityG Sensor Pin
const int sensorPin = A0;

//Relay Sensor Pin
const byte pinRele = 9;




//String AP = "WIFI_NAME";       // CHANGE ME
//String PASS = "WIFI_PASSWORD"; // CHANGE ME
//String API = "YOUR_API_KEY";   // CHANGE ME

String AP = "JONINO";       // CHANGE ME
String PASS = "79514933"; // CHANGE ME
String APIKey = "17NUWNP3IBE3QG5L";   // CHANGE ME
String URLHost = "GET /update?api_key="+ APIKey +"&";


//Thingspeak params
String HOST = "api.thingspeak.com";
String PORT = "80";


int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
int valSensor = 1;
SoftwareSerial esp8266(RX,TX); 

//Data
String field1 = "field1";
String field2 = "field2";
String field3 = "field3";
String field4 = "field4";

float temperatureC;
float humidityA;
float temperatureF;
float heatIndexF,heatIndexC;
int   humidityG;


//Var control riego
int limiteMinimoHumedad = 50; //0 = Completamente seco, 100 = Complentamente humeda





 
  
void setup() {
  Serial.begin(9600);
  esp8266.begin(115200);
  dht.begin();

  //Rlay Setup
  pinMode(pinRele, OUTPUT);

  
  //Verify WiFi Status
  sendCommand("AT",5,"OK");
  
  //Set Wifi Mode: 1: Stations Mode
  sendCommand("AT+CWMODE=1",5,"OK");
  
  //Connect to WiFi
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
}
 
void loop() {
  
  valSensor = getSensorData();
  
  //Config URL ThinkSpeak API
  //String APIData = URLHost + field +"="+String(valSensor);
  String APIData = URLHost + field1 +"="+String(temperatureC) + "&" + field2 +"="+String(humidityA) + "&" + field3 +"="+String(heatIndexC) + "&" +field4 +"="+String(humidityG);
  
  //Enable multiple connections: 1: Multiple connections
  sendCommand("AT+CIPMUX=1",5,"OK");
  
  //Establish TCP Connection with the HOST (ThingSpeak)
  //sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");

  //Establish TCP Connection with the HOST (NodeRed)
  sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");

  
  //Send data
  Serial.print("API Data");
  Serial.print(APIData);
  sendCommand("AT+CIPSEND=0," +String(APIData.length()+4),4,">");

  
  esp8266.println(APIData);
  delay(1500);
  countTrueCommand++;
  
  //Close TCP Connection
  sendCommand("AT+CIPCLOSE=0",5,"OK");
}

//Get info data sensor
int getSensorData(){
  
  //return random(1000); // Replace with 

  delay(5000);

  Serial.println("temperature");
  
 
  // Leemos la humedad relativa
  humidityA = dht.readHumidity();
  Serial.print(humidityA);
  // Leemos la temperatura en grados centígrados (por defecto)
  
  temperatureC = dht.readTemperature();
  Serial.print(temperatureC);
  
  // Leemos la temperatura en grados Fahreheit
  temperatureF = dht.readTemperature(true);
  Serial.print(temperatureF);
  
  //Get growth humidity

  humidityG= getSoilMoisture();
  
  
  
 
  // Comprobamos si ha habido algún error en la lectura
  if (isnan(humidityA) || isnan(temperatureC) || isnan(temperatureF)) {
    Serial.println("Error obteniendo los datos del sensor DHT11");
    humidityA=0;
    temperatureC= 0;
    temperatureF= 0;
    return;
  }
 
  // Calcular el índice de calor en Fahreheit
  heatIndexF = dht.computeHeatIndex(temperatureF, humidityA);
  
  // Calcular el índice de calor en grados centígrados
  heatIndexC = dht.computeHeatIndex(temperatureC, humidityA, false);
 
  Serial.print("Humedad: ");
  Serial.print(humidityA);
  Serial.print(" %\t");
  Serial.print("Temperatura: ");
  Serial.print(temperatureC);
  Serial.print(" *C ");
  Serial.print(temperatureF);
  Serial.print(" *F\t");
  Serial.print("Índice de calor: ");
  Serial.print(heatIndexC);
  Serial.print(" *C ");
  Serial.print(heatIndexF);
  Serial.print(" *F");
  Serial.print("Humedad suelo: ");
  Serial.print(humidityG);
  Serial.println(" %\t");
  
  
}

//Send Code
void sendCommand(String command, int maxTime, char readReplay[]) {
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
    Serial.println("Response OK");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Response FAIL");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
 }


//Get Soil Moisture
int getSoilMoisture(){

  int humidity = analogRead(sensorPin);
  humidityG = map((analogRead(sensorPin)),0,1023,100,0);
  Serial.print("humidityG: ");
  
  Serial.println(humidityG);
  //Control de flujo de agua
   if (humidityG >= limiteMinimoHumedad){
    // Close flow
    Serial.print("Close flow");
  
    
    digitalWrite(pinRele, 1);
    //digitalWrite(pinRele, !digitalRead(pinRele));
    }
    else{
      //Open flow
      Serial.print("Open flow");
  
      digitalWrite(pinRele, 0);
      }
  return humidityG;
  
}

//Setup irrigation
void setIrrigation(){
  
  }


//JSON Serialize
/*void json() {
  String humedad = String(humidity, 2); 
  String temperatura = String(temperatureC, 2);
  payload = "{";
  payload += "\"temperatura\":"; payload += temperatura; payload += ",";
  payload += "\"humedad\":"; payload += humedad; payload += "}";
}*/
 
