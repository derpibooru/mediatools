RM      := rm -f
CC      := gcc
CFLAGS  := -O3 -Wall -Isrc -I/usr/include/ffmpeg
LIBS    := -lavformat -lavutil -lavcodec
LDFLAGS :=
INSTALL ?= install
PREFIX  ?= /usr/local

COMMON_OBJECTS     := build/validation.o
MEDIASTAT_OBJECTS  := build/stat.o

# Phony rules
.PHONY: all mediastat clean

all: mediastat 

install: all
	$(INSTALL) build/mediastat $(PREFIX)/bin/mediastat

uninstall:
	$(RM) -rf $(PREFIX)/bin/mediastat

mediastat: build/mediastat

clean:
	$(RM) $(COMMON_OBJECTS) $(MEDIASTAT_OBJECTS)

# Build rules
build/mediastat: $(COMMON_OBJECTS) $(MEDIASTAT_OBJECTS)
	$(CC) $^ $(LDFLAGS) $(LIBS) -o $@

build/%.o: src/%.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

# Declare dependencies
-include $(COMMON_OBJECTS:.o=.d)
-include $(MEDIASTAT_OBJECTS:.o=.d)
