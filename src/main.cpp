/*******************************************************************

  Web radio simple à base d'ESP32 et VS1053

  Basé sur un sketch de Vince Gellár (github.com/vincegellar)

  Bibliotheque VS1053 de baldram (https://github.com/baldram/ESP_VS1053_Library)

  Plus d'infos:  https://electroniqueamateur.blogspot.com/2021/03/esp32-et-vs1053-ecouter-la-radio-sur.html


*********************************************************************/

#include <Arduino.h>
#include <WiFiManager.h>
#include <ESP32_VS1053_Stream.h>
#include <PubSubClient.h>

#include "mqtt.h"

#define SPI_CLK_PIN 5
#define SPI_MISO_PIN 19
#define SPI_MOSI_PIN 18

#define VS1053_CS 32
#define VS1053_DCS 33
#define VS1053_DREQ 15

#define NOMBRECHAINES 8 // nombre de chaînes prédéfinies

// nom et mot de passe de votre réseau:
const char *ssid = "octavia";
const char *password = "12341234";

char msg[MQTT_MSG_BUFFER_SIZE];
unsigned long lastMsg = 0;
int value = 0;

#define BUFFSIZE 64  //32, 64 ou 128
uint8_t mp3buff[BUFFSIZE];

int volume = 85;  // volume sonore 0 à 100
uint8_t tonalite[4] = {0, 1, 0, 15};

int chaine = 0; //station actuellement sélectionnée

uint8_t spatial_level = 0;
WiFiManager wm;
WiFiClient wifi_client;
PubSubClient mqtt_client = mqtt::get_client(wifi_client);

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);
ESP32_VS1053_Stream player_stream;

// connexion à une chaine
void connexionChaine (uint8_t chaine) {
  static const char *chaines[NOMBRECHAINES] = {
    "http://stream03.ustream.ca/cism128.mp3:8000",
    "http://streamer-03.rtl.fr/rtl-1-44-64?listen=webCwsBCggNCQgLDQUGBAcGBg;",
    "http://icecast.rtl.fr/rtl-1-44-64",
    "http://radios.rtbf.be/wr-c21-metal-128.mp3",
    "http://direct.franceinter.fr/live/franceinter-midfi.aac",
    "http://ice4.somafm.com/seventies-128-mp3",
    "http://lyon1ere.ice.infomaniak.ch/lyon1ere-high.mp3",
    "http://stream.rcs.revma.com/5gd04cwptg0uv",
  };

  Serial.print("Connection a ");

  player_stream.stopSong();
  Serial.print("Demande du stream: ");
  Serial.println(chaines[chaine]);

  snprintf(msg, MQTT_MSG_BUFFER_SIZE, "Changement de chaine radio: %s", chaines[chaine]);
  mqtt::publish(mqtt_client, "webradio/inTopic", msg);
  player_stream.connecttohost(chaines[chaine]);
}

void handleCommand(const char cmd) {
  // n: prochaine chaine
  if (cmd == 'n') {
    Serial.println("Chaine suivante");
    if (chaine < (NOMBRECHAINES - 1)) {
      chaine++;
    }
    else { // retour au début de la liste
      chaine = 0;
    }
    connexionChaine(chaine);
  }

  if (cmd == 'v') {
    Serial.println("Chaine précédente");
    if (chaine > 0) {
      chaine--;
    }
    else {
      chaine = (NOMBRECHAINES - 1);
    }
    connexionChaine(chaine);
  }

  // +: augmenter le volume
  if (cmd == '+') {
    if (volume < 100) {
      Serial.println("Plus fort");
      volume++;
      player.setVolume(volume);
    }
  }

  // -: diminuer le volume
  if (cmd == '-') {
    if (volume > 0) {
      Serial.println("Moins fort");
      volume--;
      player.setVolume(volume);
    }
  }

  if (cmd == 'g') {
    if (tonalite[2] < 15) {
      tonalite[2]++;
      Serial.print("Bass + (");
      Serial.print(tonalite[2]);
      Serial.println(")");
      player.setTone(tonalite);
    }
  }

  if (cmd == 'f') {
    if (tonalite[2] > 0) {
      tonalite[2]--;
      Serial.print("Bass - (");
      Serial.print(tonalite[2]);
      Serial.println(")");
      player.setTone(tonalite);
    }
  }
  if (cmd == 'j') {
    if (tonalite[0] < 15) {
      tonalite[0]++;
      Serial.print("Treble + (");
      Serial.print(tonalite[0]);
      Serial.println(")");
      player.setTone(tonalite);
    }
  }

  if (cmd == 'h') {
    if (tonalite[0] > 0) {
      tonalite[0]--;
      Serial.print("Treble - (");
      Serial.print(tonalite[0]);
      Serial.println(")");
      player.setTone(tonalite);
    }
  }

  if (cmd == 'd') {
    tonalite[0] = 0;
    tonalite[2] = 0;
    player.setTone(tonalite);
    Serial.println("Tonalité par défaut");
  }
}


void msg_callback(const char * topic, const byte * payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print(static_cast<char>(payload[i]));
    handleCommand(static_cast<char>(payload[i]));
  }
  Serial.println();
}


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  WiFi.mode(WIFI_STA);
  SPI.setHwCs(true);
  SPI.begin(SPI_CLK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);
  Serial.begin(115200);

  setup_wifi();
  mqtt::setup(mqtt_client, msg_callback);

  bool c = player_stream.startDecoder(VS1053_CS, VS1053_DCS, VS1053_DREQ);
  bool ic = player_stream.isChipConnected();

  // Initialize the VS1053 decoder
  if (!c || !ic) {
    Serial.println(c);
    Serial.println(ic);
    Serial.println("Decoder not running - system halted");
  }

  Serial.println("\n\nRadio WiFi");
  Serial.println("");

  Serial.println("Controles: ");
  Serial.println("\t n: synthoniser une autre chaine");
  Serial.println("\t + / -: controle du volume");

  Serial.print("Connexion au reseau ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  const bool res = wm.autoConnect("Justine-ESP32");
  if (!res) {
    Serial.println("Failed to connect");
  }
  else {
    Serial.println("WiFi connected");
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connecte");
  Serial.println("Adresse IP: ");
  Serial.println(WiFi.localIP());

  player_stream.setVolume(volume);
  player.begin();
  player.switchToMp3Mode();
  player.setVolume(volume);

  connexionChaine(chaine);

}

void loop() {
  if (!mqtt_client.connected()) {
    mqtt::reconnect(mqtt_client);
  }
  mqtt_client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf(msg, MQTT_MSG_BUFFER_SIZE, "hello world #%d", value);
    mqtt::publish(mqtt_client, "webradio/outTopic", msg);
  }

  player_stream.loop();
  delay(5);

  if (Serial.available()) {
    const char c = Serial.read();
    handleCommand(c);
  }

}