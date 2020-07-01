ifdef DEBUG
	CFLAGS := -fsanitize=address,undefined -O0 -g3
else
	CFLAGS := -O2 -g1
endif

SOUND := SDL2
ifeq ($(SOUND),SDL2)
	CFLAGS += -D SOUND_SDL2
else ifeq ($(SOUND),FILE)
	CFLAGS += -D SOUND_FILE
else
	CFLAGS += -D SOUND_NONE
endif

override LDLIBS += $(shell sdl2-config --libs) -lm
override CFLAGS += $(shell sdl2-config --cflags) -std=c99 -pedantic

all: dgbs_player
dgbs_player: dgbs_player.o minigbs_apu.o
