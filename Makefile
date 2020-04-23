TARGET = glearth
LIBS = -lm -ldl -pthread -lSDL2
CXX = g++
CFLAGS = -Wall -O3 -Iinclude $(sdl2-config --cflags)

.PHONY: default all clean run

default: $(TARGET)
all: default

BINDIR = bin
SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:src/%.cpp=$(BINDIR)/%.o)
HEADERS = $(wildcard src/*.h)

$(BINDIR)/%.o: src/%.cpp $(HEADERS) | $(BINDIR)
	$(CXX) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(BINDIR):
	mkdir -p $(BINDIR)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f $(BINDIR)/*.o
	-rm -f $(TARGET)

run: $(TARGET)
	-./$(TARGET)
