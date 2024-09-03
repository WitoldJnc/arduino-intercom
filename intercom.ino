
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

const char* ssid = ""; //wifi ssid
const char* password = ""; //wifi password
#define BOTtoken ""; //tg bot token

X509List cert(TELEGRAM_CERTIFICATE_ROOT);

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 300;
unsigned long lastTimeBotRan;
const int pin14 = 14;  // d5
const int rxPin = 3;   //rx

const int MAX_SIZE = 10;   // Максимальный размер массива
String chatIds[MAX_SIZE];  // Пустой массив строк для хранения chat_id
int currentSize = 0;       // Текущий размер массива


void setup() {

  configTime(0, 0, "pool.ntp.org");  
  client.setTrustAnchors(&cert); 

  pinMode(rxPin, OUTPUT);
  pinMode(pin14, INPUT);
  digitalWrite(rxPin, 0);

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
}
bool isEmpty() {
  return currentSize == 0;
}

void addUniqueValue(String value) {
  bool found = false;
  for (int i = 0; i < currentSize; ++i) {
    if (chatIds[i] == value) {
      found = true;
      break;
    }
  }

  if (!found) {
    if (currentSize < MAX_SIZE) {
      chatIds[currentSize] = value;
      currentSize++;
    }
  }
}

void handleNewMessages(int numNewMessages) {
  int pinState = digitalRead(pin14);

  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/open") {
      if (pinState == HIGH) {
        digitalWrite(rxPin, HIGH);
        delay(3000);
        digitalWrite(rxPin, LOW);
      } else {
        digitalWrite(rxPin, LOW);
      }
      for (int j = 0; j < currentSize; j++) {
        bot.sendMessage(chatIds[j], "Дверь открыта", "");
      }
    }

    if (text == "/ignore") {
      delay(30000);
    }

    if (text == "/start") {
      String chat_id = bot.messages[i].chat_id;
      addUniqueValue(chat_id);
      String keyboardJson = "[[\"/open\", \"/ignore\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "В случае звонка вы можете:", "", keyboardJson, true);
    }
  }
}


void loop() {
  int pinState = digitalRead(pin14);

  if (pinState == HIGH) {
    for (int j = 0; j < currentSize; j++) {
      String keyboardJson = "[[\"/open\", \"/ignore\"]]";
      bot.sendMessageWithReplyKeyboard(chatIds[j], "В домофон звонят", "", keyboardJson, true);
    }
    delay(10000);
  }
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
