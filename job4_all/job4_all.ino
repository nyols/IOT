#include <painlessMesh.h>
#include <DHT.h>

#define LED 2
#define BLINK_PERIOD 3000
#define BLINK_DURATION 100
#define MESH_SSID "ISEP-4G"
#define MESH_PASSWORD "keluargacemara"
#define MESH_PORT 5555

#define SOIL_MOISTURE_PIN A0
#define DHT_PIN 32
#define DHT_TYPE DHT11
#define MQ2_PIN 33

DHT dht(DHT_PIN, DHT_TYPE);

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;
bool calc_delay = false;
SimpleList<uint32_t> nodes;

void sendMessage();
void receivedCallback(uint32_t from, String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);
void delayReceivedCallback(uint32_t from, int32_t delay);

Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage); // start with a one second interval
Task blinkNoNodes;
bool onFlag = false;

float soilMoistureValue = 0.0;
float temperature = 0.0;
float humidity = 0.0;
int mq2Value = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  dht.begin();

  mesh.setDebugMsgTypes(ERROR | DEBUG); // set before init() so that you can see error messages
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();

  blinkNoNodes.set(BLINK_PERIOD, (mesh.getNodeList().size() + 1) * 2, []() {
    if (onFlag)
      onFlag = false;
    else
      onFlag = true;

    blinkNoNodes.delay(BLINK_DURATION);

    if (blinkNoNodes.isLastIteration()) {
      blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
      blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD * 1000)) / 1000);
    }
  });

  userScheduler.addTask(blinkNoNodes);
  blinkNoNodes.enable();
}

void loop() {
  mesh.update();
  digitalWrite(LED, !onFlag);
  delay(1);
}

void sendMessage() {
  String msg = "Soil Moisture: " + String(soilMoistureValue) + " | Temperature: " + String(temperature) +
               " | Humidity: " + String(humidity) + " | MQ-2 Value: " + String(mq2Value);

  mesh.sendBroadcast(msg);

  if (calc_delay) {
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end()) {
      mesh.startDelayMeas(*node);
      node++;
    }
    calc_delay = false;
  }

  Serial.println("Sending message: " + msg);
  taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5)); // between 1 and 5 seconds
}

void receivedCallback(uint32_t from, String &msg) {
  Serial.println("Received from " + String(from) + " message: " + msg);
}

void newConnectionCallback(uint32_t nodeId) {
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD * 1000)) / 1000);

  Serial.println("New Connection, nodeId = " + String(nodeId));
  Serial.println(mesh.subConnectionJson(true));
}

void changedConnectionCallback() {
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD * 1000)) / 1000);

  nodes = mesh.getNodeList();
  Serial.println("Num nodes: " + String(nodes.size()));
  Serial.print("Connection list:");

  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) {
    Serial.print(" " + String(*node));
    node++;
  }

  Serial.println();
  calc_delay = true;
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.println("Adjusted time " + String(mesh.getNodeTime()) + ". Offset = " + String(offset));
}

void delayReceivedCallback(uint32_t from, int32_t delay) {
  Serial.println("Delay to node " + String(from) + " is " + String(delay) + " us");
}

void readSensors() {
  soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  mq2Value = analogRead(MQ2_PIN);
}
