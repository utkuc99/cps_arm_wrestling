//CS350 CPS Arm Wrestling Project

//libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"

//wifi
const char* ssid     = "ESP8266-Access-Point";
const char* password = "123456789";

//score
int t = 50;

//buttonValues
bool prev = true;
bool now = true;
bool prev2 = false;
bool now2 = false;

//user score
int score1 = 1;
int score2 = 1;

//millisecond difference
int mildif = 0;
int startMillis = 0;

//state
int started = 0;
int finished = 0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

//html
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>CS350 Arm Wrestling</h2>
  <p>
    <span class="dht-labels">Player 1</span> 
    <span id="temperature"></span>
  </p><p>
  <img src="arm"></br>
  <progress id="file" value="32" max="100"> 32% </progress>
  </p><p>
    <span class="dht-labels">Player 2</span>
    <span id="humidity"></span>
  </p>
  <p>
    <span class="dht-labels">time</span>
    <span id="time"></span>
  </p>
</body>
<script>
 
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
      document.getElementById("humidity").innerHTML = 100 - this.responseText;
      document.getElementById("file").value = 100 - this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 100 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("time").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/time", true);
  xhttp.send();
}, 100 ) ;

</script>
</html>)rawliteral";

// Replaces placeholder with values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(t);
  }
  return String();
}

void setup(){
  // Serial port
  Serial.begin(115200);
  
  Serial.print("Setting AP (Access Point)…");
  WiFi.softAP(ssid, password);

  if(!SPIFFS.begin()){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
  }

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  //Route for score
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  //Route for time
  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(mildif).c_str());
  });
  //Route for photo
   server.on("/arm", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/arm.jpg", "image/jpg");
  });

  // Start server
  server.begin();
}
 
void loop(){  
  delay(10);
  
  //Serial.print(digitalRead(D7));
  
  if((digitalRead(D3) == 0) && started == 0){
    startMillis = millis();
    started = 1;
  }
  
  if(started == 1 && finished == 0){
    now = digitalRead(D4);
    now2 = digitalRead(D0);
    if(prev != now){
      score1++;
    }
    if(prev2 != now2){
      score2++;
    }
    prev = digitalRead(D4);
    prev2= digitalRead(D0);

    int percentage = 100*score1/(score1 + score2);
    t = percentage;
  }

  if(started == 0){
    mildif = 60;
  }else{
    if(finished == 1){
      mildif = 0;
    }else{
      mildif = (60000-(millis()-startMillis))/1000;
    }
  }

  if(mildif == 0){
    finished = 1;
  }
  
}
