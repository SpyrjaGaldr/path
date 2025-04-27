SRC = path.c
OBJ = path.o
CC ?= gcc
CFLAGS ?= -Wall -Wextra -Werror -O2

ifeq ($(OS), Windows_NT)
	EXEC  ?= path.exe
	CFLAGS += -D WIN32
	DLIM = \
	INSTALL_DIR ?= C:\Program Files
	RM = del
	IGNORE = 2> NUL
else
	EXEC  ?= path
	DLIM = /
	INSTALL_DIR ?= /usr/local/bin
	RM = rm
	IGNORE = 2> /dev/null
endif

CLEAR = $(RM) -f $(OBJ)

default:
	@echo "Options:"
	@echo " make all"
	@echo " make clean"
	@echo " sudo make install"
	@echo " sudo make uninstall"

all:
	$(CC) $(CFLAGS) -o $(EXEC) $(SRC) && $(CLEAR)

install: all
	cp $(EXEC) $(INSTALL_DIR)$(DLIM)

clean:
	$(CLEAR)
	$(RM) -f $(EXEC)

uninstall: clean
	$(RM) -f $(INSTALL_DIR)$(DLIM)$(EXEC)

.PHONY: all install clean uninstall
