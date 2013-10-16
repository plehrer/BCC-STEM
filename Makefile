all:
	g++ -o erode_dilate erode_dilate.cpp `pkg-config --cflags --libs opencv`

clean:
	rm erode_dilate