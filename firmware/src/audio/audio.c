#include <math.h>

#include "bflb_audac.h" // NOLINT
#include "bflb_gpio.h" // NOLINT
#include "bflb_dma.h" // NOLINT
#include "bflb_l1c.h" // NOLINT
#include "bl616_glb.h" // NOLINT
#include "board.h" // NOLINT

#include "audio/audio.h"
#include "utils/trig.h"

const int AMPLITUDE_RANGE = 0x7FF;
const int MID_AMPLITUDE = 0x000;

struct bflb_device_s *audac_dma_hd;
struct bflb_device_s *audac_hd;
static struct bflb_dma_channel_lli_pool_s lli_pool[10];

// Allocates memory for a new Sound and its associated samples
Sound* new_sound(uint32_t num_samples) {
    Sound* s = malloc(sizeof(Sound));
    s->samples = malloc(num_samples * sizeof(uint16_t));
    return s;
}

// Frees memory of Sound and underlying samples
void free_sound(Sound* s) {
    free(s->samples);
    free(s);
}

void audio_dma_callback(void *arg) {
    static uint16_t num = 0;
    num++;
    printf("scyle_n:%d\r\n", num);
}

void audac_gpio_init(void) {
    struct bflb_device_s *gpio;

    gpio = bflb_device_get_by_name("gpio");

    // gnd is ring2 according to CTIA format https://www.headphonesty.com/2019/04/headphone-jacks-plugs-explained/#4conductor_plug_TRRS // NOLINT
    // GPIO_PIN_14 is right stereo output on ring1
    bflb_gpio_init(gpio, GPIO_PIN_14, GPIO_FUNC_AUDAC_PWM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2); // NOLINT
    // GPIO_PIN_15 is left stereo output on tip
    bflb_gpio_init(gpio, GPIO_PIN_15, GPIO_FUNC_AUDAC_PWM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2); // NOLINT

    // bflb_gpio_init(gpio, GPIO_PIN_22, GPIO_FUNC_AUDAC_PWM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2); // NOLINT
    // bflb_gpio_init(gpio, GPIO_PIN_23, GPIO_FUNC_AUDAC_PWM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2); // NOLINT
                                                                                                                         // NOLINT
    // bflb_gpio_init(gpio, GPIO_PIN_27, GPIO_FUNC_AUDAC_PWM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2); // NOLINT
    // bflb_gpio_init(gpio, GPIO_PIN_28, GPIO_FUNC_AUDAC_PWM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2); // NOLINT

    /* PA enable */
    bflb_gpio_init(gpio, GPIO_PIN_10, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0); // NOLINT
    bflb_gpio_init(gpio, GPIO_PIN_11, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0); // NOLINT
    bflb_gpio_set(gpio, GPIO_PIN_10);
    bflb_gpio_set(gpio, GPIO_PIN_11);
}

void transfer_to_dma(uint16_t *samples, uint16_t num_samples) {
    uint32_t dma_lli_cnt;
    struct bflb_dma_channel_lli_transfer_s transfers[1];
    struct bflb_dma_channel_config_s audac_dma_cfg;

    audac_dma_cfg.direction = DMA_MEMORY_TO_PERIPH;
    audac_dma_cfg.src_req = DMA_REQUEST_NONE;
    audac_dma_cfg.dst_req = DMA_REQUEST_AUDAC_TX;
    audac_dma_cfg.src_addr_inc = DMA_ADDR_INCREMENT_ENABLE;
    audac_dma_cfg.dst_addr_inc = DMA_ADDR_INCREMENT_DISABLE;
    audac_dma_cfg.src_burst_count = DMA_BURST_INCR8;
    audac_dma_cfg.dst_burst_count = DMA_BURST_INCR8;
    audac_dma_cfg.src_width = DMA_DATA_WIDTH_16BIT;
    audac_dma_cfg.dst_width = DMA_DATA_WIDTH_16BIT;

    audac_dma_hd = bflb_device_get_by_name("dma0_ch0");
    bflb_dma_channel_init(audac_dma_hd, &audac_dma_cfg);
    // bflb_dma_channel_irq_attach(audac_dma_hd, audio_dma_callback, NULL);

    transfers[0].src_addr = (uint32_t)samples;
    transfers[0].dst_addr = (uint32_t)DMA_ADDR_AUDAC_TDR;
    transfers[0].nbytes = num_samples;

    bflb_l1c_dcache_clean_all();
    // bflb_l1c_dcache_clean_range(samples, num_samples);
    // csi_dcache_clean_range(samples, num_samples);
    dma_lli_cnt = bflb_dma_channel_lli_reload(audac_dma_hd, lli_pool, 10,
        transfers, 1);
    bflb_dma_channel_lli_link_head(audac_dma_hd, lli_pool, dma_lli_cnt);
}

