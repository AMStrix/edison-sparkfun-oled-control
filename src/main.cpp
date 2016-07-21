#include "Edison_OLED.h"
#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <regex.h>

edOLED oled;

char* wifiIp;
int isWifiConnected = 0;
int batteryLevel = 0;
int cpuUsage = 0;
int memoryUsage = 0;
int ssdUsage = 0;
int networkConnections = 0;

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
    cpuUsage = 100 - atoi(exec("top -d 1 -n 1 -b | grep -E 'CPU.+[0-9]+\% idle' | grep -Eo '[0-9]+\% idle' | grep -Eo '[0-9]+' | tail -1"));
}

void updateMemoryData() {
    int res;
    int len;
    char result[BUFSIZ];
    char err_buf[BUFSIZ];
    char* src = exec("top -n 1 -b | grep -E 'Mem: [0-9]+.+ used,'");  
    const char* pattern = "([0-9]+)K used, ([0-9]+)K free";
    regex_t preg;
    regmatch_t pmatch[10];
    if ( (res = regcomp(&preg, pattern, REG_EXTENDED)) != 0) {
        regerror(res, &preg, err_buf, BUFSIZ);
        printf("regcomp: %s\n", err_buf);
        return;
    }
    res = regexec(&preg, src, 10, pmatch, REG_NOTBOL);
    if (res == REG_NOMATCH) {
        return;
    }

    len = pmatch[1].rm_eo - pmatch[1].rm_so;
    memcpy(result, src + pmatch[1].rm_so, len);
    result[len] = 0;
    int used = atoi(result);

    len = pmatch[2].rm_eo - pmatch[2].rm_so;
    memcpy(result, src + pmatch[2].rm_so, len);
    result[len] = 0;
    int free = atoi(result);

    memoryUsage = (int)round(((double)used / (used + free)) * 100);
    printf("%d %d %d pct\n", used, free, memoryUsage);

    regfree(&preg);
}

void updateSsdData() {
    ssdUsage = atoi(exec("df -a /home | grep -Eo '[0-9]+%'"));
}

void updateNetworkConnections() {
    networkConnections = atoi(exec("netstat -an | grep ESTABLISHED | wc -l"));
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
void drawCpu(edOLED o, int pct) {
    o.pixel(0, 10); o.pixel(1, 10);
    o.pixel(0, 11);
    o.pixel(0, 12);
    o.pixel(0, 13); o.pixel(1, 13);

    o.line(3, 10, 3, 14);
    o.line(3, 10, 5, 10);
    o.line(5, 10, 5, 13);
    o.line(5, 12, 3, 12);

    o.line(7, 10, 7, 13);
    o.line(7, 13, 10, 13);
    o.line(9, 13, 9, 10);

    o.rect(11, 10, LCDWIDTH - 11, 4);
    o.rectFill(11, 10, (int)round(((double)pct/100)*(LCDWIDTH - 11)), 4);
}
void drawMemory(edOLED o, int pct, int x, int y) {
    o.line(x    , y    , x    , y + 5);
    o.line(x    , y + 1, x + 3, y + 1);
    o.line(x + 2, y + 5, x + 2, y    );

    o.line(x + 4, y    , x + 4, y + 5);
    o.pixel(x + 5, y);
    o.pixel(x + 5, y + 2);
    o.pixel(x + 5, y + 4);

    o.line(x + 7, y + 0, x + 7,  y + 5);
    o.line(x + 7, y + 1, x + 10, y + 1);
    o.line(x + 9, y + 5, x + 9,  y + 0);

    o.rect(x + 11, y, LCDWIDTH - (x + 11), 5);
    o.rectFill(x + 11, y, (int)round(((double)pct/100)*(LCDWIDTH - (x + 11))), 5);
}
void drawSsd(edOLED o, int pct, int x, int y) {
    o.line(x     , y    , x + 2, y + 0);
    o.pixel(x    , y + 1);
    o.line(x     , y + 2, x + 2, y + 2);
    o.pixel(x + 1, y + 3);
    o.line(x     , y + 4, x + 2, y + 4);

    o.line( x + 3, y    , x + 5, y + 0);
    o.pixel(x + 3, y + 1);
    o.line( x + 3, y + 2, x + 5, y + 2);
    o.pixel(x + 4, y + 3);
    o.line( x + 3, y + 4, x + 5, y + 4);

    o.line( x + 6, y + 0, x + 6, y + 5);
    o.pixel(x + 7, y + 0);
    o.line( x + 8, y + 1, x + 8, y + 4);
    o.pixel(x + 7, y + 4);

    o.rect(x + 11, y, LCDWIDTH - (x + 11), 5);
    o.rectFill(x + 11, y, (int)round(((double)pct/100)*(LCDWIDTH - (x + 11))), 5);
}
void drawNetworkConnections(edOLED o, int n, int x, int y) {
    o.setCursor(x, y);
    o.setFontType(0);
    char* msg;
    asprintf(&msg, "CON %d", n);
    o.print(msg);
    free(msg);
}
void drawExit(edOLED o) {
    o.clear(PAGE);
    o.setCursor(10,20);
    o.setFontType(0);
    o.print("goodbye");
    o.display();
    sleep(1);
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
    updateMemoryData();
    updateSsdData();
    updateNetworkConnections();
}
void render() {
    oled.clear(PAGE);
    drawWifi(oled, isWifiConnected);
    drawBattery(oled, batteryLevel);
    drawCpu(oled, cpuUsage);
    drawMemory(oled, memoryUsage, 0, 16);
    drawSsd(oled, ssdUsage, 0, 23);
    drawNetworkConnections(oled, networkConnections, 0, 30);
    oled.display();
}
int main(void) {
    signal(SIGINT, handleSignal);

    oled.begin();
    oled.clear(PAGE);
    oled.display();
    
    while(1) {
        updateAllData();
        render();
        sleep(5);
    }

    return 0;
}

