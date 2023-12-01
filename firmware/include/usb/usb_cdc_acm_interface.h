#ifndef CDC_ACM_H
#define CDC_ACM_H

#include "usbd_core.h"

void cdc_acm_init(void);
void data_received(uint32_t nbytes, uint8_t *bytes);
void dtr_changed(bool dtr);
void output(const char *, ...);
void debuglog(const char *, ...);
void debugwarn(const char *, ...);
void debugerror(const char *, ...);
extern void (*dtr_changed_ptr)(bool);
extern void (*data_received_ptr)(uint32_t, uint8_t *);
void raw_output(size_t, uint8_t *);

#endif
