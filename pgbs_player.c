#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <unistd.h>
#include "audio.h"

int main(int argc, char *argv[])
{
	SDL_AudioDeviceID dev;

	if(argc != 2 || argv[1][0] != '-')
	{
		printf("cat file.pgbs | %s -\n", argv[0]);
		return EXIT_FAILURE;
	}

	if(SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		printf("SDL failure: %s\n", SDL_GetError());
		return EXIT_FAILURE;
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


		audio_write(0xff06, 0);
		audio_write(0xff07, 0b00000000);
		audio_init();
		SDL_PauseAudioDevice(dev, 0);
	}

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	while(1);

	SDL_Quit();
	puts("Finished");

	return EXIT_SUCCESS;
}

void process_cpu(void)
{
	uint8_t instr;
	printf("process ");
	while(read(STDIN_FILENO, &instr, 1) == 1)
	{
		printf("%#04x ", instr);
		if((instr & (1 << 7)) == 0)
		{
			printf("SET ");
			// SET
			uint16_t address = 0;
			uint8_t val;

			read(STDIN_FILENO, &val, 1);
			/* Instruction contains address. */
			address = instr + 0xFF06;

			audio_write(address, val);
		}
		else
		{
			// RET
			// Pause for required length given tma tac.
			printf("RET\n");
			return;
		}

	}
}
