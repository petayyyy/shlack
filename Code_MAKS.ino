// WiFi
#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <OneWire.h>
#include <ArduinoJson.h>

// Sensor
int sensor_pin = A0;

// Display
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);

// Sensor
#define sensorCount 2
char* sensorName[] = {"Data","Indicator"};
float sensorValues[sensorCount];

// Name Sensor
#define  Data 0
#define  Indicator 1

// WiFi config
WiFiServer server(80);
char ssid[] = "Galaxy S10b57e";
char pass[] = "1234567890";

// ThingWorx
char iot_server[] = "pp-2101111453dh.devportal.ptc.io";
//IPAddress iot_address(192,168,43,36);
char appKey[] = "c8036c7b-af01-4800-8897-c51c539586c7";
char thingName[] = "Thing1";
char serviceName[] = "code1";

// Timer
long timer_iot_timeout = 0;
#define TIMEOUT 1000 // 1 second timout
#define IOT_TIMEOUT1 5000
#define IOT_TIMEOUT2 100
unsigned long timer_thingworx = 0;
unsigned long timer_print = 0;
unsigned long timer_auto = 0;
unsigned long timer_sensors = 0;

#define BUFF_LENGTH 256
char buff[BUFF_LENGTH] = "";
int btn_state = 0;
int auto_control = 0;

void setup() {
  Serial.begin(115200);
  
  Serial.println("Conecting to WiFi");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println(".");
    }
  Serial.println("Local IP:");
  Serial.println(WiFi.localIP());
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
}

void GetData(){
  sensorValues[Data] = analogRead(sensor_pin); // Считываем показания с датчика
  sensorValues[Data] = map(sensorValues[Data], 579, 0, 0, 100);
  if (isnan(sensorValues[Data]))
  {
    Serial.println("Failed to read from sensor!");
    sensorValues[Data] = 70;
  }
}

void printData()
{
  Serial.println("Обработанные данные:");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd_printstr("Data = " + String(sensorValues[Data]) + " %");
  Serial.println("Data = " + String(sensorValues[Data]) + " %");
  lcd.setCursor(0, 1);
  print_line();  
  Serial.println();
}
void print_line(){
  for (int i = 0; i < sensorValues[Indicator]; i++)
    {
      lcd.print(".");
      Serial.print(".");
    }
}
void lcd_printstr(String str1)
{
  for (int i = 0; i < str1.length(); i++)
  {
    lcd.print(str1.charAt(i));
  }
}

void sendThingWorxStream()
{ 
  WiFiClient client = server.available();
  // Подключение к серверу
  Serial.println("Connecting to IoT server...");
  if (1)
  {
    // Проверка установления соединения
    if (1)
    {
      // Отправка заголовка сетевого пакета
      Serial.println("Sending data to IoT server...\n");
      Serial.print("POST pp-2101111453dh.devportal.ptc.io/Thingworx/Things/");  client.print("POST pp-2101111453dh.devportal.ptc.io/Thingworx/Things/");
      Serial.print(thingName);  client.print(thingName);
      Serial.print("/Services/"); client.print("/Services/");
      Serial.print(serviceName); client.print(serviceName);
      Serial.print("?appKey="); client.print("?appKey=");
      Serial.print(appKey);  client.print(appKey);
      Serial.print("&method=post&x-thingworx-session=true");  client.print("&method=post&x-thingworx-session=true");
      
      // Отправка данных с датчиков
      Serial.println();
      Serial.println("Отправляем данные:");
      Serial.print("&"); client.print("&");Serial.print(sensorName[0]);  client.print(sensorName[0]);
      Serial.print("=");  client.print("=");Serial.print(sensorValues[0]); client.print(sensorValues[0]);
      Serial.println();
      client.println(" HTTP/1.1");  client.println("Accept: application/json");   client.print("Host: ");   client.println(iot_server);   client.println("Content-Type: application/json");   client.println();
      
      // Ждем ответа от сервера
      timer_iot_timeout = millis();
      while ((client.available() == 0) && (millis() < timer_iot_timeout + IOT_TIMEOUT1)) {delay(10);}

      // Выводим ответ о сервера, и, если медленное соединение, ждем выход по таймауту
      int iii = 0;
      bool currentLineIsBlank = true;
      bool flagJSON = false;
      timer_iot_timeout = millis();
      while ((millis() < timer_iot_timeout + IOT_TIMEOUT2) && (client.connected()))
      {
        while (client.available() > 0)
        {
          char symb = client.read();
          //Serial.print(symb);
          if (symb == '{')
          {
            flagJSON = true;
          }
          else if (symb == '}')
          {
            flagJSON = false;
          }
          if (flagJSON == true)
          {
            buff[iii] = symb;
            iii ++;
          }
          delay(10);
          timer_iot_timeout = millis();
        }
        delay(10);
      }
      buff[iii] = '}';
      buff[iii + 1] = '\0';
      Serial.println("////////////////");
      Serial.println(buff);
      StaticJsonBuffer<BUFF_LENGTH> jsonBuffer;
      JsonObject& json_array = jsonBuffer.parseObject(buff);
      sensorValues[Indicator] = json_array["Indicator"];
      
      // Закрываем соединение
      client.stop();
    }
  }
}
void loop() {
  GetData();
  sendThingWorxStream();
  printData();
  delay(3000);
}
