SRC=$(wildcard src/*.cpp)
INC=$(wildcard src/*.hpp)
BUILD=build

all: cmakeRule dirs

$(BUILD):
	@mkdir -p build
	
cmakeRule: $(BUILD)
	cd build && cmake ..
	
dirs: $(SRC) $(INC)
	cd build && make
	
run:
	./build/opendlv-control-kiwifollowing --cid=111 --verbose
