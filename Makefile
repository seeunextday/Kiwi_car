SRC=$(wildcard src/*.cpp)
INC=$(wildcard src/*.h)
BUILD=build

all: cmakeRule dirs

$(BUILD):
	@mkdir -p build

cmakeRule: $(BUILD)
	cd build && cmake ..

dirs: $(SRC) $(INC)
	cd build && make
