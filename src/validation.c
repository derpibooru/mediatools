#include <libavformat/avformat.h>
#include <stdbool.h>
#include <string.h>

#include "validation.h"

const AVRational t_1h = { 3600, 1 };
const AVRational t_0h  = { 0, 1 };

int mediatools_validate_video(AVFormatContext *format)
{
    uint64_t num_vstreams = 0;
    uint64_t num_astreams = 0;
    int64_t vstream_idx = -1;
    int64_t astream_idx = -1;

    for (size_t i = 0; i < format->nb_streams; ++i) {
        if (format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            num_vstreams++;
            vstream_idx = i;
        }
        if (format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            num_astreams++;
            astream_idx = i;
        }
    }

    if (num_vstreams != 1) {
        printf("Found %lu video streams (must be 1)\n", num_vstreams);
        return false;
    }

    if (num_astreams > 1) {
        printf("Found %lu audio streams (must be 0 or 1)\n", num_astreams);
        return false;
    }

    AVInputFormat *iformat = format->iformat;
    AVCodecParameters *vpar = format->streams[vstream_idx]->codecpar;
    AVCodecParameters *apar = NULL;

    if (astream_idx != -1)
        apar = format->streams[astream_idx]->codecpar;

    if (strstr(iformat->name, "matroska")) {
        switch (vpar->codec_id) {
        default:
            printf("Bad video codec for WebM container (must be VP8 or VP9)\n");
            return false;
        case AV_CODEC_ID_VP8:
        case AV_CODEC_ID_VP9:
            ;
        }

        if (apar) {
            switch (apar->codec_id) {
            default:
                printf("Bad audio codec for WebM container (must be Opus or Vorbis)\n");
                return false;
            case AV_CODEC_ID_VORBIS:
            case AV_CODEC_ID_OPUS:
                ;
            }
        }
    } else {
        printf("Unknown input format\n");
        return false;
    }

    if (vpar->width < 2 || vpar->width > 4096) {
        printf("Bad width %d (must be 2..4096)\n", vpar->width);
        return false;
    }

    if (vpar->height < 2 || vpar->height > 4096) {
        printf("Bad height %d (must be 2..4096)\n", vpar->height);
        return false;
    }

    return true;
}

int mediatools_validate_duration(AVRational dur)
{
    if (av_cmp_q(dur, t_0h) < 0 || av_cmp_q(dur, t_1h) > 0) {
        printf("Bad duration (must be 0..1 hour)\n");
        return false;
    }

    return true;
}
