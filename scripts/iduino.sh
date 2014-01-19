cp makefiles/iduino_nano/Makefile .
cp src/driver/* src/
cp src/examples/hello.c src/
make clean && make all && sudo make flash PORT=/dev/ttyUSB0
rm src/*.*
rm Makefile
