# LightBot | Telegram Bot on the ESP8265 | Ukraine issues
### It tells me whether I have light at home or not.

Telegram Bot on the ESP to know if you have light at home. Actual for Ukraine.     
So for the last 2 months I didn't have light for 12h almost every day. There supposed to be some schedule, that sometimes worked.    

### Problem

Imagine you don't have light at home and you go out to spend some time until light is goes on. And you don't know when light will go on. You can call your family or neighbor if they are at home, but that is not my case.    
Previously a made a [Plant Life Project]() that notificates me with the message **"ONLINE"** if light at home goes on. But that was not enough for me.     

**In addition** of knowing when light is going on, I want to know:
- For how long I have light;
- For how long I didn't have light;
- Time of light goes off;
- To help other people to know is there light or not.

### Solution

I came up with idea of making telegram bot or how I called it **"Lightbot"**. **The principle** of work of this bot is simple. Let's have a look at the picture below.

![concept](https://github.com/dDenVil/LightBot/blob/main/Readme_assets/Is_light_at_home.png)

Power supply will charge battety, to power ESP if light goes off. With help of optocoupler I could detect if there is power on power supply.

Also I have **the generator**, that we manualy are turning on. And I wanted to detect if light is on from the generator or the grid. The idea was to meassure the level of noise with help of microphone. There is no othe way to detect it, because the device will be installed inside the house, and there is no obvious voltage difference beetwen grid power and the generator power.      
But lately I abandoned this idea with the detection of the generator. Since we turn it on manually, I just will have to unplug the lightbot.

## How it works





