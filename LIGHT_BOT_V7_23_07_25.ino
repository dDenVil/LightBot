#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <time.h>

// Wifi network station credentials
#define WIFI_SSID_1 "YOUR_WIFI_NAME1"   
#define WIFI_PASSWORD_1 "YOUR_WIFI_PASS1"
#define WIFI_SSID_2 "YOUR_WIFI_NAME2 or YOUR_WIFI_NAME1"   //  it will connect to the best avaliable WIFI
#define WIFI_PASSWORD_2 "YOUR_WIFI_PASS1 or YOUR_WIFI_PASS2"
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "1234567890:Bot1234568954fsdfTocken2312" // your bot token
String chat_id="YOUR_ID_GROUP"; // ID of your group
String first_msg_chat_id = "YOUR_ID"; // first start message of Power ON will be send to you only

boolean powerstatus = 0; 
boolean last_powerstatus = 1;
boolean first_msg = true; // Variable to check if it's the first message
boolean message_sent_on = false;  // флаг відправки повідомлення про світло
boolean message_sent_off = false; // флаг відправки повідомлення про відсутність світла

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
const int OPTO_PIN = 14;
const unsigned long opto_time = 10000; // period of checking optocoupler 10 sec
unsigned long opto_lasttime;
const unsigned long edit_time = 10000; // оновлення повідомлення кожну хвилину
unsigned long edit_lasttime;
int last_message_id = 0; // ID останнього повідомлення для оновлення

WiFiClientSecure secured_client;
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// Функція для автоматичного визначення часового поясу Києва
int getKyivTimezone() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  
  // Визначаємо літній/зимовий час для України
  int month = timeinfo->tm_mon + 1;  // tm_mon починається з 0
  int day = timeinfo->tm_mday;
  
  // Літній час в Україні: останія неділя березня - остання неділя жовтня
  bool isDST = false;
  
  if (month > 3 && month < 10) {
    isDST = true; // квітень-вересень - завжди літній час
  } else if (month == 3) {
    // Березень - перевіряємо, чи пройшла остання неділя
    int lastSunday = 31;
    for (int i = 31; i >= 25; i--) {
      struct tm testTime = *timeinfo;
      testTime.tm_mday = i;
      mktime(&testTime);
      if (testTime.tm_wday == 0) { // неділя
        lastSunday = i;
        break;
      }
    }
    isDST = (day >= lastSunday);
  } else if (month == 10) {
    // Жовтень - перевіряємо, чи ще не пройшла остання неділя
    int lastSunday = 31;
    for (int i = 31; i >= 25; i--) {
      struct tm testTime = *timeinfo;
      testTime.tm_mday = i;
      mktime(&testTime);
      if (testTime.tm_wday == 0) { // неділя
        lastSunday = i;
        break;
      }
    }
    isDST = (day < lastSunday);
  }
  
  return isDST ? 3 : 2; // UTC+3 літом, UTC+2 взимку
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  pinMode(OPTO_PIN, INPUT_PULLUP); //sets the internal pullup resistor  
  initWiFi();
  start_tt = getTime(); // ініціалізуємо стартовий час
}

void loop() {
  if (millis() - opto_lasttime > opto_time) {   //checking optocoupler every 10 sec
    opto_lasttime = millis();
    powerstatus = digitalRead(OPTO_PIN);
    Serial.println("Value Checked");

    if (powerstatus == 1) { // -----------------------NO POWER--------------------
      Serial.println("Power is Off. Have to go sleep");
      delay(5000); // if power off was for 5sec, negotiate it
      powercheck();
      
      // Переходимо в сон тільки якщо повідомлення відправлено
      if (message_sent_off) {
        Serial.println("Message sent, going to sleep...");
        light_sleep();
        initWiFi();    // got wrong time, idk why! we got past timing
        delay(5000);  // otherwise time will be incorrect, to get current time
      }
      return;
    }

    if (powerstatus == 0) {  // -----------------------IS POWER--------------------
      Serial.println("Power is ON");
      powercheck();
    }
  }
  
  // Оновлення повідомлення про час роботи світла кожну хвилину
  if (powerstatus == 0 && message_sent_on && millis() - edit_lasttime > edit_time) {
    edit_lasttime = millis();
    editMes();
  }
}

void initWiFi() {
  Serial.print("Connecting to Wifi... ");
  int n = WiFi.scanNetworks();
  int bestNetwork = -1;
  int bestSignal = -100; // Lowest signal strength

  for (int i = 0; i < n; i++) {
    if (WiFi.SSID(i) == WIFI_SSID_1 && WiFi.RSSI(i) > bestSignal) {
      bestNetwork = 1;
      bestSignal = WiFi.RSSI(i);
    }
    if (WiFi.SSID(i) == WIFI_SSID_2 && WiFi.RSSI(i) > bestSignal) {
      bestNetwork = 2;
      bestSignal = WiFi.RSSI(i);
    }
  }

  if (bestNetwork == 1) {
    WiFi.begin(WIFI_SSID_1, WIFI_PASSWORD_1);
  } else if (bestNetwork == 2) {
    WiFi.begin(WIFI_SSID_2, WIFI_PASSWORD_2);
  } else {
    Serial.println("No known networks found.");
    return;
  }

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startAttemptTime > 40000) { // 40 seconds timeout
      Serial.println("Failed to connect. Reconnecting...");
      WiFi.disconnect();
      delay(1000);
      WiFi.reconnect();
      startAttemptTime = millis(); // reset attempt timer
    }
    Serial.print(".");
    delay(500);
  }

  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  // Автоматично визначаємо часовий пояс для Києва
  int timezone = getKyivTimezone();
  Serial.println("Using timezone: UTC+" + String(timezone));
  configTime(timezone * 3600, 0, "pool.ntp.org", "time.nist.gov"); // get UTC time via NTP
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org

  // Чекаємо синхронізацію часу
  Serial.print("Waiting for time sync");
  int timeout = 0;
  while (!getTime() && timeout < 15) {
    Serial.print(".");
    delay(1000);
    timeout++;
  }
  Serial.println();
  
  if (getTime()) {
    gettime();
    Serial.println("Time synced: " + hh + ":" + mm + ":" + ss);
  }
}

