#ifndef NANO_REVIVAL_AUDIO_H
#define NANO_REVIVAL_AUDIO_H

void audio_out_setup(void);
void audio_out_load_samples(uint16_t *samples, uint16_t num_samples);
void audio_out_play(void);
void audio_out_stop(void);
void audio_out_mute(void);
void audio_out_unmute(void);
void audio_out_set_volume(void);
int audio_out_get_fifo_cnt(void);
void audio_out_register_dma_callback(void (*f)(void *arg));

#endif /* ifndef NANO_REVIVAL_AUDIO_H */

