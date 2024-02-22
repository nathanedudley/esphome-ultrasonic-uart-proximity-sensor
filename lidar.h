
#include "esphome.h"
#include <cmath>
#include <ctime>

// LiDAR data
unsigned char data[9] = {};
// Distance from LiDAR sensor, in cm
float distance;
// Last time a distance value was published
time_t lastPublishTime;
time_t now;
// How much the time has changed since the last state change and now
double diffTime;
// How much the distance has changed
float diffDistance;
// Max time to go before publishing state
double timeThreshold = 60;
// Min change in distance to trigger publishing state before timeThreshold is reached
double distanceThreshold = 30;
// Last distance value published
float lastPublishDistance;
char buffer[200];

class LiDARSensor : public Sensor, public Component, public UARTDevice {
public:
    // Constructor
    LiDARSensor(UARTComponent *parent) : UARTDevice(parent) {}

    void setup() override {
        // Initialization, if required
    }

    void loop() override {
        if (available() >= 9) {
            read_bytes(data, 9);
            if (data[0] == 0x59 && data[1] == 0x59) {
                distance = (data[2] + (data[3] << 8));

                time(&now);
                diffTime = difftime(now, lastPublishTime);
                diffDistance = abs(distance - lastPublishDistance);

                // Publish state if either distance threshold or time threshold are reached, max of once per second
                if ((diffDistance >= distanceThreshold && diffTime > 0) || diffTime >= timeThreshold) {
                    lastPublishDistance = distance;
                    time(&lastPublishTime);
                    sprintf(buffer, "Publishing state: %f cm after %.0lf seconds", distance, diffTime);
                    ESP_LOGD("custom", buffer);
                    publish_state(distance); // Publish distance in cm
                }
            } else {
                ESP_LOGW("custom", "Header mismatch or data error");
            }
            // Optional: Clear the buffer if you expect data at a high rate to prevent overflow
        }
    }
};
