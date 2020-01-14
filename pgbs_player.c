#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <unistd.h>
#include "minigb_apu.h"

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
	audio_init();
	uint_fast16_t samples = set_tma_tac(tma, tac);
	printf("samples: %ld\n", samples);

#ifdef SOUND_SDL2
	{
		SDL_AudioDeviceID dev;
		SDL_AudioSpec want = {
			.freq = AUDIO_SAMPLE_RATE,
			.format = AUDIO_F32SYS,
			.channels = 2,
			.samples = samples,
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

#ifdef SOUND_FILE
	FILE *wav_out = fopen("out.raw", "w");
#endif

	while(running)
	{
		process_cpu();
#ifdef SOUND_NONE
		/* Compiling with no sound driver means we call audio_callback
		 * ourselves manuals. */
		static uint8_t stream[1024];
		int len = 1024;
		audio_callback(NULL, stream, len);
#elif defined(SOUND_FILE)
		static uint8_t stream[1024];
		int len = 1024;
		audio_callback(NULL, stream, len);
		fwrite(stream, 1, len, wav_out);
#else
		usleep(16 * 1000);
#endif
	}

#ifdef SOUND_SDL2
	SDL_Quit();
#elif defined(SOUND_FILE)
	fclose(wav_out);
#endif

	fclose(f);

	return EXIT_SUCCESS;
}

void process_cpu(void)
{
	int ret;
	uint8_t instr;

	while((ret = fread(&instr, 1, 1, f)) == 1)
	{
		if((instr & (1 << 7)) == 0)
		{
			// SET
			/* Instruction is also address. */
			uint16_t address = instr + 0xFF00;
			uint8_t val;

			fread(&val, 1, 1, f);
			printf("SET %#06x %#04x\n", address, val);
			assert(instr <= 0x3F);

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
