/*
By:- Unmesh Subedi
github :- https://github.com/Celestial071
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Base64.h>

const char* ssid = "Connect";            // Replace with your WiFi SSID
const char* password = "00000000";    // Replace with your WiFi password
const char* host = "api.twilio.com";
const int httpsPort = 443;

bool isWarning = false;
// Replace with your Twilio credentials
const char* accountSid = "Your Account SID";
const char* authToken = "Your auth token";

WiFiClientSecure client;
bool messageSent = false;
String message = "";
void setup() {
  Serial.begin(9600);
  delay(10);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  client.setInsecure(); // the ESP8266 does not validate the certificate chain of the server
}

void loop() {

  if(Serial.available()){
    String incomingData = Serial.readStringUntil('\n');
    Serial.println(incomingData);
    incomingData.trim();
    if(incomingData == "-1") isWarning = true;
    if(!isWarning){
    int commaIndex = incomingData.indexOf(',');
    
    String phoneNumber = incomingData.substring(0, commaIndex);
    String token = incomingData.substring(commaIndex+1);
    Serial.println(phoneNumber);
    Serial.println(token);
    message = "The token generated for "+ phoneNumber+ " is: " + token;
    if (sendSms("ReceiversNumberwithCountryCode","SendersNumberwithCountrycode" , message)) { //<---include the numbers as required senders num --> usually bought in twilio and receiver's happens to be your or a verified number
    Serial.println("SMS Sent Successfully");
    //messageSent = true;
  } else {
    Serial.println("Error sending SMS or Invalid input Format");
  }
  }else{
    message = "We detected Suspicious activity in your Parking Stall\nContacting Local Authority";
    if(sendSms("ReceiversNumwithcountrycode", "SendersNumwithCountryCode", message)){
      Serial.println("Warning Message is Sent");
    }
  }

  }
  isWarning = false; // Wait for 10 seconds before sending another SMS
  delay(10000);
}

bool sendSms(String to, String from, String message) {
  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed");
    return false;
  }

  String url = String("/2010-04-01/Accounts/") + accountSid + "/Messages.json";

  String postData = "To=" + to + "&From=" + from + "&Body=" + message;
  String authHeader = "Basic " + base64::encode(String(accountSid) + ":" + String(authToken));


  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Authorization: " + authHeader + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded\r\n" +
               "Content-Length: " + postData.length() + "\r\n\r\n" +
               postData + "\r\n");

  int timeout = millis() + 5000;
  while (client.available() == 0) {
    if (millis() > timeout) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return false;
    }
  }

  // Read all the lines of the reply and print them to Serial (for debugging purposes)
  while (client.available()) {
    String line = client.readStringUntil('\r');
    //Serial.print(line);
  }
  return true;
}
