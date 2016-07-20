#include "Edison_OLED.h"
#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <signal.h>

edOLED oled;

char* wifiIp;
int isWifiConnected = 0;
int batteryLevel = 0;
int cpuUsage = 0;

char* exec(const char* command) {
    FILE* fp;
    char* line = NULL;
    char* result = (char*) calloc(1, 1);
    size_t len = 0;
    fflush(NULL);
    fp = popen(command, "r");
    if (fp == NULL) {
        printf("Cannot execut command:\n%s\n", command);
        return NULL;
    }
    while(getline(&line, &len, fp) != -1) {
        result = (char*) realloc(result, strlen(result) + strlen(line) + 1);
        strncpy(result + strlen(result), line, strlen(line) + 1);
        free(line);
        line = NULL;
    }
    fflush(fp);
    if (pclose(fp) != 0) {
        perror("Cannot close stream.\n");
    }
    return result;
}

char* getWifiIp() {
    char* res = exec("ifconfig wlan0 | grep 'inet addr' |\
     grep -Eo 'inet addr:[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+' |\
     grep -Eo '[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+'");
    res[strlen(res) - 1] = '\0';
    return res;
}

void updateBatteryData() {
    batteryLevel = atoi(exec("battery-voltage | grep 'Battery level' | grep -Eo [0-9]+"));
}

void updateWifiData() {
    wifiIp = getWifiIp();
    isWifiConnected = strlen(wifiIp);
}

void updateCpuData() {
    cpuUsage = 100 - atoi(exec("top -n 1 -b | grep -E 'CPU.+[0-9]+\% idle' | grep -Eo '[0-9]+\% idle' | grep -Eo '[0-9]+'"));
}

void drawWifi(edOLED o, int on) {
    if (on) {
        o.line(0, 0, 10, 0);
        o.line(2, 2, 8, 2);
        o.line(4, 4, 6, 4);
    } else {
        o.line(0, 0, 11, 0);
        o.line(0, 0, 5, 5);
        o.line(5, 5, 10, 0);
    }
}
void drawBattery(edOLED o, int pct) {
    o.rect(LCDWIDTH - 20, 0, 20, 6);
    o.rectFill(LCDWIDTH - 19, 1,(int)round(((double)pct/100)*20) ,5);
}
void drawExit(edOLED o) {
    o.clear(PAGE);
    o.setCursor(10,20);
    o.setFontType(0);
    o.print("goodbye");
    o.display();
    sleep(3);
    o.clear(PAGE);
    o.display();
}

void handleSignal(int sig) {
    switch(sig) {
        case SIGINT: 
            drawExit(oled);
            exit(0);
        default: return;
    }
}

void updateAllData() {
    updateBatteryData();
    updateCpuData();
    updateWifiData();
}
void render() {
    drawWifi(oled, isWifiConnected);
    drawBattery(oled, batteryLevel);
    oled.display();
}
int main(void) {
    signal(SIGINT, handleSignal);

    oled.begin();
    oled.clear(PAGE);
    oled.display();

    oled.setCursor(10,20);
    oled.setFontType(0);
    oled.print("hi there");

    oled.display();
    
    while(1) {
        updateAllData();
        render();
        printf("isWifiConnected [%s]\n", wifiIp);
        printf("getBatteryLevel %d\n", batteryLevel);
        printf("cpuUsage %d", cpuUsage);
        sleep(5);
    }

    return 0;
}

