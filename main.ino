#include <HTTPClient.h>
#include <ArduinoJson.h> 
#include <LiquidCrystal.h>
#include <WiFi.h>
#include "time.h"

// WiFi Configuration
const char* ssid = "KLEY Saclay"; // SSID of the public Wi-Fi network


// POST Authentication Data
const char* serverName = "https://wireless.wifirst.net/goform/HtmlLoginRequest";
String httpRequestData = "username=GUEST%2Fyour_username%40wifirst.net&password=your_password&success_url=https%3A%2F%2Fportal-selfcare.wifirst.net%2F&error_url=https%3A%2F%2Fportal-front.wifirst.net%2Fconnect-error&update_session=0";
const int maxAttempts = 5; // Maximum number of authentication attempts


// API Configuration
const String LineRef = ""; // Identifier for the line, currently unset
const String MonitoringRef = "STIF:StopPoint:Q:491107:"; // Identifier for the monitoring point
const char* apiURLBase = "https://prim.iledefrance-mobilites.fr/marketplace/stop-monitoring";
const char* apiKey = "bxnkeju2euPInjXpG1RcqJadePGunNCj"; // API key for authentication

unsigned long lastApiCallMillis = 0; // Timestamp of the last API call in milliseconds
const unsigned long apiCallInterval = 30000; // Interval between API calls (30 seconds)
time_t lastApiCallTimes[2] = {0, 0}; // Array to store the timestamps of the last two API calls


// Time Configuration
const char* ntpServer = "pool.ntp.org"; // NTP server for time synchronization
const long gmtOffset_sec = 0; // UTC offset in seconds
const int daylightOffset_sec = 0; // Daylight saving time offset in seconds
struct tm timeinfo = {0}; // Structure to hold the current time information


// LCD Display Initialization
LiquidCrystal lcd(33, 25, 26, 27, 14, 12); // Initializing the LCD display with pin numbers


// Push Button Configuration
const int buttonPin = 34; // GPIO pin connected to the push button
bool lastButtonState = HIGH; // Last known state of the button (HIGH indicates not pressed with a pull-up resistor)
unsigned long lastDebounceTime = 0; // Last time the button state was debounced
const unsigned long debounceDelay = 50; // Debounce delay in milliseconds


void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi network ");
  Serial.println(ssid);
  printlcd("Connecting to Wi-Fi network", 0, 1);
  printlcd(ssid, 1, 0);

  WiFi.begin(ssid); // Start Wi-Fi connection with the given SSID

  // Wait for the connection to be established
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000); // Wait a second before retrying
    Serial.print(".");
    printlcd(" . ", 0, 0); // Display progress dots on the LCD
  }

  // Once connected, display connection success and IP address on both Serial and LCD
  printlcd("Connected to Wi-Fi", 0, 1);
  printlcd(WiFi.localIP().toString(), 1, 0); // Display the assigned IP address
  printlcd(ssid, 1, 0);
  Serial.println("Connected to Wi-Fi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void authenticateToCaptivePortal() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Begin with the URL for initial access
    http.begin("https://portal-front.wifirst.net/"); // Initial GET request
    ;
    Serial.println("Making initial GET request to portal-front.wifirst.net...");

    int httpResponseCode = http.GET(); // Making a GET request

    if (httpResponseCode > 0) {
      Serial.print("Initial GET Response code: ");
      printlcd("Initial GET Response code: ",0,1);
      printlcd(String(httpResponseCode),1,0);
      Serial.println(httpResponseCode);
      if (httpResponseCode == 200) {
        http.end();
        return;
      }
      String response = http.getString(); // Get the response content for potential parsing
    } else {
      Serial.print("Error on initial GET: ");
      Serial.println(httpResponseCode);
      http.end(); // Important to close the connection before returning
      return;
    }

    http.end(); // Close the initial GET connection

    // Setup data for POST request
    http.begin("https://wireless.wifirst.net/goform/HtmlLoginRequest"); // URL for the POST request
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Specify content type

    // Construct the POST data. Make sure to replace placeholders with actual values
    String httpRequestData = "username=GUEST%2Fend_user_2172922@wifirst.net&password=be6255050256e8f8&success_url=https%3A%2F%2Fportal-selfcare.wifirst.net%2F&error_url=https%3A%2F%2Fportal-front.wifirst.net%2Fconnect-error&update_session=0";

    Serial.println("Sending authentication POST request...");
    printlcd("authentication POST request",0,1);


    httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      Serial.print("POST Response code: ");
      Serial.println(httpResponseCode);
      printlcd("POST Response code: ",0,1);
      printlcd(String(httpResponseCode),1,0);
      String response = http.getString(); // Optionally capture the response
      Serial.println(response);
    } else {
      Serial.print("Error on POST: ");
      Serial.println(httpResponseCode);
      printlcd("Error on POST: ",0,1);
      printlcd(String(httpResponseCode),1,0);
    }

    http.end(); // Close the connection
  } else {
    Serial.println("Not connected to WiFi, cannot authenticate.");
  }
}

