#pragma once

bool native_startAudioRec();
int native_stopAudioRec();
bool native_getRecBuffer(short *buf, int size);
bool native_playBuffer(short *buf, int size);
void native_stopPlay();

void native_startDnsSD();
int native_getDnsSDdiscoveryNum();
void native_getDnsSDdiscoveryItem(int i, char *buf, int bsize);
void native_stopDnsSD();

void native_startSmartConfig(const char *ssid, const char *pwd);
void native_stopSmartConfig();

void native_getSSID(char *buf, unsigned bsize);
