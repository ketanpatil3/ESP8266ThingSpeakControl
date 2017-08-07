/*
To upload through terminal you can use: curl -F "image=@firmware.bin" esp8266-webupdate.local/update
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <LiquidCrystal_I2C.h>
#include <FS.h>
#include <NtpClientLib.h>
#include <TimeLib.h>
#include <EEPROM.h>
#define displayDelay 4

const char* host = "esp8266-webupdate";
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";
const char* ssid = "";
const char* password = "";
const char* ipThingSpeak = "184.106.153.149";
const char* apiKeyThingSpeak = "";
//field 1 = motorStatus
//field 2 = motorLead
//field 3 = topLead
//field 4 = bottomLead
//field 5 = motorRunDuration

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
  <meta charset="utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP8266 WATERCONTROLLER</title>
  </head>
  <style media="screen" type="text/css">
  html{font-family:sans-serif;-webkit-text-size-adjust:100%;-ms-text-size-adjust:100%}body{margin:0}header{display:block}a{background-color:transparent}a:active,a:hover{outline:0}b{font-weight:700}button{margin:0;font:inherit;color:inherit}button{overflow:visible}button{text-transform:none}button{-webkit-appearance:button;cursor:pointer}button::-moz-focus-inner{padding:0;border:0}@media print{*,:after,:before{color:#000!important;text-shadow:none!important;background:0 0!important;-webkit-box-shadow:none!important;box-shadow:none!important}a,a:visited{text-decoration:underline}a[href]:after{content:" (" attr(href) ")"}p{orphans:3;widows:3}.navbar{display:none}}@font-face{font-family:'Glyphicons Halflings';src:url(../fonts/glyphico.eot);src:url(../fonts/glyphico.eot?#iefix) format('embedded-opentype'),url(../fonts/glyphico.woff2) format('woff2'),url(../fonts/glyphico.woff) format('woff'),url(../fonts/glyphico.ttf) format('truetype'),url(../fonts/glyphico.svg#glyphicons_halflingsregular) format('svg')}*{-webkit-box-sizing:border-box;-moz-box-sizing:border-box;box-sizing:border-box}:after,:before{-webkit-box-sizing:border-box;-moz-box-sizing:border-box;box-sizing:border-box}html{font-size:10px;-webkit-tap-highlight-color:rgba(0,0,0,0)}body{font-family:"Helvetica Neue",Helvetica,Arial,sans-serif;font-size:14px;line-height:1.42857143;color:#333;background-color:#fff}button{font-family:inherit;font-size:inherit;line-height:inherit}a{color:#337ab7;text-decoration:none}a:focus,a:hover{color:#23527c;text-decoration:underline}a:focus{outline:thin dotted;outline:5px auto -webkit-focus-ring-color;outline-offset:-2px}p{margin:0 0 10px}.container{padding-right:15px;padding-left:15px;margin-right:auto;margin-left:auto}@media (min-width:768px){.container{width:750px}}@media (min-width:992px){.container{width:970px}}@media (min-width:1200px){.container{width:1170px}}.col-sm-1,.col-sm-2,.col-sm-4,.col-sm-6{position:relative;min-height:1px;padding-right:15px;padding-left:15px}@media (min-width:768px){.col-sm-1,.col-sm-2,.col-sm-4,.col-sm-6{float:left}.col-sm-6{width:50%}.col-sm-4{width:33.33333333%}.col-sm-2{width:16.66666667%}.col-sm-1{width:8.33333333%}.col-sm-offset-2{margin-left:16.66666667%}}label{display:inline-block;max-width:100%;margin-bottom:5px;font-weight:700}.form-group{margin-bottom:15px}.form-control-static{min-height:34px;padding-top:7px;padding-bottom:7px;margin-bottom:0}.form-horizontal .form-group{margin-right:-15px;margin-left:-15px}@media (min-width:768px){.form-horizontal .control-label{padding-top:7px;margin-bottom:0;text-align:right}}.btn{display:inline-block;padding:6px 12px;margin-bottom:0;font-size:14px;font-weight:400;line-height:1.42857143;text-align:center;white-space:nowrap;vertical-align:middle;-ms-touch-action:manipulation;touch-action:manipulation;cursor:pointer;-webkit-user-select:none;-moz-user-select:none;-ms-user-select:none;user-select:none;background-image:none;border:1px solid transparent;border-radius:4px}.btn:active:focus,.btn:focus{outline:thin dotted;outline:5px auto -webkit-focus-ring-color;outline-offset:-2px}.btn:focus,.btn:hover{color:#333;text-decoration:none}.btn:active{background-image:none;outline:0;-webkit-box-shadow:inset 0 3px 5px rgba(0,0,0,.125);box-shadow:inset 0 3px 5px rgba(0,0,0,.125)}.btn-default{color:#333;background-color:#fff;border-color:#ccc}.btn-default:focus{color:#333;background-color:#e6e6e6;border-color:#8c8c8c}.btn-default:hover{color:#333;background-color:#e6e6e6;border-color:#adadad}.btn-default:active{color:#333;background-color:#e6e6e6;border-color:#adadad}.btn-default:active:focus,.btn-default:active:hover{color:#333;background-color:#d4d4d4;border-color:#8c8c8c}.btn-default:active{background-image:none}.btn-group{position:relative;display:inline-block;vertical-align:middle}.btn-group>.btn{position:relative;float:left}.btn-group>.btn:active,.btn-group>.btn:focus,.btn-group>.btn:hover{z-index:2}.btn-group .btn+.btn{margin-left:-1px}.btn-group>.btn:first-child{margin-left:0}.btn-group>.btn:first-child:not(:last-child):not(.dropdown-toggle){border-top-right-radius:0;border-bottom-right-radius:0}.btn-group>.btn:last-child:not(:first-child){border-top-left-radius:0;border-bottom-left-radius:0}.navbar{position:relative;min-height:50px;margin-bottom:20px;border:1px solid transparent}@media (min-width:768px){.navbar{border-radius:4px}}@media (min-width:768px){.navbar-header{float:left}}.container>.navbar-header{margin-right:-15px;margin-left:-15px}@media (min-width:768px){.container>.navbar-header{margin-right:0;margin-left:0}}.navbar-static-top{z-index:1000;border-width:0 0 1px}@media (min-width:768px){.navbar-static-top{border-radius:0}}.navbar-brand{float:left;height:50px;padding:15px 15px;font-size:18px;line-height:20px}.navbar-brand:focus,.navbar-brand:hover{text-decoration:none}@media (min-width:768px){.navbar>.container .navbar-brand{margin-left:-15px}}.navbar-default{background-color:#f8f8f8;border-color:#e7e7e7}.navbar-default .navbar-brand{color:#777}.navbar-default .navbar-brand:focus,.navbar-default .navbar-brand:hover{color:#5e5e5e;background-color:transparent}.container:after,.container:before,.form-horizontal .form-group:after,.form-horizontal .form-group:before,.navbar-header:after,.navbar-header:before,.navbar:after,.navbar:before{display:table;content:" "}.container:after,.form-horizontal .form-group:after,.navbar-header:after,.navbar:after{clear:both}@-ms-viewport{width:device-width}
  .topnav {
    background-color: #333;
    overflow: hidden;
  }
  .topnav a {
    float: left;
    display: block;
    color: #f2f2f2;
    text-align: center;
    padding: 14px 16px;
    text-decoration: none;
    font-size: 17px;
  }
  .topnav a:hover {
    background-color: #ddd;
    color: black;
  }
  .topnav a.active {
    background-color: #4CAF50;
    color: white;
  }
  </style>
  <script>
  $(document).ready( function() {
    getAll();
  });
  function setPower(value) {
    $.post(urlBase + "power?value=" + value, function(data) {
      updatePowerButtons(data);
      $("#status").html("Set Power: " + data);
    });
  }
  function updatePowerButtons(value) {
    if(value == 0) {
      $("#btnPowerOn").attr("class", "btn btn-default");
      $("#btnPowerOff").attr("class", "btn btn-primary");
    } else {
      $("#btnPowerOn").attr("class", "btn btn-primary");
      $("#btnPowerOff").attr("class", "btn btn-default");
    }
  }
  $("#btnRefresh").click(function() {
    getAll();
  });
  $("#btnPowerOn").click(function() {
    setPower(1);
  });

  $("#btnPowerOff").click(function() {
    setPower(0);
  });
  function getAll() {
    var urlBase = window.location.hostname;
    $.get(urlBase + "all", function(data) {
      allData = data;

      $("#status").html("Connecting...");
      updatePowerButtons(data.power);
      $("#status").html("Ready");
    });
  }
  function setPower(value) {
    $.post(urlBase + "power?value=" + value, function(data) {
      updatePowerButtons(data);
      $("#status").html("Set Power: " + data);
    });
  }

  </script>
  <body>
  <header class="navbar navbar-default navbar-static-top" id="top" role="banner">
  <div class="container">
  <div class="navbar-header">
  <a class="navbar-brand" href="/">ESP8266 WATERCONTROLLER</a>
  </div>
  </div>
  </header>
  <div class="container">
  <form class="form-horizontal">
  <div class="form-group">
  <div class="col-sm-1 col-sm-offset-2">
  <button type="button" class="btn btn-default">
  <span id="btnRefresh"><b class>&#10226;</b></span>
  </button>
  </div>
  <div class="col-sm-4">
  <p id="status" class="form-control-static">Status</p>
  </div>
  </div>
  <div class="form-group">
  <label class="col-sm-2 control-label">Power</label>
  <div class="col-sm-6">
  <div class="btn-group" role="group" aria-label="Power">
  <button type="button" class="btn btn-default" id="btnPowerOn">On</button>
  <button type="button" class="btn btn-default" id="btnPowerOff">Off</button>
  </div>
  </div>
  </div>
  <div class="form-group">
  <label for="inputBrightness" class="col-sm-2 control-label">Last Tank Filled Timestamp</label>
  </div>
  <div class="form-group">
  <label for="inputBrightness" class="col-sm-2 control-label">Last Refresh</label>
  </div>
  </form>
  </div>
  </body>
  </html>
)rawliteral";

WiFiClient client;
byte buttonAction = 0;
byte FirstTimeDisplay = 0, createStringCase = 0;
char bufferLCD2[40];
byte bufferLCD2Len = 0;
#define SERIAL_VERBOSE
LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
bool timeIPToggle = false;
uint8_t power = 1;
unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 2000;           // interval at which to blink (milliseconds)
unsigned long counter = 0;
byte topLead = 0;
byte bottomLead = 0;
byte motorLead = 0;
byte motorState = 0;
byte motorOnTimeDisplayed = 0;
int sampleTimeCounter = 0;
//bool createString = false;
int storedMinute = 0;
unsigned long ntcTime = 0, motorStartTime = 0, leadsCheckTime = 0, secondLineDisplay = 0, firstLineLcdMil = 0, diplayRefreshMil = 0, thingSpeakRefresh = 0;
boolean syncEventTriggered = false;
time_t getNtpTime();

String getAllDate() {
  byte readValue;
  String sendStr = "";
  int i;
  for (i = 1; i < 42; i++) {
    readValue = EEPROM.read(i);
    sendStr = String(readValue, HEX) + sendStr;
  }
  return sendStr;
}

void sendAll()
{
  String json = "{";
  json += "\"power\":" + String(power);// + ",";
  json += "\"payload\":" + getAllDate();
  json += "}";
  httpServer.send(200, "text/json", json);
  json = String();
}
void handleRoot()
{
  httpServer.send_P(200, "text/html", INDEX_HTML);
}


void sendPower()
{
  String json = String(power);
  httpServer.send(200, "text/json", json);
  json = String();
}
void setPower(uint8_t value)
{
  power = value == 0 ? 0 : 1;
}
void addDate(unsigned long dateTime, byte onTimeMin) {
  byte pointer = EEPROM.read(0x00);
  byte DateMSB = EEPROM.read(0x01);
  if (pointer > 0x0A) {
    //wrong date reset all
    //total length 2+3*10+10 = 42bytes    //<counter><Date3><date0><date1><date2><date0><date1><date2><date0><date1><date2><date0><date1><date2>
    pointer = 0;
  }
  //Split date unsigned long valye = 0x12345678;
  byte arr[4];
  arr[0] = dateTime & 0xFF;         // 0x78   //date0
  arr[1] = (dateTime >> 8) & 0xFF;  // 0x56   //date1
  arr[2] = (dateTime >> 16) & 0xFF; // 0x34  //date2
  arr[3] = (dateTime >> 24) & 0xFF; // 0x12  //date3
  EEPROM.write(0x01, arr[3]);
  EEPROM.write(pointer * 3 + 0 + 2, arr[0]);
  EEPROM.write(pointer * 3 + 1 + 2, arr[1]);
  EEPROM.write(pointer * 3 + 2 + 2, arr[2]);
  EEPROM.write(pointer + 32, onTimeMin);
  EEPROM.write(0, pointer++); // EEPROM.write(addr, val);
  EEPROM.commit();
}
void relayOff() {
  motorState = 0;
  lcd.backlight(); // Relay off
  //Store the time in EEPROM
  addDate(motorStartTime,(byte)(millis() - motorStartTime)/60000);
}
void relayON() {
  motorState = 1;
  motorStartTime = millis();
  lcd.noBacklight(); // Relay on
}
unsigned long getLastTankFill() {
  byte pointer = EEPROM.read(0x00);
  return (EEPROM.read(0x01) << 24) | (EEPROM.read(pointer * 3 + 2 + 2) << 16) | (EEPROM.read(pointer * 3 + 1 + 2) << 8) | EEPROM.read(pointer * 3 + 0 + 2);
}
void displayLcd(char *mem, byte row) {
  char localChar[17];
  localChar[16] = '\0';
  memcpy(localChar, mem, 16);
  lcd.setCursor(0, row);
  lcd.printstr(localChar);
}
void itoa1(char *bufferLCD, int value, byte position) {
  if (value > 9) {
    itoa(value, &bufferLCD[position], 10);
  } else {
    bufferLCD[position] = '0';
    itoa(value, &bufferLCD[position + 1], 10);
  }
}
void createString(byte createStringCase) {
  //Create String for display
  switch (createStringCase) {
    case 1: {
      //Last Tank Filled 28/05/2017 12:12
      unsigned long tempMillis = getLastTankFill();
      memcpy(bufferLCD2, (char *)"Last Tank Fill 00/00/00 00:00 MOTOR OFF", 39);
      itoa1(bufferLCD2,day(tempMillis), 15);
      itoa1(bufferLCD2,month(tempMillis), 18);
      itoa1(bufferLCD2,year(tempMillis) % 1000, 21);
      itoa1(bufferLCD2,hour(tempMillis), 24);
      itoa1(bufferLCD2,minute(tempMillis), 27);
      bufferLCD2Len = 40;
      //itoa adds '\0' at the end the below operation will remove it
      bufferLCD2[17] = '/';
      bufferLCD2[20] = '/';
      bufferLCD2[23] = ' ';
      bufferLCD2[26] = ':';
      bufferLCD2[29] = ' ';
      break;
    }
    case 2: {
      //MOTOR ON 000Mins
      memcpy(bufferLCD2, (char *)"MOTOR ON for 00 mins", 20);
      itoa1(bufferLCD2,motorOnTimeDisplayed, 14);
      bufferLCD2Len = 21;
      bufferLCD2[15] = ' ';
      bufferLCD2[16] = 'm';
      break;
    }
    case 3: {
      //MOTOR DRY RUN ERROR
      memcpy(bufferLCD2, (char *)"MOTOR DRY RUN ERROR\0",  19);
      bufferLCD2Len = 20;
      break;
    }
    case 4: {
      //TRY AFTER SOME TIME
      memcpy(bufferLCD2, (char *)"TRY AFTER SOME TIME\0",  19);
      bufferLCD2Len = 20;
      break;
    }
    default: {
      //control refreshes
    }
  }
}
void reverse(char str[], int length)
{
  int start = 0;
  int end = length -1;
  while (start < end)
  {
    swap(*(str+start), *(str+end));
    start++;
    end--;
  }
}


void checkLeads() {
  unsigned int i;
  static unsigned int j = 0, k = 0, l = 0, counter = 0;
  static unsigned long LocalcheckLeadTime = 0, LocalcheckCountTime = 0;
  const int sampleTime = 3000; //2000mSec
  const int leadSampleTime = 20; //2000mSec
  if ((millis() - LocalcheckLeadTime) > leadSampleTime) {
    counter++;
    if (digitalRead(12) == HIGH)
    j++;
    if (digitalRead(10) == HIGH)
    k++;
    if (digitalRead(9) == HIGH)
    l++;
    LocalcheckLeadTime = millis();
  }
  if ((millis() - LocalcheckCountTime) > sampleTime) {
    if ((( j * 100) / counter) > 90){
      topLead = 1; // Water touching the lead
    }
    else{
      topLead = 0; // Water not touching the lead
    }

    if ((( k * 100) / counter) > 90){
      bottomLead = 1; //Water touching the lead
    }
    else{
      bottomLead = 0;  //Water not touching the lead
    }

    if ((( l * 100) / counter) > 50){
      motorLead = 1; //Water touching the lead
    }
    else{
      motorLead = 0;  //Water not touching the lead
    }
    j = 0;
    k = 0;
    l = 0;
    counter = 0;
    LocalcheckCountTime = millis();
  }
}
void keyCheck() {
  static unsigned long buttonActionTime = 0;
  static byte buttonDebounce = 0;
  int buttonCheckInternalMillis = 100;
  if (buttonAction == 0 && ((millis() - buttonActionTime) > buttonCheckInternalMillis)) {
    if (digitalRead(12) == HIGH) {
      // Make sure it is pressed for some time
      buttonDebounce++;
      if( buttonDebounce > (buttonCheckInternalMillis*30)){
        buttonDebounce = 0;
        buttonAction = 1;
      }
    } else if (digitalRead(13) == HIGH) {
      buttonDebounce++;
      if( buttonDebounce > (buttonCheckInternalMillis*30)){
        buttonDebounce = 0;
        buttonAction = 2;
      }
    }
    buttonActionTime = millis();
  }
}

void updateThingSpeak(){
	static unsigned long timeOut=0;
	static byte clientState=1, motorCommandReceived;
	switch(clientState){
		case 1:{   //Init connection
		if((millis() - thingSpeakRefresh)> (1000*60*5)){
				if (!client.connect(ipThingSpeak, 80)) {
				  thingSpeakRefresh += 1000; // Wait for 1 second
				  return;
				}else{
					clientState = 2; //jump to next case
				}
			}
		}
		case 2:{  // fire a call to read relay command
			//Get command status
			//https://api.thingspeak.com/channels/311318/fields/5.json?api_key=3INANMRD24CAG09L&results=2
			client.print(String("GET /channels/311318/fields/5.json?api_key=") + String(apiKeyThingSpeak)+ "&results=2  HTTP/1.1\r\n" +
					   "Host: " + String(ipThingSpeak) + "\r\n" +  "Connection: keep-alive\r\n\r\n");
			clientState = 3;
			timeOut = millis();
			break;
		}
		case 3:{ //wait for data till timeout
		/*Sample response: {"channel":{"id":311318,"name":"watercontroller","description":"Water controller based on ESP8266","latitude":"0.0","longitude":"0.0","field1":"motorstatus","field2":"motorLead","field3":"topLead","field4":"bottonLead","field5":"motorCommand","created_at":"2017-08-02T11:55:17Z","updated_at":"2017-08-02T12:12:41Z","last_entry_id":1},"feeds":[{"created_at":"2017-08-02T11:57:54Z","entry_id":1,"field5":null}]} */
			if(client.available()){
				String strData = client.readStringUntil('\n');
				 if(strData != "") {
					int dataIndex = strData.lastIndexOf("field5");
					if(dataIndex == -1){
						//Field5 not found. Move to next section
						clientState = 4;
						break;
					}
					motorCommandReceived = 0;
					String field5Val = strData.substring(dataIndex+9,strData.length()-4);
					if (field5Val == "1" ){
					//Start motor
						relayON();
						motorCommandReceived = 1;
					}
				 }

			} else{
			//check for timeout to skip to case 4
			if((millis()-timeOut) > 5000){
				clientState = 4;
				break;
				}
			}
		}
		case 4:{
			String api = "&field1=";
			api += String(motorState);
			api += "&field2="+String(motorLead);
			api += "&field3="+String(topLead);
			api += "&field4="+String(bottomLead);
			if(motorCommandReceived == 1){
				motorCommandReceived = 0;
				api += "&field5=0";
			}
			client.print(String("GET /update?key=") + String(apiKeyThingSpeak)+api+ " HTTP/1.1\r\n" +
					   "Host: " + String(ipThingSpeak) + "\r\n" +
					   "Connection: keep-alive\r\n\r\n");
			clientState = 5;
			timeOut = millis();
		    break;
		}
		case 5:{
		if(client.available()){
			clientState = 1;
			} else{
			//check for timeout to skip to case 4
			if((millis()-timeOut) > 5000){
				clientState = 1;
				break;
				}
			}
		}
	}
}
void updateThingSpeakNow(unsigned long currentmillis){
	thingSpeakRefresh = currentmillis - 1000*60*5;
}
void setup(void) {
  #ifdef SERIAL_VERBOSE
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting...");
  #endif
  SPIFFS.begin();
  #ifdef SERIAL_VERBOSE
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    yield();
    String fileName = dir.fileName();
    size_t fileSize = dir.fileSize();
    Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), String(fileSize).c_str());
  }
  Serial.printf("\n");
  #endif
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // ... Give ESP 10 seconds to connect to station.
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    #ifdef SERIAL_VERBOSE
    Serial.write('.');
    //Serial.print(WiFi.status());
    #endif

    delay(500);
    yield();
  }
  if (WiFi.status() == WL_CONNECTED) {
    #ifdef SERIAL_VERBOSE
    // ... print IP Address
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    #endif
  }
  else {
    #ifdef SERIAL_VERBOSE
    Serial.println("Can not connect to WiFi station. Go into AP mode.");
    #endif
    // Go into software AP mode.
    WiFi.mode(WIFI_AP);

    delay(10);

    WiFi.softAP(ssid, password);
    #ifdef SERIAL_VERBOSE
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
    #endif
  }
  MDNS.begin(host);

  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  //Server related code
  //httpServer.serveStatic("/", SPIFFS, "/index.html"); // ,"max-age=86400"
  // httpServer.serveStatic("/index.html", SPIFFS, "/index.html");
  httpServer.on("/", handleRoot);
  httpServer.on("/all", HTTP_GET, []() {
    sendAll();
  });
  httpServer.on("/power", HTTP_POST, []() {
    String value = httpServer.arg("value");
    setPower(value.toInt());
    sendPower();
  });
  httpServer.on("/power", HTTP_GET, []() {
    sendPower();
  });
  //Server related code Should be before begin
  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
  #ifdef SERIAL_VERBOSE
  Serial.printf("HTTPUpdateServer ready! Open http://");
  Serial.print(WiFi.localIP());
  Serial.printf("%s in your browser and login with username '%s' and password '%s'\n", update_path, update_username, update_password);
  #endif

  //LCD test code
  relayOff(); // Relay off
  lcd.init();
  //Start NTP
  NTP.begin("in.pool.ntp.org", 0, false);
  //EEPROM
  EEPROM.begin(64);
  //test
  addDate(0x592550A8, 1);
  addDate(0x592650A8, 2);
  addDate(0x592750A8, 3);
  addDate(0x592850A8, 4);
  addDate(0x592950A8, 5);
  addDate(0x593050A8, 6);
  addDate(0x593150A8, 7);
  addDate(0x593250A8, 8);
  addDate(0x593350A8, 9);
  addDate(0x593450A8, 10);
  addDate(0x593550A8, 11);
  //PIN
  pinMode(12, INPUT); //TOP LEAD
  pinMode(10, INPUT); //BOTTOM LEAD
  pinMode(9, INPUT); // MOTOR LEAD
  pinMode(0, INPUT); //BUTTON 0
  pinMode(0, INPUT); // BUTTON 1
}
void loop(void) {
  char bufferLCD1[16];
  IPAddress ip;
  unsigned long currentMillis = millis();
  byte motorOnTime = 0;
  httpServer.handleClient();
  yield();

  //first line LCD code
  if (currentMillis - firstLineLcdMil > 10000) {
    //Store and display time
    firstLineLcdMil = currentMillis;
    if (timeIPToggle) {
      time_t currentTime = now() + 5 * 3600 + 1800; //Converting to IST (5*60*60+30*60)
      //memcpy(bufferLCD1, (char *)"00/00/0000 00:00", 16);
      itoa1(bufferLCD1,day(currentTime), 0);
      itoa1(bufferLCD1,month(currentTime), 3);
      itoa(year(currentTime) , &bufferLCD1[6], 10);
      itoa1(bufferLCD1,hour(currentTime), 11);
      itoa1(bufferLCD1,minute(currentTime), 14);
      //remove '\0' added by itoa operation
      bufferLCD1[2] = '/';
      bufferLCD1[5] = '/';
      bufferLCD1[10] = ' ';
      bufferLCD1[13] = ':';
    }
    else {
      if (WiFi.status() == WL_CONNECTED) {
        ip = WiFi.localIP();
        memcpy(bufferLCD1, (char *)"IP:", 3);
        int stpPoint = 3;
        for (int i = 0; i < 4; i++) {
          int num = ip[i];
          int j = stpPoint;
          while (num != 0) {
            int rem = num % 10;
            bufferLCD1[j++] = rem + '0'; //For changing base add (rem > 9)? (rem-10) + 'a' : rem + '0';
            num = num / 10;
          }
          reverse(&bufferLCD1[stpPoint],(j-stpPoint));
          stpPoint = j;
          if(stpPoint>15){
            //Make sure no memory leaks
            break;
          }
          bufferLCD1[stpPoint++] = ':';
        }
      }
    }
    timeIPToggle = !timeIPToggle;
    displayLcd(bufferLCD1, 0);
    #ifdef SERIAL_VERBOSE
    Serial.println(bufferLCD1);
    #endif
  }


  //Second line LCD Display
  if (currentMillis - secondLineDisplay > 600) {
    secondLineDisplay = currentMillis;
    if (FirstTimeDisplay == 0) {
      displayLcd(bufferLCD2, 1);
      FirstTimeDisplay++;
    }
    else if (FirstTimeDisplay > displayDelay ) {
      if (FirstTimeDisplay - displayDelay + 16 < bufferLCD2Len) {
        displayLcd(&bufferLCD2[(FirstTimeDisplay - displayDelay)], 1);
        FirstTimeDisplay++;
      } else {
        FirstTimeDisplay = 0;
      }
    }
    else {
      FirstTimeDisplay++;
    }
  }
  //Display refresh
  if (currentMillis - diplayRefreshMil > 2000) {
    diplayRefreshMil = currentMillis;
    createString(createStringCase);
    #ifdef SERIAL_VERBOSE
    Serial.println(bufferLCD2);
    #endif
  }
  //Core logic
  checkLeads();
  if (motorState == 1){
    if (motorLead == 0) {
      //Dry Run Case
      if ((currentMillis - motorStartTime) > 100000) {
        //min motor on time to stop spikes
        relayOff();
        createStringCase = 3;
      }
    } else if(topLead == 1){
      if ((currentMillis - motorStartTime) > 10000 ) {
        relayOff();
        createStringCase = 1;
		    updateThingSpeakNow(currentMillis);
      }
    }
  } else if( motorState == 0 ) {
    if (bottomLead == 0) {
      //Start motor tank empty
      relayON();
      createStringCase = 2;
	    updateThingSpeakNow(currentMillis);
    }
  }
  //Key check
  keyCheck();
  if(buttonAction == 1){
    buttonAction = 0;
    if(motorState == 0){
		if ((currentMillis - motorStartTime) > 100000) {
			relayON();
			createStringCase = 2;
			updateThingSpeakNow(currentMillis);
		}else{
			createStringCase = 4;
		}
    } else {
      relayOff();
      createStringCase = 1;
      updateThingSpeakNow(currentMillis);
    }
  } else if(buttonAction == 2){
    if(createStringCase == 3){ //clear dryrun error
      createStringCase = 1;
    }
  }
  //Reset display try after sometime to normal one
  if(createStringCase == 4){
	  if ((currentMillis - motorStartTime) > 100000){
		  createStringCase = 1;
	  }
  }
  //Send data to ting speak
	  updateThingSpeak();
}
