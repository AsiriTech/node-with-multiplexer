#include <ESP8266WiFi.h>
#define ledPower D1
#include "DHT.h"
#define MUX_A D4
#define MUX_B D3
#define MUX_C D2
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define ANALOG_INPUT A0

DHT dht(D2, DHT11);

char buffer[40];

LiquidCrystal_I2C lcd(0x3F, 20, 4); 
byte celcius[8] = {B00011, B00011, B00000, B11111, B10000, B10000, B10000, B11111};  //Celcius Custom Character

byte up_left[8] =   {B01100, B01000, B01000, B01000, B01000, B01000, B01000, B01000}; //box boders
byte up_right[8] =  {B00111, B00001, B01001, B00001, B00001, B01001, B00001, B00001}; //box boders
byte down_left[8] = {B00100, B00100, B00100, B00100, B00100, B00100, B00100, B00110}; //box boders
byte down_right[8] = {B00001, B00001, B00001, B00001, B00001, B00001, B00001, B00111}; //box boders

#define MUX_A D8
#define MUX_B D7
#define MUX_C D6

String apiKey = "O1FZUXQ5UUSKSTQA";
const char *ssid =  "Pixel_2606";
const char *pass =  "123456789";
const char* server = "api.thingspeak.com";

WiFiClient client;

//DHT dht(1, DHTTYPE);

int sampleingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;

float vOmeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

void setup() {
  //Deifne output pins for Mux
  Serial.begin(115200);
  pinMode(ledPower, OUTPUT);
  pinMode(MUX_A, OUTPUT);
  pinMode(MUX_B, OUTPUT);
  pinMode(MUX_C, OUTPUT);
  dht.begin();
  Wire.begin(2, 0);
  lcd.init();   // initializing the LCD
  lcd.backlight();  //LCD Backlight ON

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");


  lcd.createChar(4, celcius);
  lcd.createChar(5, up_left);    //define values to custom characters in Box
  lcd.createChar(6, up_right);   //define values to custom characters in Box
  lcd.createChar(7, down_left);  //define values to custom characters in Box
  lcd.createChar(8, down_right); //define values to custom characters in Box
}

void changeMux(int c, int b, int a) {
  digitalWrite(MUX_A, a);
  digitalWrite(MUX_B, b);
  digitalWrite(MUX_C, c);
}

void loop() {
  float value;

  changeMux(LOW, LOW, LOW);
  float value1 = analogRead(ANALOG_INPUT); //Value of the sensor connected Option 0 pin of Mux
  Serial.print("CO Value = ");
  Serial.print(value1 * 5);

  changeMux(LOW, LOW, HIGH);
  float value2 = analogRead(ANALOG_INPUT); //Value of the sensor connected Option 1 pin of Mux
  Serial.print("    CO2 Value = ");
  Serial.println(value2 * 5);

  changeMux(LOW, LOW, HIGH);
  float value3 = analogRead(ANALOG_INPUT); //Value of the sensor connected Option 1 pin of Mux
  Serial.print(" Noice Value = ");
  Serial.println(value3 * ( 5.0 / 1023));
  Serial.print(" ");

  changeMux(LOW, LOW, HIGH);
  float value4 = analogRead(ANALOG_INPUT); //Value of the sensor connected Option 1 pin of Mux
  Serial.print(" Particle = ");
  Serial.println(value4);
  Serial.print(" ");

  lcd.setCursor(0, 0);
  lcd.print("CO2:");
  lcd.setCursor(4, 0);
  lcd.print(value1 * 4);
  //lcd.setCursor(8, 0);

  lcd.setCursor(0, 1);
  lcd.print("CO:");
  lcd.setCursor(3, 1);
  lcd.print(value2 / 2);
  //lcd.setCursor(8, 0);

  lcd.setCursor(0, 2);
  lcd.print("Noice:");
  lcd.setCursor(6, 2);
  lcd.print(value3 / 5);
  //lcd.setCursor(8, 0);

  int h = dht.readHumidity();
  int t = dht.readTemperature();

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print("\t");
  Serial.print("Humidity: ");
  Serial.println(h);
  Serial.print(" ");

  lcd.setCursor(0, 3);
  lcd.print("Parti:");
  lcd.setCursor(6, 3);
  lcd.print(dustDensity);  ////particle senser value up there

  //temprature box
  lcd.setCursor(16, 0);
  lcd.print("TEM");
  lcd.setCursor(16, 1);
  lcd.print(t);
  lcd.setCursor(18, 1);
  lcd.write(byte(4));

  lcd.setCursor(16, 2);
  lcd.print("HUMI:");
  lcd.setCursor(16, 3);
  lcd.print(h);
  //lcd.setCursor(18,1);
  //lcd.write(byte(4));

  lcd.setCursor(15, 0);
  lcd.write(byte(5));
  lcd.setCursor(19, 0);
  lcd.write(byte(6));

  lcd.setCursor(15, 3);
  lcd.write(byte(7));
  lcd.setCursor(19, 3);
  lcd.write(byte(8));


  if (client.connect(server, 80)) {
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(t);
    postStr += "&field2=";
    postStr += String(h);
    postStr += "\r\n\r\n";

    //String postStr = apiKey;
    postStr += "&field3=";
    postStr += String(value1 * 4);
    postStr += "&field4=";
    postStr += String(value2 / 2);
    postStr += "&field5=";
    postStr += String(value3);
    postStr += "&field6=";
    postStr += String(dustDensity);
    postStr += "\r\n\r\n";


    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print("\t");
    Serial.print("Humidity: ");
    Serial.println(h);
    

    pinMode(ledPower, LOW);
    delayMicroseconds(sampleingTime);

    delayMicroseconds(deltaTime);
    digitalWrite(ledPower , HIGH);
    delayMicroseconds(sleepTime);

    calcVoltage = value4 * (5.0 / 1024.0);
    dustDensity = 170 * calcVoltage - 0.1;
    Serial.println(dustDensity);
    Serial.println("");

  }
  client.stop();
  delay(1000);
}
