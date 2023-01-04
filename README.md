# LightBot | Telegram Bot on the ESP8265 | Ukraine issues
## It tells me whether I have light at home or not.

Telegram Bot on the ESP to know if you have light at home. Actual for Ukraine.     
So for the last 2 months I didn't have light for 12h almost every day. There supposed to be some schedule, that sometimes worked.    

![Here's what it looks like](https://github.com/dDenVil/LightBot/blob/main/Readme_assets/look.png?raw=true)

### Problem

Imagine you don't have light at home and you go out to spend some time until light is goes on. And you don't know when light will go on. You can call your family or neighbor if they are at home, but that is not my case.    
Previously a made a [Plant Life Project](https://github.com/dDenVil/Smart_Home_ESP-01_PlantLife) that notificates me with the message **"ONLINE"** if light at home goes on. But that was not enough for me.     

**In addition** of knowing when light is going on, I want to know:
- For how long I have light;
- For how long I didn't have light;
- Time of light goes off;
- To help other people to know is there light or not.

### Solution

I came up with idea of making telegram bot or how I called it **"Lightbot"**. **The principle** of work of this bot is simple.      
I made the UPS for router, **it will work only** if your WI-FI don't turn off immidiately with grid ( it needs a few seconds to send the message to telegram when light goes off)! Let's have a look at the picture below.

![concept](https://github.com/dDenVil/LightBot/blob/main/Readme_assets/Is_light_at_home.png)

Power supply will charge battety, to power ESP if light goes off. With help of optocoupler I could detect if there is power on power supply.

Also I have **the generator**, that we manualy are turning on. And I wanted to detect if light is on from the generator or the grid. The idea was to meassure the level of noise with help of microphone. There is no othe way to detect it, because the device will be installed inside the house, and there is no obvious voltage difference beetwen grid power and the generator power.      
But lately I abandoned this idea with the detection of the generator. Since we turn it on manually, I just will have to unplug the lightbot.

## How does it work

There is a channel that post next messages. If light is ON, it shows: 
```
🟢 There is light from 18:27. 
There was no light for 2 h. 12 min.
 
💡 17h.57min.49sec.   
```
Third row "💡" updates every 10 seconds to know for how long do I have light. If light is OFF, it sends message and goes into light sleep mode.
```
🔴 There is no light from 12:26. 
There was light for 17 h. 57 m.
```
[How it looks in the telegram](https://github.com/dDenVil/LightBot/blob/main/Readme_assets/telegram.png?raw=true)

### Scheme 

Yep, there is no sense of using optocoupler, because they have common ground, but I decided to do this universal. With light sleep mode it can work for months without charging (which will never happen).

![scheme](https://github.com/dDenVil/LightBot/blob/main/Readme_assets/sheme.png)

I recommend you to add [flash and reset buttons](https://github.com/dDenVil/LightBot/blob/main/Readme_assets/buttons.png). This will make your life easier.

### Uploading the code

- Create telergam bot
  - Search for [@BotFather](https://t.me/BotFather)
  - /start -> /newbot -> .... -> copy  the HTTP API
  - create the channel 
- Get your channel Telegram ID from [@username_to_id_bot](https://t.me/username_to_id_bot)
- Add the bot as admin to the channel
- Change the [code](https://github.com/dDenVil/LightBot/blob/main/Code.txt)
```c
// Wifi network station credentials
#define WIFI_SSID "SSID"   
#define WIFI_PASSWORD "PASS"
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "1234567:Bot_TokeN"
String chat_id="CHANNEL_ID";
int timezone = 2  ;
```
- Load the code to ESP:
    - Connect to TTL-conventer;
    - In Arduino IDE go to references and add additional board manager URL `http://arduino.esp8266.com/stable/package_esp8266com_index.json`;
    - Add board to the Arduino IDE (Tools->Board->Board Manager->ESP8266);
    - Choose correct board and Port (Tools->Board->ESP8266->Generic ESP8266 Module);
    - Install Telegram Bot Library (Sketch->Include library->Add .ZIP library) and other WiFi libraries;
    - Start the ESP in flash mode (short GPIO0 and GND) and load the [code](https://github.com/dDenVil/LightBot/blob/main/Code.txt).
- Connect everythig up
- Install it in the case

I have found an old case from charger, that fitted evrything perfectly inside.
![look](https://github.com/dDenVil/LightBot/blob/main/Readme_assets/device.png)



