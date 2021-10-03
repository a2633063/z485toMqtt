#ifndef __USER_MQTT_H__
#define __USER_MQTT_H__

extern void user_mqtt_init(void);
extern char * user_mqtt_get_state_topic(void);
extern char * user_mqtt_get_set_topic();
extern char * user_mqtt_get_sensor_topic();
extern char * user_mqtt_get_will_topic();
#endif

