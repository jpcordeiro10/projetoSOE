CXX = g++
CXXFLAGS = -std=c++14 -I/usr/local/include $(shell pkg-config --cflags opencv4)
LDFLAGS = -lTgBot -lboost_system -lssl -lcrypto -lpthread -lzbar $(shell pkg-config --libs opencv4) 

SRC = projeto_soe.cpp
OBJ = $(SRC:.cpp=.o)
EXECUTABLE = projeto_soe

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(EXECUTABLE)

