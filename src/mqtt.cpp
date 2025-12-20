//
// Created by justine on 18/12/25.
//

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

#define MQTT_MSG_BUFFER_SIZE 256

namespace mqtt {

const char* mqtt_server = "test.mosquitto.org";

PubSubClient get_client(WiFiClient &client) {
  return PubSubClient(client);
}

void publish(PubSubClient& mqtt_client, const String& topic, const String& msg) {
  Serial.print("Publish message: ");
  Serial.println(msg);
  mqtt_client.publish("webradio/outTopic", msg.c_str());
}

void setup(
  PubSubClient &mqtt_client,
  const std::function<void(char*, byte*, unsigned int length)>& callback) {

  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(callback);
}

void reconnect(PubSubClient &mqtt_client) {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqtt_client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqtt_client.publish("webradio/outTopic", "hello world");
      // ... and resubscribe
      mqtt_client.subscribe("webradio/inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

};

