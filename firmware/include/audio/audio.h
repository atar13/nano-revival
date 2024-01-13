#ifndef FIRMWARE_INCLUDE_AUDIO_AUDIO_H_
#define FIRMWARE_INCLUDE_AUDIO_AUDIO_H_

typedef struct {
    uint16_t* samples;
    uint32_t num_samples;
    uint32_t sampling_rate;
} Sound;

Sound* new_sound(uint32_t num_samples);
void free_sound(Sound* s);

void audio_out_setup(void);
void audio_out_load_samples(uint16_t *samples, uint16_t num_samples);
void audio_out_play(void);
void audio_out_stop(void);
void audio_out_mute(void);
void audio_out_unmute(void);
void audio_out_set_volume(void);
int audio_out_get_fifo_cnt(void);
void audio_out_register_dma_callback(void (*f)(void *arg));

Sound* oscillating_sound(uint16_t freq_hz);
Sound* fp_oscillating_sound(uint16_t freq_hz);

#endif  // FIRMWARE_INCLUDE_AUDIO_AUDIO_H_
