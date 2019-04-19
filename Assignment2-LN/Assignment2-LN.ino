/*
 * Leana Nakkour 
 * HCDE 440
 * Assignment 2
 * April 18, 2019
*/

/*
 * Current Problems:
 *  Can't connect to Weather API, verify API call is correct
 *  Can't access the specific weather information
 *  Can I connect/use Http client? Not working --- updated ESP8266 and now it hilights HTTPClient object
    Error compiling for board Adafruit Feather HUZZAH ESP8266. [vesion 2.5.0 installed] --- works now, installed version 2.4.2 instead
 */


/******************** SCENARIO AND DESCRIPTION **********************/
/*   Scenario: A wedding planner is trying to determine if 
 *   they should set up their clients' reception indoors in
 *   the reserved hotel ballroom or arrange for seating to be set
 *   up out in the hotel's garden. 
 *   
 *   The Geolocation and Open Weather APIs detect the upcoming weather forecast, 
 *   particularly rain, and uses a [FIGURE IT OUT] to alert the wedding 
 *   planner before reception setup is to commence at the hotel.
 *  
 */

#include "config.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>        //provides the ability to parse and construct JSON objects

const char* key = "caaa012260600520cacdd6dc03229830";           // Personal API key for accurate Geolocation tracking of IP addresses
const char* weatherAPIkey= "e8f7cdfb1bd8edcf0475cb27ef2b30d0";  // Personal API key for Open Weather API

/*  here we create a new data type definition, a box to hold other data types
 *  for each name:value pair coming in from the service, we will create a slot
 *  in our structure to hold our data
 */
 
typedef struct { 
  String ip;  // Variable to store IP address as a string    
  String cc;  // Variable to store Continent Code as a string
  String cn;  // Variable to store Continent Name as a string
  String rc;  // Variable to store Region Code as a string
  String rn;  // Variable to store Region Name as a string
  String cy;  // Variable to store City as a string
  String ln;  // Variable to store Longitude as a string
  String lt;  // Variable to store Latitude as a string
    
} GeoData;     //then we give our new data structure a name so we can use it in our code

GeoData location; //we have created a GeoData type, but not an instance of that type,
                  //so we create the variable 'location' of type GeoData

typedef struct {
  float tp;  // Variable to store Temperature as a float number
  String pr;  // Variable to store Pressure as a string
  String hd;  // Variable to store Humidity as a string
  String ws;  // Variable to store Wind Speed as a string
  String wd;  // Variable to store Weather Description as a string
  String wt;  // Variable to store Weather Type as a string
  
} MetData;

MetData conditions; // Creates the instance of a MetData type with the variable "conditions"

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.print("Connecting to "); Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) { // While the Wifi status reads that it is not connected
    delay(500);                           // Delay for half a second
    Serial.print(".");                    // Prints a "."
  }

  Serial.println(); Serial.println("WiFi connected"); Serial.println();
  //Serial.print("Your ESP has been assigned the internal IP address ");
  // IP address 192.168.43.64
  //Serial.println(WiFi.localIP());

  getGeo(); 

/*  The next lines prints out sentences state the 
 *   wedding planner's name and the agenda (Wedding) of the day
 *   and uses the Geolocation API to generate the location of the 
 *   wedding venue by it's City and State.
 */
 
  Serial.println("Wedding Planner: Janet May");
  Serial.println("Today: Tim and Tana's Wedding at the Hilton in " + location.cy + ", " + location.rc + ".");

  getMet(); 

/*  The next lines print out the current weather conditions in the area 
 *  where the wedding is being held (location.cy, location.rc) and whether 
 *  the reception can take place outdoors or should be held indoors.
 */

    Serial.println("The current weather in " + location.cy + ", " + location.rc + " is " + conditions.wt + ". More specifically, there is " + conditions.wd);
    Serial.println("The current temperature is: " + String(conditions.tp) + " degrees Fahrenheit.");
    if(conditions.wt == "Rain" || conditions.wt == "Wind" || conditions.wt == "Snow" ||conditions.tp > 69 ) {
      Serial.println("Setup reception indoors.");
    } else { 
      Serial.println("Setup reception outdoors.");
    }
}

void loop() {


}

String getIP() {
  HTTPClient theClient;
  String ipAddress;

  theClient.begin("http://api.ipify.org/?format=json");
  int httpCode = theClient.GET();

  if (httpCode > 0) {
    if (httpCode == 200) {

      DynamicJsonBuffer jsonBuffer;

      String payload = theClient.getString();
      JsonObject& root = jsonBuffer.parse(payload);
      ipAddress = root["ip"].as<String>();

    } else {
      Serial.println("Something went wrong with connecting to the endpoint.");
      return "error";
    }
  }
  return ipAddress;
}

void getGeo() {
  HTTPClient theClient;
  Serial.println("Making HTTP request");
  theClient.begin("http://api.ipstack.com/" + getIP() + "?access_key=" + key); //return IP as .json object
  int httpCode = theClient.GET();

  if (httpCode > 0) {
    if (httpCode == 200) {
      Serial.println("Received HTTP payload.");
      DynamicJsonBuffer jsonBuffer;
      String payload = theClient.getString();
      Serial.println("Parsing...");
      JsonObject& root = jsonBuffer.parse(payload);

      // Test if parsing succeeds.
      if (!root.success()) {
        Serial.println("parseObject() failed");
        Serial.println(payload);
        return;
      }

      //Some debugging lines below:
      //      Serial.println(payload);
      //      root.printTo(Serial);

      //Using .dot syntax, we refer to the variable "location" which is of
      //type GeoData, and place our data into the data structure.

      location.ip = root["ip"].as<String>();            //we cast the values as Strings b/c
      location.cc = root["country_code"].as<String>();  //the 'slots' in GeoData are Strings
      location.cn = root["country_name"].as<String>();
      location.rc = root["region_code"].as<String>();
      location.rn = root["region_name"].as<String>();
      location.cy = root["city"].as<String>();
      location.lt = root["latitude"].as<String>();
      location.ln = root["longitude"].as<String>();

    } else {
      Serial.println("Something went wrong with connecting to the endpoint.");
    }
  }
}

/* Wrap your HTTP request in a function called getMet(), which will be similar to getIP()and getGeo(), 
 *  and print out the data as a message to the Serial Monitor using Imperial units. 
 *  You should report temperature, humidity, windspeed, wind direction, and cloud conditions. 
 *  Call the API by city name.
 */

void getMet() {
  HTTPClient theClient;
  String apiCall = "http://api.openweathermap.org/data/2.5/weather?q=Seattle";
  apiCall += "&units=imperial&appid=";
  apiCall += weatherAPIkey;
  theClient.begin(apiCall);
  int httpCode = theClient.GET();
  if (httpCode > 0) {

    if (httpCode == HTTP_CODE_OK) {
      String payload = theClient.getString();
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(payload);
      if (!root.success()) {
        Serial.println("parseObject() failed in getMet().");
        return;
      }

      conditions.tp = root["main"]["temp"].as<float>();
      conditions.hd = root["main"]["humidity"].as<String>();
      conditions.ws = root["wind"]["speed"].as<String>();
      conditions.wt = root["weather"][0]["main"].as<String>(); 
      conditions.wd = root["weather"][0]["description"].as<String>();

      // Debugging lines:
      // Serial.println("-----------WEATHER: " + conditions.wt);  
      // Serial.println("-----------DESCRIPTION: " + conditions.wd); 
      
    }
  }
  else {
    Serial.printf("Something went wrong with connecting to the endpoint in getMet().");
  }
}
