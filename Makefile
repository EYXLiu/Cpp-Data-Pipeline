CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -pthread \
			-DBOOST_ERROR_CODE_HEADER_ONLY \
			-I./include \
			-I/opt/homebrew/opt/boost/include \
			-I/opt/homebrew/opt/openssl@3/include \
			-I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/opt/boost/lib \
          -L/opt/homebrew/opt/openssl@3/lib
LDLIBS = -lssl -lcrypto

EXEC = nr
SRC = main.cpp ./src/*.cpp

all: $(EXEC)

$(EXEC):
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(SRC) $(LDFLAGS) $(LDLIBS)

run: $(EXEC)
	@./$(EXEC)

clean:
	@rm -f $(EXEC)