void editMes() {
  if (last_message_id == 0) {
    Serial.println("No message ID to edit");
    return;
  }
  
  diff_tt = getTime() - start_tt;
  diff_ss = diff_tt % 60;  // остаток від ділення
  diff_mm = (diff_tt / 60) % 60;
  diff_hh = diff_tt / 3600;
  
  Serial.println("Updating message - Uptime: " + String(diff_hh) + ":" + String(diff_mm) + ":" + String(diff_ss));
  
  // Відправляємо нове повідомлення з оновленням замість редагування
  String update_text = text4 + text3 + String(diff_hh) + " год. " + String(diff_mm) + " хв. " + String(diff_ss) + " сек.</code>";
  
  // Використовуємо параметр для редагування повідомлення
  if (bot.sendMessage(chat_id, update_text, "HTML", last_message_id)) {
    Serial.println("Message updated successfully");
  } else {
    Serial.println("Failed to update message");
  }
}

void gettime(void) {
  time_t now;
  struct tm * timeinfo;
  time(&now);
  timeinfo = localtime(&now);  
  ihh = timeinfo->tm_hour;
  imm = timeinfo->tm_min;
  iss = timeinfo->tm_sec;
  
  if (ihh < 10) hh = "0" + String(ihh);
  else hh = String(ihh);
  if (imm < 10) mm = "0" + String(imm);
  else mm = String(imm);
  if (iss < 10) ss = "0" + String(iss);
  else ss = String(iss);

  Serial.print("Current time: ");
  Serial.print(hh);
  Serial.print(":");
  Serial.print(mm);
  Serial.print(":");
  Serial.println(ss);
}

unsigned long getTime() { //time in seconds from 1970
  time_t now;
  time(&now);
  if (now < 1000000000) { // перевірка чи час синхронізовано
    return 0;
  }
  return now;
}

void light_sleep() {
  Serial.println("Entering sleep mode...");
  WiFi.mode(WIFI_OFF); 
  wifi_station_disconnect();
  wifi_set_opmode_current(NULL_MODE);
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
  gpio_pin_wakeup_enable(GPIO_ID_PIN(14), GPIO_PIN_INTR_LOLEVEL); // level of waking up
  wifi_fpm_open(); // Enables force sleep
  wifi_fpm_do_sleep(0xFFFFFFF); // Sleep for longest possible time 0xFFFFFFF or sleep 
  delay(100);
  Serial.println(F("Woke up!"));
}

void powercheck() {
  if (powerstatus != last_powerstatus) {
    gettime(); //time
    last_powerstatus = powerstatus;
    Serial.println("Power status changed to: " + String(powerstatus));

    stop_tt = getTime();
    if (stop_tt == 0) {
      Serial.println("Time not synced, retrying...");
      delay(2000);
      stop_tt = getTime();
    }
    
    diff_tt = stop_tt - start_tt;
    diff_ss = diff_tt % 60;  // залишок від ділення
    diff_mm = (diff_tt / 60) % 60;
    diff_hh = diff_tt / 3600;

    // Check if diff_hh is greater than 4000 (invalid time)
    if (diff_hh > 4000) {
      diff_hh = 0;
      diff_mm = 0;
    }

    start_tt = stop_tt; 
    Serial.println("Timestamp: " + String(stop_tt));

    if (powerstatus == 0 && !message_sent_on) { // Світло з'явилося
      // If this is the first message for powerstatus == 0
      String target_chat = first_msg ? first_msg_chat_id : chat_id;
      
      text1 = "\U0001F7E2 Світло <b>є</b> з <u>";
      text2 = ".</u> \nСвітла не було <u>";
      text3 = "\n \n<code>\U0001F4A1 ";
      text4 = text1 + hh + ":" + mm + text2 + String(diff_hh) + "год. " + String(diff_mm) + "хв.</u>";
      String message_text = text4 + text3 + "0год. 0хв. 0сек.</code>";
      
      if (bot.sendMessage(target_chat, message_text, "HTML")) {
        last_message_id = bot.last_sent_message_id;
        message_sent_on = true;
        message_sent_off = false;
        edit_lasttime = millis(); // скидаємо таймер оновлення
        first_msg = false;
        Serial.println("Light ON message sent successfully to " + target_chat);
      } else {
        Serial.println("Failed to send light ON message");
      }
    }
    
    if (powerstatus == 1 && !message_sent_off) { // Світло зникло
      text1 = "\U0001F534 Світла <b>немає</b> з <u>";
      text2 = ".</u> \nСвітло було <u>"; 
      text4 = text1 + hh + ":" + mm + text2 + String(diff_hh) + "год. " + String(diff_mm) + "хв.</u>";
      
      if (bot.sendMessage(chat_id, text4, "HTML")) {
        message_sent_off = true;
        message_sent_on = false;
        last_message_id = 0; // скидаємо ID для наступного циклу
        Serial.println("Light OFF message sent successfully");
      } else {
        Serial.println("Failed to send light OFF message");
      }
    }
  }
}
