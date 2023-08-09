#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#include "usbh_core.h"
#include "bflb_mtimer.h"
#include "board.h"

#include "usbd_core.h"
#include "usbd_cdc.h"

extern void cdc_acm_init(void);
extern void output(const char *, ...);
extern void debuglog(const char *, ...);
extern void debugwarn(const char *, ...);
extern void debugerror(const char *, ...);
extern void (*dtr_changed_ptr)(bool);
extern void (*data_received_ptr)(uint32_t, uint8_t *);
extern void raw_output(size_t, uint8_t *);

uint32_t buffer_init(char *);

static clock_t boot_time; 

clock_t uptime() {
    return bflb_mtimer_get_time_ms() - boot_time;
}

const char *prompt = "$ ";
const char *echo_cmd = "echo";
const int echo_cmd_len = 4;
const char *uptime_cmd = "uptime";
const int uptime_cmd_len = 6;

#define MS_IN_SEC 1000
#define SEC_IN_MIN 60
#define MIN_IN_HR 60
#define HR_IN_DAY 24

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t cmd_buffer[1024];
volatile bool display_prompt = false;
volatile uint32_t curr_char = 0;

void process_cmd(uint8_t *cmd, uint32_t cmd_len){
  if (strncmp((char *)cmd, echo_cmd, strlen(echo_cmd)) == 0){
    raw_output(cmd_len - echo_cmd_len + 1, cmd + echo_cmd_len);
    bflb_mtimer_delay_ms(1); /* There is a microsecond delay as well */
    output("\r\n");
    return;
  } else if (strncmp((char *)cmd, uptime_cmd, strlen(uptime_cmd)) == 0) {
    long uptime_ms = uptime();
    long uptime_s = (uptime_ms / MS_IN_SEC) % SEC_IN_MIN;
    long uptime_min = (uptime_ms / (MS_IN_SEC * SEC_IN_MIN)) % MIN_IN_HR;
    long uptime_hr = (uptime_ms / (MS_IN_SEC * SEC_IN_MIN * MIN_IN_HR)) % HR_IN_DAY;
    long uptime_day = (uptime_ms / (MS_IN_SEC * SEC_IN_MIN * MIN_IN_HR * HR_IN_DAY));
    output("up %ld day, %ld hours, %ld minutes, %ld seconds\r\n", uptime_day, uptime_hr, uptime_min, uptime_s);
    // bflb_mtimer_delay_ms(1);
    // output("\r\n");
    return;
  }
}

void data_received(uint32_t nbytes, uint8_t *bytes) {
  /* I think we're getting an SOH after our output, but not sure why exactly */
  /* This if statement is a bit fragile (e.g. it doesn't cover SOH + data) */
  /* so we may need some further processing */
  if (curr_char == 0 && nbytes == 1 && *bytes == 0x01) return;
  /* if (nbytes == 1) */
  /*   debuglog("Received the letter '%c'. curr_char %d\r\n", *bytes, curr_char); */
  if (curr_char + nbytes >= 1024) {
    /* We will overflow - bail */
    debugerror("command too long");
    output("\r\nCOMMAND TOO LONG\r\n%s", prompt);
    curr_char = 0;
    return;
  }
  /* Process new data */
  memcpy(&cmd_buffer[curr_char], bytes, nbytes);
  raw_output(nbytes, &cmd_buffer[curr_char]); /* Echo data back to console */
  if (nbytes == 1 && cmd_buffer[curr_char] == '\r') {
    /* User hit enter, process command */
    output("\r\n");
    bflb_mtimer_delay_ms(1); /* There is a microsecond delay as well */
    cmd_buffer[curr_char] = '\0';
    debuglog("Processing command '%s'\r\n", &cmd_buffer[0]);
    process_cmd(&cmd_buffer[0], curr_char - 1);
    output("%s", prompt);
    curr_char = 0;
    return;
  }
  curr_char += nbytes;
}

void dtr_changed(bool dtr) {
  if (dtr) {
    debuglog("DTR enabled: requesting prompt\r\n");
    display_prompt = true;
  }
}

int main(void) {
  board_init();

  cdc_acm_init();
  debuglog("Initialized");
  boot_time = bflb_mtimer_get_time_ms();

  dtr_changed_ptr = &dtr_changed;
  data_received_ptr = &data_received;
  while (1) {
    if (display_prompt) {
      /* We can't display directly on the dtr_enabled interrupt, must be on the
       * main loop. Without any delay, we will not see a prompt. But even 1ms
       * is enough
       */
      display_prompt = false;
      bflb_mtimer_delay_ms(1);
      output(prompt);
      debuglog("displayed prompt\r\n");
      curr_char = 0;
    }
  }
}

