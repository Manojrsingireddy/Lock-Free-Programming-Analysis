CC = g++
CC_OPTIONS = -c -Wall -pthread -O3
RM = /bin/rm -f
SRC_DIR = src
BUILD_DIR = build
SRC = stack.cc hashtable.cc BST.cc
OBJS = ${SRC:%.cc=${BUILD_DIR}/%.o}
MAIN = run

all: ${MAIN}

build/%.o: ${SRC_DIR}/%.cc ${SRC_DIR}/%.h
	${CC} -o $@ ${CC_OPTIONS} $<

${MAIN}: ${OBJS}
	${CC} -o $@ ${SRC_DIR}/$@.cc ${OBJS} -O3

clean:
	${RM} build/* ${MAIN}
