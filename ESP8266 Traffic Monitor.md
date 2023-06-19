
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
- Red LED:          D6 (12)
- Yellow LED:      D1 (5)
- Green LED:       D2 (4)
- Button:             D5 (14)
- OLED SDA:       D4 (2)
- OLED SCL:        D3 (0)

One side of the button is connected to ground and the other side is connected to the ESP pin and VCC through a 1k resistor. This configures it as an active low button, so it will read as a '1' when not pressed and a '0' when pressed. The button could be used to refresh the display or turn on the OLED or something.

The LEDs are connected to ground through a 330 resistor to limit current. They are standard 5mm LEDs.

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



This code will connect the ESP to a wifi network and give you its IP address. Replace ssid and password with the correct credentials for the network

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