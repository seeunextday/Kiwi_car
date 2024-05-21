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
		
runreplay:
	./build/opendlv-perception-kiwidetection --cid=111 --name=img.argb --width=1280 --height=720 --verbose --dp=1 --minDist=7.2 --cannyThreshold=100 --accumulatorThreshold=27 --minRadius=0 --maxRadius=20

runcar:
	./build/opendlv-perception-kiwidetection --cid=111 --name=img.bgr --bgr --width=640 --height=480 --verbose --dp=1 --minDist=9 --cannyThreshold=100 --accumulatorThreshold=27 --minRadius=0 --maxRadius=20