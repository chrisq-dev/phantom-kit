#ifndef CHANNEL_HOPPER_H
#define CHANNEL_HOPPER_H

#include <Arduino.h>

extern "C" {
    #include "user_interface.h"
}

class ChannelHopper {
public:
    ChannelHopper();
    void begin(int apChannel);
    void startHopping();
    void stopHopping();
    void update();
    bool isHopping();
    int getCurrentChannel();
    void setCurrentChannel(int ch);
    void returnToAPChannel();

private:
    int apChannel;
    int currentChannel;
    bool hopping;
    unsigned long lastHop;
};

#endif
