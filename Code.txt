#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Wifi network station credentials
#define WIFI_SSID "SSID"   
#define WIFI_PASSWORD "PASS"
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "1234567:Bot_TokeN"
String chat_id="CHANNEL_ID";
int timezone = 2  ;


long sleep = 300e6 ;  //10 min = 600  sec   NOT USED
boolean  powerstatus = 0; 
boolean last_powerstatus=1;
int ihh = 0; // current time hh:mm:ss
int imm = 0;
int iss = 0;

String hh;
String mm;
String ss;

unsigned long start_tt;
unsigned long stop_tt;

long diff_tt;
int diff_hh;
int diff_mm;
int diff_ss;


String text1, text2, text3, text4;
const int OPTO_PIN=12;
const unsigned long opto_time= 10000; // period of checking optocoupler 10 sec
unsigned long opto_lasttime;  
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", timezone*3600);

WiFiClientSecure secured_client;
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

void setup() {
  Serial.begin(115200);
  Serial.println();
  pinMode(OPTO_PIN, INPUT_PULLUP); //sets the internal pullup resistor  
  initWiFi();  
}
//______________________________________________________________________________________________________________________ 
void loop() {
  
  if (millis() - opto_lasttime > opto_time) {   //checking optocoupler every 30sec
    opto_lasttime = millis();
    powerstatus=digitalRead(OPTO_PIN);
    Serial.println("Value Checked");

      if (powerstatus==1) { // -----------------------NO POWER--------------------
      Serial.println("Power is Off. Have to go sleep");
      powercheck();
      light_sleep();
      delay(2000);
      initWiFi();    // got wrong time, idk why! we got past timing
      return;
      }

      if (powerstatus==0) {  // -----------------------IS POWER--------------------
      Serial.println("Power is ON");
      powercheck();
      editMes();
      }
     
      
  }
}

//______________________________________________________________________________________________________________________
void initWiFi(){
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  configTime(timezone*3600, 0, "europe.pool.ntp.org");      // get UTC time via NTP for Telegram
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org

    /*
    timeClient.update();
    time_t  last_gt ; 
    delay(5000);
    while(last_gt != timeClient.getEpochTime()) {
    timeClient.update();
    last_gt = timeClient.getEpochTime();
    Serial.print("_ ");
    delay(500);
    timeClient.update();
    }
    timeClient.update();
    Serial.println();
    Serial.println(timeClient.getFormattedTime());
    */

    while ( timeClient.update() == false){
      Serial.print("_ ");
      timeClient.update();
      delay(500); 
    }
    
}
//______________________________________________________________________________________________________________________
void editMes(){
        //gettime();
        diff_tt = getTime() - start_tt;
        diff_ss = diff_tt % 60;  // ostatok from deviding
        diff_mm= (diff_tt / 60) % 60;
        diff_hh = diff_tt / 3600;
        Serial.println(getTime());
        
        bot.sendMessage(chat_id, text4 + text3 + diff_hh + "год." + diff_mm + "хв." + diff_ss +"сек.</code>","HTML", bot.last_sent_message_id); 
}
//______________________________________________________________________________________________________________________
void gettime(void) {
  
  timeClient.update();
  
  ihh = timeClient.getHours();
  imm = timeClient.getMinutes();
  iss = timeClient.getSeconds();
  
  if (ihh<10) hh="0"+ String(ihh);
  else hh=String(ihh);
  if (imm<10) mm="0"+ String(imm);
  else mm=String(imm);
  if (iss<10) ss="0"+ String(iss);
  else ss=String(iss);

  Serial.print(hh);
  Serial.print(":");
  Serial.print(mm);
  Serial.print(":");
  Serial.println(ss);
  
}
unsigned long getTime() { //time in seconds from 1970
  time_t epochTime = timeClient.getEpochTime();
  return epochTime;
}
//______________________________________________________________________________________________________________________
void light_sleep(){
timeClient.end();
WiFi.mode(WIFI_OFF); 
wifi_station_disconnect();
wifi_set_opmode_current(NULL_MODE);
//extern os_timer_t *timer_list;
//timer_list = nullptr;
wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
gpio_pin_wakeup_enable(GPIO_ID_PIN(OPTO_PIN), GPIO_PIN_INTR_LOLEVEL); // level of wakning up
//wifi_fpm_set_wakeup_cb(fpm_wakup_cb_func);
wifi_fpm_open(); // Enables force sleep
wifi_fpm_do_sleep(0xFFFFFFF); // Sleep for longest possible time 0xFFFFFFF or sleep 
delay(100);
Serial.println(F("Woke up!"));
/*
gpio_pin_wakeup_disable();
wifi_fpm_close();
*/
}
//______________________________________________________________________________________________________________________
void powercheck(){

    if (powerstatus != last_powerstatus) {

      gettime(); //time
      last_powerstatus = powerstatus;
      Serial.println("Changed");

      stop_tt = getTime(); /////////////////////////////////////////////////////////////////////////////////////////////////////

      diff_tt=stop_tt-start_tt;
      diff_ss = diff_tt % 60;  // ostatok from deviding
      diff_mm = (diff_tt / 60) % 60;
      diff_hh = diff_tt / 3600;


      Serial.print(start_tt);
      Serial.print(" - ");
      Serial.print(stop_tt);
      Serial.print(" - "); 
      Serial.print(diff_mm);
      Serial.print(":");
      Serial.println(diff_ss);
      
      start_tt = stop_tt; /////////////////////////////////////////////////////////////////////////////////////////////////////

      
      if (powerstatus == 0){ 
      text1 = "\U0001F7E2 Світло <b>є</b> з <u>";
      text2 = ".</u> \nСвітла не було <u>";
      text3 = "\n \n<code>\U0001F4A1 ";
      text4 = text1 + hh + ":" + mm + text2 + diff_hh + " год. " + diff_mm + " хв.</u>";
      bot.sendMessage(chat_id, text4 + text3 +" 0год. 0хв. 0сек.</code>","HTML");
      }
      if (powerstatus == 1){
      text1 = "\U0001F534 Світла <b>немає</b> з <u>";
      text2 = ".</u> \nСвітло було <u>"; 
      //text3 = "\n \n<code>\U0001F4A1\U0001F6AB ";
      text4 = text1 + hh + ":" + mm + text2 + diff_hh + " год. " + diff_mm + " хв.</u>";
      bot.sendMessage(chat_id, text4,"HTML" ); // + text3 +" 0год. 0хв. 5сек.</code>"
      }
    }
  }
//______________________________________________________________________________________________________________________
void fpm_wakup_cb_func() {
  // ok to use blocking functions in the callback, but not
  // delay(), which appears to cause a reset
  Serial.println("Light sleep is over");
}
