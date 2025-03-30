#ifndef _CIAS_VOICE_UPLOAD_H_
#define _CIAS_VOICE_UPLOAD_H_
#include "cias_common.h"

#if VOICE_UPLOAD_BY_UART
void voice_upload_enable(void);
void voice_upload_disable(void);
int voice_upload_task_init(void);
int upload_raw_audio_data(short *src_addr, int frame_len_size);
void cias_speex_destroy(void);
#endif

#endif //_CIAS_VOICE_UPLOAD_H_