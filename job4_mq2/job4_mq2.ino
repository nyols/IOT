#include <painlessMesh.h>

#define MQ2_PIN 33
#define MESH_SSID "@stianza_"
#define MESH_PASSWORD "123456789101"
#define MESH_PORT 5555

Scheduler userScheduler;
painlessMesh mesh;

void sendMessage();
void receivedCallback(uint32_t from, String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);
void delayReceivedCallback(uint32_t from, int32_t delay);

Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);

float mq2Value = 0.0;

void setup() {
  Serial.begin(115200);
  pinMode(MQ2_PIN, INPUT);

  mesh.setDebugMsgTypes(ERROR | DEBUG);
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  mesh.update();
  delay(1);
}

void sendMessage() {
  mq2Value = analogRead(MQ2_PIN);

  String msg = "MQ-2 Value: " + String(mq2Value);

  mesh.sendBroadcast(msg);

  Serial.println("Sending message: " + msg);
  taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5)); // between 1 and 5 seconds
}

void receivedCallback(uint32_t from, String &msg) {
  Serial.println("Received from " + String(from) + " message: " + msg);
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.println("New Connection, nodeId = " + String(nodeId));
  Serial.println(mesh.subConnectionJson(true));
}

void changedConnectionCallback() {
  Serial.println("Changed connections");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.println("Adjusted time " + String(mesh.getNodeTime()) + ". Offset = " + String(offset));
}

void delayReceivedCallback(uint32_t from, int32_t delay) {
  Serial.println("Delay to node " + String(from) + " is " + String(delay) + " us");
}
