# Straight forward Makefile to compile all examples in a row

INCDIR=-I./common
LIBS=-lGLESv2 -lEGL -lm -lX11

COMMONSRC=./common/esShader.c    \
          ./common/esTransform.c \
          ./common/esShapes.c    \
          ./common/esUtil.c
COMMONHRD=esUtil.h

TekenBeesten=./src/TekenBeesten.c

default: all

all: ./src/TekenBeesten 

clean:
	find . -name "Teken*.o" | xargs rm -f

./src/TekenBeesten: ${COMMONSRC} ${COMMONHDR} ${TekenBeesten}
	gcc ${COMMONSRC} ${TekenBeesten} -o ./$@ ${INCDIR} ${LIBS}
