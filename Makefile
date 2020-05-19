RM      := rm -f
CFLAGS  := -O3 -Wall -Isrc -I/usr/include/ffmpeg
LIBS    := -lavformat -lavutil -lavcodec -lswscale
LDFLAGS :=
INSTALL ?= install
PREFIX  ?= /usr/local

COMMON_OBJECTS     := build/validation.o build/png.o
MEDIASTAT_OBJECTS  := build/stat.o
MEDIATHUMB_OBJECTS := build/thumb.o

# Phony rules
.PHONY: all mediastat clean

all: mediastat mediathumb

install: all
	$(INSTALL) build/mediastat $(PREFIX)/bin/mediastat
	$(INSTALL) build/mediathumb $(PREFIX)/bin/mediathumb

uninstall:
	$(RM) -rf $(PREFIX)/bin/mediastat
	$(RM) -rf $(PREFIX)/bin/mediathumb

mediastat: build/mediastat

mediathumb: build/mediathumb

clean:
	$(RM) $(COMMON_OBJECTS) $(MEDIASTAT_OBJECTS)$(MEDIATHUMB_OBJECTS)

# Build rules
build/mediastat: $(COMMON_OBJECTS) $(MEDIASTAT_OBJECTS)
	$(CC) $^ $(LDFLAGS) $(LIBS) -o $@

build/mediathumb: $(COMMON_OBJECTS) $(MEDIATHUMB_OBJECTS)
	$(CC) $^ $(LDFLAGS) $(LIBS) -o $@

build/%.o: src/%.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

# Declare dependencies
-include $(COMMON_OBJECTS:.o=.d)
-include $(MEDIASTAT_OBJECTS:.o=.d)
