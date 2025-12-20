//
// Created by justine on 18/12/25.
//

#ifndef MQTT_H
#define MQTT_H

#include <PubSubClient.h>
#include <WiFiClient.h>

#define MQTT_MSG_BUFFER_SIZE	50

namespace mqtt {
  PubSubClient get_client(WiFiClient &client);
  void setup(PubSubClient &mqtt_client);
  void reconnect(PubSubClient &mqtt_client);
  void publish(PubSubClient &mqtt_client, const String& topic, const String& msg);
  void setup(
    PubSubClient &mqtt_client,
    const std::function<void(char*, byte*, unsigned int length)>& callback);
}

#endif //MQTT_H
