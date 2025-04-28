SRC = path.c
CC ?= gcc
CFLAGS ?= -Wall -Wextra -Werror -O2
NAME ?= path
OBJ = $(NAME).o

ifeq ($(OS), Windows_NT)
	EXEC  = $(NAME).exe
	CFLAGS += -D WIN32
	DLIM = "\\"
	INSTALL_DIR ?= C:\Program Files
	RM = del
	IGNORE = 2> NUL
else
	EXEC  = $(NAME)
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

