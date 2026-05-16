#include "channel_hopper.h"
#include "config.h"

extern void addLog(const String& msg);

ChannelHopper::ChannelHopper() {
    apChannel = AP_CHANNEL;
    currentChannel = AP_CHANNEL;
    hopping = false;
    lastHop = 0;
}

void ChannelHopper::begin(int apCh) {
    apChannel = apCh;
    currentChannel = apCh;
    wifi_set_channel(currentChannel);
    addLog("[HOPPER] Canal AP: " + String(apChannel));
}

void ChannelHopper::startHopping() {
    hopping = true;
    lastHop = millis();
    addLog("[HOPPER] Channel hopping iniciado");
}

void ChannelHopper::stopHopping() {
    hopping = false;
    returnToAPChannel();
    addLog("[HOPPER] Channel hopping detenido");
}

void ChannelHopper::update() {
    if (!hopping) return;
    
    unsigned long now = millis();
    if (now - lastHop >= CHANNEL_HOP_INTERVAL) {
        currentChannel++;
        if (currentChannel > MAX_CHANNEL) {
            currentChannel = MIN_CHANNEL;
        }
        wifi_set_channel(currentChannel);
        lastHop = now;
    }
}

bool ChannelHopper::isHopping() {
    return hopping;
}

int ChannelHopper::getCurrentChannel() {
    return currentChannel;
}

void ChannelHopper::setCurrentChannel(int ch) {
    if (ch >= MIN_CHANNEL && ch <= MAX_CHANNEL) {
        currentChannel = ch;
        wifi_set_channel(currentChannel);
    }
}

void ChannelHopper::returnToAPChannel() {
    currentChannel = apChannel;
    wifi_set_channel(currentChannel);
}
