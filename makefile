OLED_DIR = deps/Edison_OLED_Block/Firmware/pong
LIBWPA = deps/wpa_supplicant-2.1/wpa_supplicant/libwpa_ctrl.a

CC = g++
CFLAGS = -c -Wall -I$(OLED_DIR)/oled -I$(OLED_DIR)/gpio
CFLAGS += -Ideps/wpa_supplicant-2.1/src/common/
LDFLAGS =
SOURCES = src/main.cpp 
SOURCES += $(OLED_DIR)/spi/spi_port_edison.cpp $(OLED_DIR)/spi/spi_device_edison.cpp 
SOURCES += $(OLED_DIR)/oled/Edison_OLED.cpp $(OLED_DIR)/gpio/gpio_edison.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = src/main

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBWPA) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean: 
	rm -rf *.o $(EXECUTABLE)
