#include <string.h>
#include <stdint.h>

#include "bflb_mtimer.h" // NOLINT

#include "usb/usb_cdc_acm_interface.h"
#include "audio/audio.h"

extern const char* prompt;
extern clock_t boot_time;

clock_t uptime() {
    return bflb_mtimer_get_time_ms() - boot_time;
}

#define MS_IN_SEC 1000
#define SEC_IN_MIN 60
#define MIN_IN_HR 60
#define HR_IN_DAY 24

const char *echo_cmd = "echo";
const char *uptime_cmd = "uptime";
const char *audio_load_cmd = "audio-load";
const char *audio_play_cmd = "audio-play";
const char *audio_stop_cmd = "audio-stop";

void run_echo(uint8_t *cmd, uint32_t cmd_len);
void run_uptime(uint8_t *cmd, uint32_t cmd_len);
void run_audio_load(uint8_t *cmd, uint32_t cmd_len);
void run_audio_play(uint8_t *cmd, uint32_t cmd_len);
void run_audio_stop(uint8_t *cmd, uint32_t cmd_len);


void process_cmd(uint8_t *cmd, uint32_t cmd_len) {
  char* char_cmd = (char *)cmd;
  if (!strncmp(char_cmd, echo_cmd, strlen(echo_cmd))) {
    run_echo(cmd, cmd_len);
  } else if (!strncmp(char_cmd, uptime_cmd, strlen(uptime_cmd))) {
    run_uptime(cmd, cmd_len);
  } else if (!strncmp(char_cmd, audio_load_cmd, strlen(audio_load_cmd))) {
    run_audio_load(cmd, cmd_len);
  } else if (!strncmp(char_cmd, audio_play_cmd, strlen(audio_play_cmd))) {
    run_audio_play(cmd, cmd_len);
  } else if (!strncmp(char_cmd, audio_stop_cmd, strlen(audio_stop_cmd))) {
    run_audio_stop(cmd, cmd_len);
  }
}



void run_echo(uint8_t *cmd, uint32_t cmd_len) {
    raw_output(cmd_len - strlen(echo_cmd) + 1, cmd + strlen(echo_cmd));
    bflb_mtimer_delay_ms(1);
    output("\r\n");
    return;
}

// calculate uptime since boot
void run_uptime(uint8_t *cmd, uint32_t cmd_len) {
    int64_t uptime_ms = uptime();
    int64_t uptime_s = (uptime_ms / MS_IN_SEC) % SEC_IN_MIN;
    int64_t uptime_min = (uptime_ms / (MS_IN_SEC * SEC_IN_MIN)) % MIN_IN_HR;
    int64_t uptime_hr = (uptime_ms / (MS_IN_SEC * SEC_IN_MIN * MIN_IN_HR)) % HR_IN_DAY; // NOLINT
    int64_t uptime_day = (uptime_ms / (MS_IN_SEC * SEC_IN_MIN * MIN_IN_HR * HR_IN_DAY)); // NOLINT
    output("up %ld day, %ld hours, %ld minutes, %ld seconds",
        uptime_day,
        uptime_hr,
        uptime_min,
        uptime_s);
    bflb_mtimer_delay_ms(1);
    output("\r\n");
    return;
}


static uint16_t samples[8] = {
    // 0x0000, 0x30FB, 0x5A81, 0x7640, 0x7FFF, 0x7640, 0x5A81, 0x30FB,
    // 0x0000, 0xCF05, 0xA57F, 0x89C0, 0x8001, 0x89C0, 0xA57F, 0xCF05,
    // 500, 500, 500, 500, 500, 500, 500, 500
    0, 0, 0, 0, 0, 0, 0, 0
};


void run_audio_load(uint8_t *cmd, uint32_t cmd_len) {
    audio_out_load_samples(samples, 16);
    output("have %d samples after load\r\n", audio_out_get_fifo_cnt());
    bflb_mtimer_delay_ms(1);
    output("\r\n");
    return;
}


void run_audio_play(uint8_t *cmd, uint32_t cmd_len) {
    output("playing audio \r\n");
    audio_out_unmute();
    audio_out_play();
    bflb_mtimer_delay_ms(1);
    output("\r\n");
    return;
}

void run_audio_stop(uint8_t *cmd, uint32_t cmd_len) {
    output("stopping audio \r\n");
    audio_out_stop();
    bflb_mtimer_delay_ms(1);
    output("\r\n");
    return;
}