/* audio dac init */
static void audac_init(void) {
    struct bflb_audac_init_config_s audac_init_cfg = {
        .sampling_rate = AUDAC_SAMPLING_RATE_32K,
        .output_mode = AUDAC_OUTPUT_MODE_PWM,
        .source_channels_num = AUDAC_SOURCE_CHANNEL_DUAL,
        .mixer_mode = AUDAC_MIXER_MODE_ONLY_L,
        .data_format = AUDAC_DATA_FORMAT_16BIT,
        .fifo_threshold = 7,
    };

    struct bflb_audac_volume_config_s audac_volume_cfg = {
        .mute_ramp_en = true,
        .mute_up_ramp_rate = AUDAC_RAMP_RATE_FS_32,
        .mute_down_ramp_rate = AUDAC_RAMP_RATE_FS_8,
        .volume_update_mode = AUDAC_VOLUME_UPDATE_MODE_RAMP,
        .volume_ramp_rate = AUDAC_RAMP_RATE_FS_128,
        .volume_zero_cross_timeout = AUDAC_RAMP_RATE_FS_128,
    };

    /* clock cfg */
    GLB_Config_AUDIO_PLL_To_491P52M();
    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_AUDIO);

    /* audac init */
    audac_hd = bflb_device_get_by_name("audac");
    bflb_audac_init(audac_hd, &audac_init_cfg);
    /* Set volume value dB, arg range -191 to + 36, 0.5dB step, 
        range -95.5dB to +18dB. 
        Note that volume in dB does not scale linearly (seems to be logarithmic)
    */
    // bflb_audac_feature_control(audac_hd, AUDAC_CMD_SET_VOLUME_VAL,
    //     (size_t)(-15 * 2));
    bflb_audac_feature_control(audac_hd, AUDAC_CMD_SET_VOLUME_VAL,
        (size_t)(0 * 2));
    bflb_audac_volume_init(audac_hd, &audac_volume_cfg);
    /* audac enable dma */
    bflb_audac_link_rxdma(audac_hd, true);
}


void audio_out_setup(void) {
    audac_gpio_init();
    audac_init();
    // audio_out_stop();
    // bflb_audac_feature_control(audac_hd, AUDAC_CMD_CLEAR_TX_FIFO, 0);
}

void audio_out_load_samples(uint16_t *samples, uint16_t num_samples) {
    bflb_audac_feature_control(audac_hd, AUDAC_CMD_CLEAR_TX_FIFO, 0);
    transfer_to_dma(samples, num_samples);
    bflb_dma_channel_start(audac_dma_hd);
}

void audio_out_play(void) {
    bflb_audac_feature_control(audac_hd, AUDAC_CMD_PLAY_START, 0);
}

void audio_out_stop(void) {
    bflb_audac_feature_control(audac_hd, AUDAC_CMD_PLAY_STOP, 0);
}


void audio_out_mute(void) {
    bflb_audac_feature_control(audac_hd, AUDAC_CMD_SET_MUTE, true);
}

void audio_out_unmute(void) {
    bflb_audac_feature_control(audac_hd, AUDAC_CMD_SET_MUTE, false);
}

void audio_out_set_volume(void) {
}

int audio_out_get_fifo_cnt(void) {
    return bflb_audac_feature_control(audac_hd, AUDAC_CMD_GET_TX_FIFO_CNT, 0);
}

void audio_out_register_dma_callback(void (*f)(void *arg)) {
    bflb_dma_channel_irq_attach(audac_dma_hd, f, NULL);
}

// oscillating_sound generates a sound that oscilaltes at a
// frequency
Sound* oscillating_sound(uint16_t freq_hz) {
    const int SAMPLING_RATE = 32000;

    const int PERIOD = SAMPLING_RATE/freq_hz;
    const int TWO_CH_PERIOD = 2 * PERIOD;
    const int NUM_STEPS = TWO_CH_PERIOD / 8;
    const int STEP = AMPLITUDE_RANGE / NUM_STEPS;

    Sound* sound = new_sound(TWO_CH_PERIOD);
    sound->num_samples = TWO_CH_PERIOD;
    sound->sampling_rate = SAMPLING_RATE;

    for (int i = 0; i < TWO_CH_PERIOD - 1; i+=2) {
        int sin_lut_idx = (256 * i) / TWO_CH_PERIOD;
        // use sine look up table to avoid math.h sine function
        // and floating point operations
        uint16_t sound_amplitude = audio_out_sin_lut[sin_lut_idx];

        sound->samples[i] = sound_amplitude;
        sound->samples[i+1] = sound_amplitude;
    }

    return sound;
}


// Slower version that generates a sound of a single note using math.h sine
// function and floating point math
Sound* fp_oscillating_sound(uint16_t freq_hz) {
    const int SAMPLING_RATE = 32000;
    const int AMPLITUDE = 0x7FF;
    const int MID = 0;

    const int PERIOD = SAMPLING_RATE/freq_hz;
    const int TWO_CH_PERIOD = 2 * PERIOD;

    Sound* sound = new_sound(TWO_CH_PERIOD);
    sound->num_samples = TWO_CH_PERIOD;
    sound->sampling_rate = SAMPLING_RATE;

    for (int i = 0; i < TWO_CH_PERIOD - 1; i+=2) {
        uint16_t sound_amplitude = (AMPLITUDE *
            sin((double)(i % (TWO_CH_PERIOD))/((double)TWO_CH_PERIOD) *
                2 * M_PI))
            + MID;
        sound->samples[i] = sound_amplitude;
        sound->samples[i+1] = sound_amplitude;
    }

    return sound;
}
