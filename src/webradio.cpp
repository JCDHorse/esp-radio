//
// Created by justine on 20/12/25.
//

#include "webradio.h"

#include "mqtt.h"

#include <HardwareSerial.h>

constexpr size_t webradio::CHANNEL_COUNT = 8;
const char *webradio::CHANNELS[CHANNEL_COUNT] = {
  "http://stream03.ustream.ca/cism128.mp3:8000",
  "http://streamer-03.rtl.fr/rtl-1-44-64?listen=webCwsBCggNCQgLDQUGBAcGBg;",
  "http://icecast.rtl.fr/rtl-1-44-64",
  "http://radios.rtbf.be/wr-c21-metal-128.mp3",
  "http://direct.franceinter.fr/live/franceinter-midfi.aac",
  "http://ice4.somafm.com/seventies-128-mp3",
  "http://lyon1ere.ice.infomaniak.ch/lyon1ere-high.mp3",
  "http://stream.rcs.revma.com/5gd04cwptg0uv",
};

webradio::webradio(WiFiClient &_wifi_client)
: m_player(VS1053_CS, VS1053_DCS, VS1053_DREQ),
  m_mqtt(_wifi_client),
  m_started(false),
  m_channel(0),
  m_volume(70),
  m_tone{0, 1, 0, 15}
{}

void webradio::handle_command(const char cmd) {
  switch (cmd) {
  case 'n':
    next_channel();
    break;
  case 'v':
    prev_channel();
    break;
  case '+':
    volume_up();
    break;
  case '-':
    volume_down();
    break;
  case 'g':
    bass_up();
    break;
  case 'f':
    bass_down();
    break;
  case 'j':
    treble_up();
    break;
  case 'h':
    treble_down();
    break;
  case 'd':
    tone_default();
    break;
  case 's':
    break;
  default:
    break;
  }
}

void webradio::init() {
  // Initialize the VS1053 decoder
  if (m_stream.startDecoder(VS1053_CS, VS1053_DCS, VS1053_DREQ)
      || m_stream.isChipConnected()) {
    Serial.println("Decoder not running - system halted");
    return;
  }
  m_started = true;
}

void webradio::connect_channel() {
  static char msg[MQTT_MSG_BUFFER_SIZE];

  Serial.print("Connection a ");

  m_stream.stopSong();
  Serial.print("Demande du stream: ");
  Serial.println(CHANNELS[m_channel]);

  snprintf(msg, MQTT_MSG_BUFFER_SIZE, "Changement de chaine radio: %s", CHANNELS[CHANNEL_COUNT]);
  mqtt::publish(m_mqtt, "webradio/inTopic", msg);
  m_stream.connecttohost(CHANNELS[m_channel]);
}

void webradio::next_channel() {
  m_channel = (m_channel + 1) % CHANNEL_COUNT;
  connect_channel();
}

void webradio::prev_channel() {
  m_channel = m_channel == 0 ? CHANNEL_COUNT - 1 : m_channel - 1;
  connect_channel();
}
void webradio::volume_up() {
  m_volume += 5;
  m_player.setVolume(m_volume);
}

void webradio::volume_down() {
  m_volume -= 5;
  m_player.setVolume(m_volume);
}
void webradio::bass_up() {
  m_tone[2]++;
  m_player.setTone(m_tone);
}

void webradio::bass_down() {
  m_tone[2]--;
  m_player.setTone(m_tone);
}

void webradio::treble_up() {
  m_tone[2]++;
  m_player.setTone(m_tone);
}

void webradio::treble_down() {
  m_tone[2]--;
  m_player.setTone(m_tone);
}

void webradio::tone_default() {
  m_tone[0] = 0;
  m_tone[1] = 1;
  m_tone[2] = 0;
  m_tone[3] = 15;
  m_player.setTone(m_tone);
}

