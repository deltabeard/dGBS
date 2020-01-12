CFLAGS += -fsanitize=address -fsanitize=undefined
LDLIBS += -lm

SOUND ?= SDL2
ifeq ($(SOUND),SDL2)
	CFLAGS += $(shell sdl2-config --cflags) -D SOUND_SDL2
	LDLIBS += $(shell sdl2-config --libs)
else
	CFLAGS += -D SOUND_NONE
endif

.PHONY: check
all: check pgbs_player
pgbs_player: pgbs_player.c minigbs_apu.c
check:
ifneq ($(SOUND),SDL2)
	$(info Compiling without audio playback.)
endif
