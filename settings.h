// WiFi Configuration
const char* ssid = "¿?";
const char* password = "¿?";

// MQTT Cibfiguration
const char* mqtt_server = "192.168.1.114";
const int mqtt_port = 1883;
const char* mqtt_id = "Irrigation_sensor";
const char* mqtt_pub_topic_waterflow = "/home/irrigation_sensor/water";
const char* mqtt_sub_topic_healthcheck = "/home/irrigation_sensor";
const char* mqtt_sub_topic_operation = "/home/irrigation_sensor/operation";

// Other params
const int update_time_sensors = 30000;
#define FlowSensorPin D2 // Or any other interrupt-capable pin

