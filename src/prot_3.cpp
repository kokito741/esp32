#include "Adafruit_Sensor.h"
#include <Arduino.h>
#include "DHT.h"
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <FirebaseESP32.h>
#include <Firebase_ESP_Client.h>
#define DHT_SENSOR_TYPE DHT22
#define DHT_SENSOR_PIN 22
DHT dht=DHT(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
//Provide the token generation process info this will mostly help us debug the progress and the state of token generation
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
#define API_KEY "AIzaSyBU7Tn2EOD03z5eOseQ2rbKoBzbS3waS9w"
#define DATABASE_URL "https://esp32-dd238-default-rtdb.europe-west1.firebasedatabase.app/"
FirebaseData firebaseData;


const char* SSID = "kokinetwork-2G";
const char* PASSWORD = "0887588455";
#define DEVICE_ID "esp32-dev-1"

#define LOCATION "Living Room"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson Tempreature_json;
FirebaseJson Humidity_json;
FirebaseJson DateTaken_json;
String path="";//base path
String currentdata_path="";
String temp_path="";
String hum_path="";
String tim_path="";
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
float humidity=0.0;
float temperature=0.0;
std::string formattedDate="0.0.0.0";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
void FireBase_init(){
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
    config.api_key = API_KEY;
    auth.user.email = "esp32-dev@abv.bg";
    auth.user.password =  "987456321k";
    config.database_url = DATABASE_URL;
    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);
    Serial.print("Signing in ... ");
    config.token_status_callback = tokenStatusCallback;
    config.max_token_generation_retry = 5;

    Firebase.begin(&config, &auth);
    Serial.println(" firebase begin");
    Serial.println( Firebase.ready());
    Serial.println( "WAITING FOR FIREBASE TOKEN GENERATION");
    while (!Firebase.ready()){ Serial.print("..");delay(300);    };
    path += auth.token.uid.c_str(); //<- user uid
    path+="/";
    path+=LOCATION;
    path+="/";

}
void Json_init()
{
    Tempreature_json.add("Value",temperature);

    Humidity_json.add("Value",humidity);

}
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
    dht.begin();
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    timeClient.begin();
    timeClient.setTimeOffset(7200);
    FireBase_init();
    Json_init();
    Serial.println(currentdata_path);
    Serial.println(temp_path);
    Serial.println(hum_path);
    Serial.println(String(millis()));




}
void loop() {
    delay(10000);
    Serial.println("Delay finished");
    digitalWrite(LED_BUILTIN, HIGH);
    float temperature= dht.readTemperature();
    float humidity=dht.readHumidity();
    Serial.println("Sensors recorded");
    Serial.println("BEGIN DATE TAKEN");
//CURENT DATA
    timeClient.forceUpdate();
    String weekDay = weekDays[timeClient.getDay()];
    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime ((time_t *)&epochTime);
    int monthDay = ptm->tm_mday;
    int currentMonth = ptm->tm_mon+1;
    String currentMonthName = months[currentMonth-1];
    int currentYear = ptm->tm_year+1900;
    String currentDate = String(monthDay) + "-" + String(currentMonth) + "-" + String(currentYear)+" - " + timeClient.getFormattedTime();
    Serial.print("Current date: ");
    Serial.println(currentDate);







    Serial.println("path build start");
    currentdata_path = path+currentDate+"/"+DEVICE_ID;
    //currentdata_path = "cUgRiGk3v1Yfi59j0eOlNrK7gnn1/Living Room/24-12-2023 - 21:57:32/esp32-dev-1";
    Serial.println(currentdata_path);

    temp_path=currentdata_path+"/temperature";
    hum_path=currentdata_path+"/humidity";
    Serial.println("path build finished");
    Serial.println("Json set start");
    Tempreature_json.set("Value",temperature);
    Humidity_json.set("Value",humidity);
    Serial.println("Json set finished");
    Serial.println("Updating firebase");
    Firebase.updateNode(fbdo, temp_path , Tempreature_json);
    Firebase.updateNode(fbdo, hum_path , Humidity_json);
    Serial.println("Update firebase complete");
    Serial.println("Delay started");
    digitalWrite(LED_BUILTIN, LOW);
}
