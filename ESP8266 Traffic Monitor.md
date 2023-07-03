
ESP8266 device with a small OLED to show the traffic time in minutes from home to work (build two to keep one at work) using google maps api. Simple interface, maybe a single button to retry connection or something but otherwise only show a single number on the screen. Some LEDs on the top could give a quick idea of how bad or good the travel time is based off an average or something.

Using a small LCD may be better than an OLED because OLEDs are susceptible to burn in, where LCDs arent. The LCD backlight could just be turned off, its likely that it wont be needed. The advantage to the OLEDs is their size (small) and the fact there are many on-hand right now (6-18-23). 

A workaround could be to add a button to see the exact time, while the display is off at idle. The LEDs would always be lit to give an idea of the travel time. Another option could be a bunch of LEDs 

Mockup of what it would look like in Fusion360
![[Pasted image 20230618105211.png]]

![[Pasted image 20230618113051.png]]

## Setting up ESP8266
Go to Arduino IDE preferences and enter the following URL for the additional boards manager:
`http://arduino.esp8266.com/stable/package_esp8266com_index.json`
This allows you to go to the boards manager and search for 'ESP' and install the boards. Without this you cannot select the ESP board to compile to.

If using the module as seen above, select 'Generic ESP8266' in the board menu after downloading the boards. 
![[Pasted image 20230618114452.png]]

Dont change these settings, they should work just fine:
![[Pasted image 20230618114518.png]]

Now select your COM port and it should work fine. Uploading code should be automatic, and you shouldn't need to press any buttons or anything.
![[Pasted image 20230618114548.png]]

Note the onboard LED is active low, not active high

Note the difference between PCB pin labels and Arduino IO numbers

## OLED Display

Using the OLED with the ACROBOTIC_SSD1306 library did not work out of the box. Compilation error for "control reached end of non void function." Go into the cpp file for this library, in Documents>Arduino>libraries>ACROBOTIC_SSD1306 and find the putChar function. Its return type is a bool, and I did not want to change that (youd have to change it in a bunch of places) so just add a `return true;` to the end of the function and it works then.

## Hardware

PCB pin label (arduino IDE number)
- OLED SCL:      D1     (5)
- OLED SDA:     D2     (4)
- Button:            D3    (0)
- Green LED:     D7      (13)
- Yellow LED:     D5     (14)
- Red LED:          D6     (12)


One side of the button is connected to ground and the other side is connected to the ESP pin and VCC through a 1k resistor. This configures it as an active low button, so it will read as a '1' when not pressed and a '0' when pressed. The button could be used to refresh the display or turn on the OLED or something.

The LEDs are connected to ground through a 330 resistor to limit current. They are standard 5mm LEDs.

## Watchdog

The ESP has a watchdog timer that must be fed. If you have a loop that goes for more than about a second and does not include a delay statement, call yield(). This will feed the watchdog.

## Hardware test code

Using the hardware definitions above. This flashes all LEDs for one second, and then loops. Loop is turning on the green LED only if the button is pushed. This also shows up on the display to prove everything is working correctly.

```
#include <Wire.h>
#include <ACROBOTIC_SSD1306.h>

#define SDA 2
#define SCL 0
#define BUTTON 14
#define GRN_LED 4
#define YLW_LED 5
#define RED_LED 12

void setup() {
  Wire.begin(SDA, SCL);
  oled.init();
  oled.clearDisplay();
  oled.setTextXY(0, 0);
  oled.putString("Hello World!");
  oled.setTextXY(1,0);          // row 1 col 0
  oled.putString("Button Pushed?");
  oled.setTextXY(2,0);
  oled.putString("No");
  
  pinMode(BUTTON, INPUT);
  pinMode(GRN_LED, OUTPUT);
  pinMode(YLW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  digitalWrite(GRN_LED, HIGH);
  digitalWrite(YLW_LED, HIGH);
  digitalWrite(RED_LED, HIGH);
  delay(1000);
  digitalWrite(GRN_LED, LOW);
  digitalWrite(YLW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  delay(1000);  
}

void loop() {
  if(digitalRead(BUTTON) == LOW) {
    // If the button is pressed, turn on the green LED
    digitalWrite(GRN_LED, HIGH);
    oled.setTextXY(2,0);
    oled.putString("Yes");
  } else {
    digitalWrite(GRN_LED, LOW);
    oled.setTextXY(2,0);
    oled.putString("No ");
  }
}
```

## Network Connection

It will be necessary to connect the ESP to the internet to access the Google Maps API. Another API may be used but either way we will need internet.

https://tttapa.github.io/ESP8266/Chap07%20-%20Wi-Fi%20Connections.html

More library info here: https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html
https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi
https://www.arduino.cc/reference/en/libraries/wifi/



