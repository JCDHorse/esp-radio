//
// Created by justine on 20/12/25.
//

#ifndef WEBRADIO_H
#define WEBRADIO_H

#include "ESP32_VS1053_Stream.h"
#include "PubSubClient.h"
#include "VS1053.h"

#define VS1053_CS 32
#define VS1053_DCS 33
#define VS1053_DREQ 15

class webradio {
private:
  static const size_t CHANNEL_COUNT;
  static const char *CHANNELS[CHANNEL_COUNT];

  VS1053 m_player;
  ESP32_VS1053_Stream m_stream;
  PubSubClient m_mqtt;

  bool m_started;
  size_t m_channel;
  uint8_t m_volume;
  uint8_t m_tone[4];

public:
  webradio(WiFiClient& _wifi_client);
  void handle_command(char cmd);
  void init();

  // Channels
  void connect_channel();
  void next_channel();
  void prev_channel();

  // Volume
  void volume_up();
  void volume_down();

  // Tone
  void bass_up();
  void bass_down();
  void treble_up();
  void treble_down();
  void tone_default();
};



#endif //WEBRADIO_H
