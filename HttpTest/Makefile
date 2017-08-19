#for library
all:
	./hcpp *.hcpp
	g++ -shared -fPIC -g -std=c++11 -o out.so *.cpp commonfile/*.cpp
clean:
	rm *.hinfo *.cpp
