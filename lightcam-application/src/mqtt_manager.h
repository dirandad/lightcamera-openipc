
#if !defined(__MQTT_MANAGER_H__)
#define __MQTT_MANAGER_H__

#include "mqtt.h"


int mqtt_manager_init(char* addr, char* port, char* topic, char* user, char* password, void *publish_callback);
int mqtt_manager_publish(char* topic, char message[256]);
void mqtt_manager_uninit();

#endif