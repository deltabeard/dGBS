#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <unistd.h>
#include "minigbs_apu.h"

static volatile uint_fast8_t running = 1;
static FILE *f;

void process_cpu(void);

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("%s FILE\n", argv[0]);
		return EXIT_FAILURE;
	}

#ifdef SOUND_SDL2
	if(SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		printf("SDL failure: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}
#endif

	if(strcmp(argv[1], "-") == 0)
		f = stdin;
	else
	{
		f = fopen(argv[1], "rb");
		assert(f != NULL);
	}

	// TODO: Use proper header format.
	uint8_t tma;
	uint8_t tac;
	assert(fread(&tma, 1, 1, f));
	assert(fread(&tac, 1, 1, f));
	audio_write(0xff06, tma);
	audio_write(0xff07, tac);
	audio_init(process_cpu);

#ifdef SOUND_SDL2
	{
		SDL_AudioDeviceID dev;
		SDL_AudioSpec want = {
			.freq = AUDIO_SAMPLE_RATE,
			.format = AUDIO_F32SYS,
			.channels = 2,
			.samples = 1024,
			.callback = audio_callback,
			.userdata = NULL
		};

		printf("Audio driver: %s\n", SDL_GetAudioDeviceName(0, 0));

		if((dev = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0)) == 0)
		{
			printf("SDL could not open audio device: %s\n",
				SDL_GetError());
			fclose(f);
			exit(EXIT_FAILURE);
		}

		SDL_PauseAudioDevice(dev, 0);
	}
#endif

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);

	while(running)
	{
#ifdef SOUND_NONE
		/* Compiling with no sound driver means we call audio_callback
		 * ourselves manuals. */
		static uint8_t stream[1024];
		int len = 1024;
		audio_callback(NULL, stream, len);
#endif
	}

#ifdef SOUND_SDL2
	SDL_Quit();
#endif

	audio_deinit();
	fclose(f);

	return EXIT_SUCCESS;
}

void process_cpu(void)
{
	int ret;
	uint8_t instr;

	puts("Beat");

	while((ret = fread(&instr, 1, 1, f)) == 1)
	{
		if((instr & (1 << 7)) == 0)
		{
			// SET
			/* Instruction is also address. */
			uint16_t address = instr + 0xFF06;
			uint8_t val;
			fread(&val, 1, 1, f);
			printf("SET %#06x %#04x\n", address, val);
			audio_write(address, val);
		}
		else
		{
			// RET
			puts("RET");
			return;
		}
	}

	if(ret == 0)
	{
		running = 0;
		puts("Reached EOF.");
	}
	else if(ret > 1)
	{
		puts("Error: Read too many bytes.");
		abort();
	}
}
