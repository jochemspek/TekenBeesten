# Straight forward Makefile to compile all examples in a row

INCDIR=-I./common_osx
LIBS=-lglut -lGL -lm -lX11

COMMONSRC=./common_osx/esShader.c    \
          ./common_osx/esTransform.c \
          ./common_osx/esShapes.c    \
          ./common_osx/esUtil.c
COMMONHRD=esUtil.h

TekenBeesten=./src/TekenBeesten.c

default: all

all: ./src/TekenBeesten 

clean:
	find . -name "Teken*.o" | xargs rm -f

./src/TekenBeesten: ${COMMONSRC} ${COMMONHDR} ${TekenBeesten}
	gcc -g ${COMMONSRC} ${TekenBeesten} -o ./$@ ${INCDIR} ${LIBS}

