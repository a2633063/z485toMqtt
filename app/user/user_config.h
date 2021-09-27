#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

//#define USE_OPTIMIZE_PRINTF

#include "os_type.h"

#define BCDtoDEC(x) ( ((x)>>4)*10+(x)%0x10  )           //BCD��ת��Ϊʮ���Ʊ�ʾ��ʽ
#define DECtoBCD(x) ( (((x)/10)<<4)+(x)%10  )           //ʮ����ת��ΪBCD���ʾ��ʽ

#define ZLIB_WIFI_CALLBACK_REPEAT  (1)  //wifi���ӹ����� �Ƿ��ظ����ûص�����
#define ZLIB_WIFI_MDSN_ENABLI  (1)      //�Ƿ�����mdns,Ĭ�ϲ�����mdns

#define VERSION "v0.0.2"

#define TYPE 99

#define TYPE_NAME "zC1"
#define DEVICE_NAME "zC1_%s"


#define USER_CONFIG_VERSION 1

#define SETTING_MQTT_STRING_LENGTH_MAX  64      //���� 4 �ֽڶ��롣
#define NAME_LENGTH 32		//���������ַ�����󳤶�

//����ֵ
#define ALERT_OFF		0xff	//�ز���
#define ALERT_NO_BEEP	0x02	//�������ط�����
#define ALERT_WITH_BEEP	0x01	//��������������

//�û���������ṹ��
typedef struct
{
    char version;
    uint8_t name[NAME_LENGTH];
    uint8_t mqtt_ip[SETTING_MQTT_STRING_LENGTH_MAX];   //mqtt service ip
    uint16_t mqtt_port;        //mqtt service port
    uint8_t mqtt_user[SETTING_MQTT_STRING_LENGTH_MAX];     //mqtt service user
    uint8_t mqtt_password[SETTING_MQTT_STRING_LENGTH_MAX];     //mqtt service user

} user_config_t;

/*
 //����ͨ�����ݽṹ��
 typedef struct {
 uint8_t flag;	//�Ƿ��Ѿ�����
 uint8_t cmd;	//cmd
 uint16_t msgID1;	//msgID
 uint8_t reed;	//�ɻɹ�״̬
 struct struct_time now;
 uint8_t msgID2;	//msgID
 uint8_t battery;	//����
 uint8_t alert;	//��������
 struct struct_time alert_time_start;	//����ʱ��
 struct struct_time alert_time_end;	//����ʱ��
 } user_sensor_t;*/
extern user_config_t user_config;

#endif

