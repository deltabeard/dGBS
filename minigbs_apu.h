#include <stdint.h>

#define AUDIO_SAMPLE_RATE 48000.0f

/**
 * Fill allocated buffer "data" with "len" number of 32-bit floating point
 * samples (native endian order) in stereo interleaved format.
 */
void audio_callback(void *ptr, uint8_t *data, int len);

/**
 * Read audio register at given address "addr".
 */
uint8_t audio_read(const uint8_t addr);

/**
 * Write "val" to audio register at given address "addr".
 */
void audio_write(const uint8_t addr, const uint8_t val);

/**
 * Initialise audio driver.
 */
void audio_init(void (*process_cpu)(void));

/**
 * Frees memory used by audio driver.
 */
void audio_deinit(void);
