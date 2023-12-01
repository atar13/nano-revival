#include <stdint.h>
#include <time.h>
#include <stdio.h>

#include "usbh_core.h"
#include "bflb_mtimer.h"
#include "board.h"

#include "usbd_core.h"
#include "usbd_cdc.h"

#include "usb_cdc_acm_interface.h"
#include "audio.h"

uint32_t buffer_init(char *);

clock_t boot_time; 

volatile bool display_prompt = false;
const char *prompt = "$ ";
extern volatile uint32_t curr_char;

void audio_dma_handler(void *arg) {
    output("finished audio dma transfer\r\n");
}

int main(void) {
  boot_time = bflb_mtimer_get_time_ms();
  board_init();

  cdc_acm_init();

  audio_out_setup();
  // audio_out_register_dma_callback(&audio_dma_handler);

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
      curr_char = 0;
    }
  }
}

