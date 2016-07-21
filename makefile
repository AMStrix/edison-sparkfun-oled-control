OLED_DIR = deps/Edison_OLED_Block/Firmware/pong

CC = g++
CFLAGS = -c -Wall -I$(OLED_DIR)/oled -I$(OLED_DIR)/gpio
LDFLAGS =
SOURCES = src/main.cpp 
SOURCES += $(OLED_DIR)/spi/spi_port_edison.cpp $(OLED_DIR)/spi/spi_device_edison.cpp 
SOURCES += $(OLED_DIR)/oled/Edison_OLED.cpp $(OLED_DIR)/gpio/gpio_edison.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = main

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean: 
	rm -rf *.o $(EXECUTABLE)