bool testInternetConnection(String serverUrl = "http://worldtimeapi.org/api/timezone/Etc/UTC") {
  if (WiFi.status() == WL_CONNECTED) { // Check if connected to Wi-Fi
    HTTPClient http;

    http.begin(serverUrl); // Use the World Time API URL for testing the internet connection

    Serial.println("Testing internet connection with server: " + serverUrl);
    printlcd("Testing internet connection with server: " + serverUrl,0,0);
    int httpResponseCode = http.GET();

    http.end(); // Always close the connection

    if (httpResponseCode == 200) { // HTTP OK
      Serial.println("Internet connection is working!");
      printlcd("Internet connection is working!",0,0);
      return true; // Connection successful
    } else {
      Serial.print("Failed to connect to the internet, HTTP response code: ");
      Serial.println(httpResponseCode);
      return false; // Connection failed
    }
  } else {
    Serial.println("Not connected to WiFi, cannot test internet connection.");
    printlcd("Not connected to WiFi, cannot test internet connection.",0,0);
    return false; // WiFi is not connected
  }
}

String fetchAndProcessApiData(String LineRef , String MonitoringRef) {
  HTTPClient http;
  // Construct the full API URL with LineRef and MonitoringRef
  String fullApiURL = String(apiURLBase) + "?MonitoringRef=" + MonitoringRef;
  // If LineRef is provided, add it to the URL
  if (!LineRef.isEmpty()) {
    fullApiURL += "&LineRef=" + LineRef;
  }
  http.begin(fullApiURL);
  http.addHeader("apiKey", apiKey);
  int httpCode = http.GET();

  if (httpCode > 0) { 
    String payload = http.getString(); 
    http.end(); 
    return payload; 
  } else {
    Serial.print("Error on HTTP request: ");
    Serial.println(httpCode);
    http.end(); 
    return ""; 
  }
}

void extractAndAdjustExpectedArrivalTimes(const String &jsonPayload, time_t result[2]) {
  // Ensure that timeinfo is updated with the current time
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  // Use mktime to convert timeinfo to time_t for comparison
  time_t currentTime = mktime(&timeinfo);
  
  int index = 0, resultIndex = 0;
  while (resultIndex < 2) {
    int startPos, endPos, statusPos, statusEndPos;
    String statusStr;

    // Find and extract ExpectedArrivalTime
    startPos = jsonPayload.indexOf("\"ExpectedArrivalTime\":", index) + 23;
    if (startPos == 22) { // If no more ExpectedArrivalTime is found
      break; // Exit the loop
    }
    endPos = jsonPayload.indexOf("Z", startPos) + 1; // Assuming the time ends with "Z"
    String timeStr = jsonPayload.substring(startPos, endPos);

    // Find and extract ArrivalStatus
    statusPos = jsonPayload.lastIndexOf("\"ArrivalStatus\":", startPos) + 16;
    statusEndPos = jsonPayload.indexOf("\"", statusPos + 1);
    if (statusPos > 15 && statusEndPos != -1) {
      statusStr = jsonPayload.substring(statusPos + 1, statusEndPos);
      Serial.println("LE statut est");
      Serial.println(statusStr);
    }

    // Verify if ArrivalStatus is "onTime", "early", or "delayed"
    if (statusStr.equals("onTime") || statusStr.equals("early") || statusStr.equals("delayed")) {
      struct tm expectedTm = {0};
      strptime(timeStr.c_str(), "%Y-%m-%dT%H:%M:%SZ", &expectedTm);
      time_t expectedTime = mktime(&expectedTm);

      // Compare with the current time using timeinfo
      if (difftime(expectedTime, currentTime) > 0) { // If ExpectedArrivalTime is in the future
        result[resultIndex++] = expectedTime; // Store in the result array and move to the next index
      }
    }

    index = endPos; // Update index for the next search iteration
  }

  // If fewer than 2 future times were found, fill the rest of the result array with 0
  for (int i = resultIndex; i < 2; ++i) {
    result[i] = 0;
  }
}

