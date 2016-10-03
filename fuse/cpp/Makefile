.PHONY: all clean

all: grandet_fuse

%.o: %.cpp helper.h
	${CXX} -O4 -g -std=c++11 `pkg-config --cflags fuse protobuf hiredis` $< -o $@ -c -I/usr/include/hiredis

grandet_fuse: main.o grandet.o
	${CXX} -O4 -g -std=c++11 $^ `pkg-config --libs fuse protobuf hiredis` ../../build/proto/libproto.a -lpthread -lboost_system -o $@

clean:
	-rm *.o
	-rm grandet_fuse
