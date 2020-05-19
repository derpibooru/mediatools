#ifndef _PNG_H_DEFINED
#define _PNG_H_DEFINED

#include <libavformat/avformat.h>

int mediatools_write_frame_to_png(AVFrame *in_frame, const char *path);

#endif