void printTimeDelta(struct tm timeinfo, time_t last_api_call_times[2]) {
  unsigned long currentMillis = millis(); // Current time in milliseconds
  time_t times[2];
  if (currentMillis - lastApiCallMillis >= apiCallInterval) {
    printlcd("Nouvelle requete",0,1);
    
    lastApiCallMillis = currentMillis; // Update the last API call time

    // Fetch new data and process it
    String payload = fetchAndProcessApiData(LineRef, MonitoringRef);
    if (!payload.isEmpty()) {
      extractAndAdjustExpectedArrivalTimes(payload, times);
    }
    else {
      // Optionally, display an error message if the payload is empty
      printlcd("Payload Empty",0,1);
      delay(2000); // Show the message for 2 seconds
    }
  } else {
    times[0] = last_api_call_times[0];
    times[1] = last_api_call_times[1];
  }

  // Get current time again to ensure accuracy
  if (!getLocalTime(&timeinfo)) {
    printlcd("Failed to get time",0,1);
    return;
  }
  time_t now = mktime(&timeinfo);

  lcd.clear(); // Clear the display for the next message
  // Iterate over the arrival times and print the time differences on the LCD
  for (int i = 0 ; i < 2 ; ++i) {
    // Compute the difference
    double diff = difftime(times[i], now);
    int diffMinutes = diff / 60;
    int diffSeconds = int(diff) % 60;

    // Print the difference on the LCD
    lcd.setCursor(0, i); // Set the cursor to the beginning of the first line
    lcd.print(i + 1);
    lcd.print(" ");
    lcd.print(diffMinutes);
    lcd.print(" min ");
    lcd.print(diffSeconds);
    lcd.print(" sec");
    
  }
  last_api_call_times[0] = times[0];
  last_api_call_times[1] = times[1];
}

void printlcd(String message, int cursor, bool clear){
    if (clear){
      lcd.clear();
    }
    lcd.setCursor(0, cursor);
    lcd.print(message);
}

void setup() {
  Serial.begin(115200); // Initialize Serial
  lcd.begin(16, 2); // Initialize LCD
  pinMode(buttonPin, INPUT); // Config pin input


  //// WIFI PART ////
  connectToWiFi(); 
  authenticateToCaptivePortal(); 
  int attemptCount = 1;
  while (!testInternetConnection() && attemptCount < maxAttempts) { // TESTER LA CONNECTION
    delay(5000 * (++attemptCount)); // Exponential back-off
    connectToWiFi(); 
    authenticateToCaptivePortal(); 
  }

  // Initialize NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Wait until time is synchronized
  //Serial.println("Synchronizing time...");
  printlcd("Synchronizing time...",0,0);
  delay(2000); // Wait a bit to ensure NTP response
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    printlcd("Failed to obtain time",0,0);
    return;
  }
  Serial.print("Current time: ");
  Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");


  ///// SET lastApiCallTimes /////
  time_t times[2];
  String payload = fetchAndProcessApiData(LineRef, MonitoringRef);
  if (!payload.isEmpty()) {
    extractAndAdjustExpectedArrivalTimes(payload, times);
  }
  else {
    printlcd("Payload is Empty",0,0);
    //Serial.println("Payload is Empty");
  }
lastApiCallTimes[0] = times[0];
lastApiCallTimes[1] = times[1];
}

void loop() {
  // Non-blocking delay
  static unsigned long lastLoopMillis = 0;
  unsigned long currentMillis = millis();


  if (currentMillis - lastLoopMillis >= 1 * 1000) { // Update "front end" 1 every second
    lastLoopMillis = currentMillis;
    printTimeDelta(timeinfo, lastApiCallTimes); // This will now decide if it needs new API data or just print the time based on last fetch
  }

  bool reading = digitalRead(buttonPin);
  

    // If the button state changes, reset the debounce timer
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // If the state change has been stable long enough
  // that any electrical noise has likely subsided, perform the action
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the button is indeed pressed (state is HIGH because the button is connected to GND)
    if (reading == HIGH) {
      // New request
      lastApiCallMillis /= 10; // Adjust this line as needed for your functionality
    }
  }

  // Record the current state as the last known button state
  lastButtonState = reading;

}
