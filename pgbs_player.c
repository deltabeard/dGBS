#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <unistd.h>
#include "audio.h"

static volatile uint_fast8_t running = 1;
static FILE *f;

int main(int argc, char *argv[])
{
	SDL_AudioDeviceID dev;

	if(argc != 2)
	{
		printf("%s FILE\n", argv[0]);
		return EXIT_FAILURE;
	}

	if(SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		printf("SDL failure: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	if(strcmp(argv[1], "-") == 0)
		f = stdin;
	else
	{
		f = fopen(argv[1], "rb");
		assert(f != NULL);
	}

	{
		SDL_AudioSpec want, have;
		want.freq = AUDIO_SAMPLE_RATE;
		want.format   = AUDIO_F32SYS,
		     want.channels = 2;
		want.samples = 1024;
		want.callback = audio_callback;
		want.userdata = NULL;
		printf("Audio driver: %s\n", SDL_GetAudioDeviceName(0, 0));

		if((dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0)) == 0)
		{
			printf("SDL could not open audio device: %s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}

		uint8_t tma;
		uint8_t tac;
		assert(fread(&tma, 1, 1, f));
		assert(fread(&tac, 1, 1, f));
		audio_write(0xff06, tma);
		audio_write(0xff07, tac);
		audio_init();
		SDL_PauseAudioDevice(dev, 0);
	}

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);

	while(running);

	SDL_Quit();
	fclose(f);
	puts("Finished");
	return EXIT_SUCCESS;
}

void process_cpu(void)
{
	static int timeout = 0;
	int ret;
	uint8_t instr;
	printf("process ");
	timeout++;

	while((ret = fread(&instr, 1, 1, f)) == 1)
	{
		if((instr & (1 << 7)) == 0)
		{
			// SET
			uint16_t address = 0;
			uint8_t val;
			fread(&val, 1, 1, f);
			/* Instruction contains address. */
			address = instr + 0xFF06;
			printf("SET %#06x %#04x\n", address, val);
			audio_write(address, val);
		}
		else
		{
			// RET
			// Pause for required length given tma tac.
			printf("RET\n");
			timeout = 0;
			return;
		}
	}

	if(ret == 0)
		printf("EOF\n");
	else if(ret > 1)
		printf("Read too many bytes\n");

	if(timeout == 20)
		running = 0;
}
