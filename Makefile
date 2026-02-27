########################
##
##  Created: Thu Jan 08 2026
##  File: Makefile
##
########################

SRC_PATH   = src
BUILD_PATH = build
INCLUDE_PATH = include/
MEMORY_PATH = ./MemoryManagement

COMPILER = gcc
FILE_TYPE = .c

GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS   = $(shell pkg-config --libs gtk+-3.0)

SRC = \
	${SRC_PATH}/main.c \
	${SRC_PATH}/ui.c \
	${SRC_PATH}/find_largest_memory_address.c \
	${SRC_PATH}/memory.c \

INCLUDE = \
	-I./${INCLUDE_PATH} \
	${GTK_CFLAGS}

OBJ = ${SRC:${SRC_PATH}/%${FILE_TYPE}=${BUILD_PATH}/%.o}
LIB_MEMORY = ${MEMORY_PATH}/libmemoryapi.a

OBJ_FLAGS = -Wall -Wextra -Werror -std=c11 -g3 ${INCLUDE}
BIN_FLAGS = ${GTK_LIBS}

BIN_NAME = memory_viewer

all: ${BIN_NAME}

debug: OBJ_FLAGS += -DDEBUG
debug: ${BIN_NAME}

${BUILD_PATH}/%.o: ${SRC_PATH}/%${FILE_TYPE}
	@mkdir -p ${dir $@}
	${COMPILER} -MD ${OBJ_FLAGS} -c $< -o $@

${LIB_MEMORY}:
	$(MAKE) -C ${MEMORY_PATH} api

${BIN_NAME}: ${OBJ} ${LIB_MEMORY}
	${COMPILER} -o ${BIN_NAME} ${OBJ} ${LIB_MEMORY} ${BIN_FLAGS}

clean:
	rm -rf ${BUILD_PATH}

fclean: clean
	$(MAKE) -C ${MEMORY_PATH} fclean
	rm -f ${BIN_NAME}

-include ${OBJ:%.o=%.d}

.PHONY: all clean fclean debug
