#############################################################################
#
# Makefile for librf24 on Raspberry Pi
#
# License: GPL (General Public License)
# Author:  gnulnulf <arco@appeltaart.mine.nu>
# Date:    2013/02/07 (version 1.0)
#
# Description:
# ------------
# use make all and mak install to install the library
# You can change the install directory by editing the LIBDIR line
#
INSTALL_DIR=/usr/local/bin
PROGRAM_NAME=central

# The recommended compiler flags for the Raspberry Pi
CCFLAGS=-Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s

all: central

central: main.o RF24.o RF24Network.o gpio.o spi.o compatibility.o
	g++ ${CCFLAGS} -lcurl -o ${PROGRAM_NAME} compatibility.o gpio.o spi.o RF24.o main.o

main.o: main.cpp
	g++ -Wall ${CCFLAGS} -c main.cpp

RF24.o: RF24.cpp
	g++ -Wall ${CCFLAGS} -c RF24.cpp

RF24Network.o: RF24Network.cpp
	g++ -Wall ${CCFLAGS} -c RF24Network.cpp

gpio.o: gpio.cpp
	g++ -Wall ${CCFLAGS} -c gpio.cpp

spi.o: spi.cpp
	g++ -Wall ${CCFLAGS} -c spi.cpp

compatibility.o: compatibility.c
	gcc -Wall ${CCFLAGS} -c compatibility.c

clean:
	rm -rf *o ${PROGRAM_NAME}

install:
	cp ${PROGRAM_NAME} ${INSTALL_DIR}/
