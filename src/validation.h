#ifndef _VALIDATION_H_DEFINED
#define _VALIDATION_H_DEFINED

#include <libavformat/avformat.h>

int mediatools_validate_video(AVFormatContext *format);
int mediatools_validate_duration(AVRational dur);

#endif
