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

#define SPI_CLK_PIN 5
#define SPI_MISO_PIN 19
#define SPI_MOSI_PIN 18

#define VS1053_CS 32
#define VS1053_DCS 33
#define VS1053_DREQ 15

// nom et mot de passe de votre réseau:
const char *ssid = "lily";
const char *password = "2301605160";

#define BUFFSIZE 64  //32, 64 ou 128
uint8_t mp3buff[BUFFSIZE];

int volume = 85;  // volume sonore 0 à 100
uint8_t tonalite[4] = {0, 1, 0, 15};

#define NOMBRECHAINES 8 // nombre de chaînes prédéfinies
int chaine = 0; //station actuellement sélectionnée

uint8_t spatial_level = 0;
WiFiManager wm;
WiFiClient client;

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
  player_stream.connecttohost(chaines[chaine]);
}

void setup() {
  WiFi.mode(WIFI_STA);
  SPI.setHwCs(true);
  SPI.begin(SPI_CLK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);
  Serial.begin(115200);

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
  player_stream.loop();
  delay(5);

  if (Serial.available()) {
    const char c = Serial.read();

    // n: prochaine chaine
    if (c == 'n') {
      Serial.println("Chaine suivante");
      client.stop();
      if (chaine < (NOMBRECHAINES - 1)) {
        chaine++;
      }
      else { // retour au début de la liste
        chaine = 0;
      }
      connexionChaine(chaine);
    }

    if (c == 'v') {
      Serial.println("Chaine précédente");
      client.stop();
      if (chaine > 0) {
        chaine--;
      }
      else {
        chaine = (NOMBRECHAINES - 1);
      }
      connexionChaine(chaine);
    }

    // +: augmenter le volume
    if (c == '+') {
      if (volume < 100) {
        Serial.println("Plus fort");
        volume++;
        player.setVolume(volume);
      }
    }

    // -: diminuer le volume
    if (c == '-') {
      if (volume > 0) {
        Serial.println("Moins fort");
        volume--;
        player.setVolume(volume);
      }
    }

    if (c == 'g') {
      if (tonalite[2] < 15) {
        tonalite[2]++;
        Serial.print("Bass + (");
        Serial.print(tonalite[2]);
        Serial.println(")");
        player.setTone(tonalite);
      }
    }

    if (c == 'f') {
      if (tonalite[2] > 0) {
        tonalite[2]--;
        Serial.print("Bass - (");
        Serial.print(tonalite[2]);
        Serial.println(")");
        player.setTone(tonalite);
      }
    }
    if (c == 'j') {
      if (tonalite[0] < 15) {
        tonalite[0]++;
        Serial.print("Treble + (");
        Serial.print(tonalite[0]);
        Serial.println(")");
        player.setTone(tonalite);
      }
    }

    if (c == 'h') {
      if (tonalite[0] > 0) {
        tonalite[0]--;
        Serial.print("Treble - (");
        Serial.print(tonalite[0]);
        Serial.println(")");
        player.setTone(tonalite);
      }
    }

    if (c == 'd') {
      tonalite[0] = 0;
      tonalite[2] = 0;
      player.setTone(tonalite);
      Serial.println("Tonalité par défaut");
    }

    // if (c == 's') {
    //   spatial_level = (spatial_level + 1) % 4;
    //   player_stream.setSpatial(spatial_level);
    //   Serial.print("Spatialisation : ");
    //   Serial.println(spatial_level);
    // }
  }

  // if (client.available() > 0) {
  //   uint8_t bytesread = client.read(mp3buff, BUFFSIZE);
  //   if (bytesread) {
  //     player_stream.playChunk(mp3buff, bytesread);
  //   }
  // }
}