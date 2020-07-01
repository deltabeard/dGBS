/**
 * Copyright (c) 2020 Mahyar Koshkouei <mk@deltabeard.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#ifdef SOUND_SDL2
#include <SDL2/SDL.h>
#endif

#include "minigbs_apu.h"

#define STREAM_SIZE 4096

static volatile uint_fast8_t running = 1;
static FILE *f = NULL;

void process_cpu(void);

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;

	if(argc != 2)
	{
		fprintf(stderr, "%s FILE\n", argv[0]);
		goto err;
	}

#ifdef SOUND_SDL2
	if(SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		fprintf(stderr, "SDL failure: %s\n", SDL_GetError());
		goto err;
	}
#endif

	if(strcmp(argv[1], "-") == 0)
	{
		f = stdin;
	}
	else
	{
		f = fopen(argv[1], "rb");
		if(f == NULL)
		{
			const char *errstr = strerror(errno);
			fprintf(stderr, "Unable to open input file: %s\n",
				errstr);
			goto err;
		}
	}

	// TODO: Use proper header format.
	uint8_t tma;
	uint8_t tac;
	if(fread(&tma, 1, sizeof(tma), f) != sizeof(tma) ||
		fread(&tac, 1, 1, f) != sizeof(tac))
	{
		fprintf(stderr, "Unable to read timing information.\n");
		goto err;
	}
	audio_write(0x06, tma);
	audio_write(0x07, tac);
	audio_init(process_cpu);

#ifdef SOUND_SDL2
	{
		SDL_AudioDeviceID dev;
		SDL_AudioSpec want = {
			.freq = AUDIO_SAMPLE_RATE,
			.format = AUDIO_F32SYS,
			.channels = 2,
			.samples = STREAM_SIZE,
			.callback = audio_callback,
			.userdata = NULL
		};

		fprintf(stdout, "Audio driver: %s\n", SDL_GetAudioDeviceName(0, 0));

		if((dev = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0)) == 0)
		{
			fprintf(stderr, "SDL could not open audio device: %s\n",
				SDL_GetError());
			goto err;
		}

		SDL_PauseAudioDevice(dev, 0);
	}
#endif

	fflush(stdout);

#ifdef SOUND_FILE
	FILE *wav_out = fopen("out.raw", "wb");
#endif

	while(running)
	{
#ifdef SOUND_NONE
		/* Compiling with no sound driver means we call audio_callback
		 * ourselves manually. */
		static uint8_t stream[STREAM_SIZE];
		audio_callback(NULL, stream, sizeof(stream));
#elif defined(SOUND_FILE)
		static uint8_t stream[STREAM_SIZE];
		audio_callback(NULL, stream, sizeof(stream));
		fwrite(stream, 1, sizeof(stream), wav_out);
#elif defined(SOUND_SDL2)
		SDL_Delay(20);
		if(SDL_QuitRequested())
		{
			fprintf(stdout, "Stopping playback.\n");
			break;
		}
#else
#error "No audio output defined"
#endif
	}

#ifdef SOUND_SDL2
	SDL_Quit();
#elif defined(SOUND_FILE)
	fclose(wav_out);
#endif

	audio_deinit();
	ret = EXIT_SUCCESS;
err:
	if(f != NULL)
		fclose(f);

	return ret;
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
			uint16_t address = instr;
			uint8_t val;

			fread(&val, 1, 1, f);
			printf("SET %#06x %#04x\n", address + 0xFF00, val);
			assert(address <= 0x3F);

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
