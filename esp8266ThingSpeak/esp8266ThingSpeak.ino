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
const char* ssid = "neo";
const char* password = "1122334455";
const char* ipThingSpeak = "184.106.153.149";
const char* apiKeyThingSpeak = "YHPV3O2YUMWRVGG3";
//field 1 = motorStatus
//field 2 = motorLead
//field 3 = topLead
//field 4 = bottomLead
//field 5 = motorRunDuration

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

    if (digitalRead(14) == LOW) {   //Button 0
      // Make sure it is pressed for some time
      buttonDebounce++;
      if( buttonDebounce > 30){
        buttonDebounce = 0;
        buttonAction = 1;
      }
    } else if (digitalRead(15) == LOW) {    //Button 1
      buttonDebounce++;
      if( buttonDebounce > 30){
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
			//https://api.thingspeak.com/channels/311318/fields/5.json?api_key=&results=2
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
					}else if(field5Val == "2"){
						//Stop motor
						relayOff();
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
      client.stop();
			} else{
			//check for timeout to skip to case 4
			if((millis()-timeOut) > 5000){
				clientState = 1;
        client.stop();
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
  pinMode(15, INPUT); //BUTTON 1
  pinMode(14, INPUT); // BUTTON 0  // For ON and OFF
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
  //Send data to thing speak
	  updateThingSpeak();
}
