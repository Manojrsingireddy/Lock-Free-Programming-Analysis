CC = g++
CC_OPTIONS = -Wall -c -O3
RM = /bin/rm -f
SRC_DIR = src
SRC = stack.cc
HEADERS = stack.h
OBJS = ${SRC:%.cc=%.o}
MAIN = run

all: ${MAIN}

build/%.o: ${SRC_DIR}/%.cc ${SRC_DIR}/%.h
	${CC} -o $@ ${CC_OPTIONS} $<

${MAIN}: build/${OBJS}
	${CC} -o $@ ${SRC_DIR}/$@.cc build/${OBJS}

clean:
	${RM} build/* ${MAIN}
