#include "bflb_audac.h"
#include "bflb_gpio.h"
#include "bflb_dma.h"
#include "bflb_l1c.h"

#include "bl616_glb.h"
#include "board.h"

#include "audio.h"

struct bflb_device_s *audac_dma_hd;
struct bflb_device_s *audac_hd;
static struct bflb_dma_channel_lli_pool_s lli_pool[10];

void audio_dma_callback(void *arg)
{
    static uint16_t num = 0;
    num++;
    printf("scyle_n:%d\r\n", num);
}

void audac_gpio_init(void)
{
    struct bflb_device_s *gpio;

    gpio = bflb_device_get_by_name("gpio");

    /* audac pwm output mode */
    bflb_gpio_init(gpio, GPIO_PIN_14, GPIO_FUNC_AUDAC_PWM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);
    bflb_gpio_init(gpio, GPIO_PIN_15, GPIO_FUNC_AUDAC_PWM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);

    // bflb_gpio_init(gpio, GPIO_PIN_22, GPIO_FUNC_AUDAC_PWM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);
    // bflb_gpio_init(gpio, GPIO_PIN_23, GPIO_FUNC_AUDAC_PWM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);

    // bflb_gpio_init(gpio, GPIO_PIN_27, GPIO_FUNC_AUDAC_PWM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);
    // bflb_gpio_init(gpio, GPIO_PIN_28, GPIO_FUNC_AUDAC_PWM | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);

    /* PA enable */
    bflb_gpio_init(gpio, GPIO_PIN_10, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);
    bflb_gpio_init(gpio, GPIO_PIN_11, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);
    bflb_gpio_set(gpio, GPIO_PIN_10);
    bflb_gpio_set(gpio, GPIO_PIN_11);
}

void transfer_to_dma(uint16_t *samples, uint16_t num_samples)
{
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
    dma_lli_cnt = bflb_dma_channel_lli_reload(audac_dma_hd, lli_pool, 10, transfers, 1);
    bflb_dma_channel_lli_link_head(audac_dma_hd, lli_pool, dma_lli_cnt);
}

/* audio dac init */
static void audac_init(void)
{
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
    bflb_audac_feature_control(audac_hd, AUDAC_CMD_SET_VOLUME_VAL, (size_t)(-15 * 2));
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
