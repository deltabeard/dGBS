ifdef VSCMD_VER
	CC := cl
	OBJEXT := obj
	SOUND := FILE
	RM := del
	EXEOUT := /Fe
	CFLAGS += /W3
else
	SOUND := SDL2
	OBJEXT := o
	RM := rm -f
	EXEOUT := -o
	CFLAGS += -std=c99 -pedantic -O2 -g3
endif

ifeq ($(SOUND),SDL2)
	CFLAGS += -D SOUND_SDL2 $(shell sdl2-config --cflags)
	LDLIBS += $(shell sdl2-config --libs)
else ifeq ($(SOUND),FILE)
	CFLAGS += -D SOUND_FILE
else
	CFLAGS += -D SOUND_NONE
endif

override LDLIBS += -lm

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.$(OBJEXT))

all: dgbs_player
dgbs_player: $(OBJS)
	$(CC) $(CFLAGS) $(EXEOUT)$@ $^ $(LDFLAGS)

%.obj: %.c
	$(CC) $(CFLAGS) /Fo$@ /c /TC $^

clean:
	$(RM) $(OBJS)
	$(RM) ./dgbs_player ./dgbs_player.exe
