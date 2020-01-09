CFLAGS += $(shell sdl2-config --cflags)
LDLIBS += $(shell sdl2-config --libs)

all: pgbs_player
pgbs_player: pgbs_player.c ./audio.c