This code will connect the ESP to a wifi network and give you its IP address. Replace ssid and password with the correct credentials for the network. See code files in repository.

```
#include <ESP8266WiFi.h>        // Include the Wi-Fi library

const char* ssid     = "SSID";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "PASS";     // The password of the Wi-Fi network

void setup() {
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');
  
  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
}

void loop() { }
```

## Online API Integration

Here is a github repo for someone who is working on a library for this exact thing. Seems more complicated than expected to integrate a device like the ESP to a google API.
https://github.com/cotestatnt/Arduino-Google-API

Someone has done basically the exact same thing as this project:
https://www.instructables.com/Arduino-Traffic-Display-Using-Google-Maps-API/
https://github.com/witnessmenow/arduino-traffic-notifier

This gives me an idea to use Twilio to configure the ESP instead of a Telegram bot as he used. Look to see if twilio is easy to use with an ESP8266
https://www.twilio.com/docs/sms/tutorials/how-to-send-sms-messages-esp8266-cpp
https://github.com/TwilioDevEd/twilio_esp8266_arduino_example

## USB Configuration

The device can be configured by using a serial terminal, such as the Arduino IDE terminal or PuTTY. The configuration should be stored in EEPROM when changed so that the device remembers its settings.

The settings that can be changed are:
1. Start location
2. End location
3. Network credentials
4. How the display behaves
	1. An idea is the display will only turn on for an hour or so a day at a specific time, such as before work or near the end of work. This reduces burn in while making the device fully hands-free during that time period. The config here would set the time that it does this. Time would be gathered from the internet somewhere.

The device will not communicate with a serial monitor unless the serial monitor sends something first. In the program loop, it will check for serial data every cycle, but not waiting for it unless there was actual data ready. Sending the device a byte will trigger it and it will respond with a list of options. Sending a command in the right format will change the device settings

Example:
```
OPTIONS:
1. CHANGE START LOCATION
2. CHANGE END LOCATION
3. CHANGE SSID
4. CHANGE PASSWORD
5. CHANGE DISPLAY
```

To change the ssid, send `SSID networkName` and it will change the network SSID it tries to connect to.

## Google APIs

A new project was created on the google API console. From there, go to library and search for the following APIs. These where what was enabled to make sure this project would work. Enable a maps API and it will walk you through how to characterize your use of the API. Then it gives the option to enable all maps APIs and thats what was done. The result was an API key. 

## Work: 40.560708, -105.028877
## Home: 40.155543, -105.048291

## Request JSON body:
```
{
  "origin": {
    "location": {
      "latLng": {
        "latitude": 40.155543,
        "longitude": -105.048291
      }
    }
  },
  "destination": {
    "location": {
      "latLng": {
        "latitude": 40.560708,
        "longitude": -105.028877
      }
    }
  },
  
}
```

## Results

So far its working correctly. Version 100 as seen in the file structure is complete but could use refactoring and optimization. Nevertheless, full functionality is present.

The device will power on and self test the display and the LEDs. Then, it will establish a WiFi connection and get the unix epoch, declaring the time that it was started.

Next, the device will begin to automatically poll the Google Maps Routes API, see the key in the developer console, and display the time on the display.

Once `PWR_ON_DISP_TIME` seconds have elapsed, the display will turn off to prolong the lifespan of the OLED display. To turn the display on, hold the button down until the display shows the traffic time. Otherwise, use the LEDs to get an idea of the commute.

The LEDs will always be lit, and as of version 100, will be green for times less than 40 minutes, yellow for times 40-50 minutes, and red for times >50 minutes. These give a quick glance at the commute time without needing to turn on the display.

When the display is on, the top left will show the device's IP address, the top right will have a clock, and the center screen is the commute time between the sets of coordinates given in the JSON request body.

The highest number version is the latest, starting at 100.

## Issues

There were many issues with parsing the JSON and it took a lot of trial and error to get it right. This program serves as a good example of building a POST request with headers and a JSON body, sending the POST request, and parsing a JSON response from an API. 

For the ESP8266, note that D1 for SCL and D2 for SDA are the defaults. This is what pins youll need to use if you use any build in I2C functionality. 

## Future Work

A case needs to be 3D printed so that its durable enough to sit on a desktop.

It would be nice if you could set the coordinates for origin and destination over serial, as well as other features like display timeout time. This makes it user friendly over having to reprogram it every time, which takes a while. Config would be stored in EEPROM.

Add a feature to turn the display on automatically during certain times of day. For example, the home device can turn on in the half hour of time around when you normally leave for work and the work device comes on automatically in the half hour around quitting time. This should also be configurable over serial.

The code needs refactored and could be cleaned up.