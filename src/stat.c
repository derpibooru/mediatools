#include <libavformat/avformat.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "validation.h"

static int64_t start_time(AVStream *stream)
{
    if (stream->start_time == AV_NOPTS_VALUE) {
        return 0;
    }

    return stream->start_time;
}

int main(int argc, char *argv[])
{
    AVFormatContext *format = NULL;
    struct stat statbuf;
    AVPacket pkt;

    if (argc != 2) {
        printf("No input specified\n");
        return -1;
    }

    av_log_set_level(AV_LOG_QUIET);

    if (stat(argv[1], &statbuf) != 0) {
        printf("Couldn't read file\n");
        return -1;
    }

    if (avformat_open_input(&format, argv[1], NULL, NULL) != 0) {
        printf("Couldn't read file\n");
        return -1;
    }

    if (avformat_find_stream_info(format, NULL) < 0) {
        printf("Couldn't read file \n");
        return -1;
    }

    if (!mediatools_validate_video(format)) {
        // Error is printed by validation function
        return -1;
    }

    int vstream_idx = av_find_best_stream(format, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (vstream_idx < 0) {
        printf("Couldn't read file\n");
        return -1;
    }

    // Best effort attempt to seek to beginning of file
    av_seek_frame(format, -1, 0, AVSEEK_FLAG_BACKWARD);

    uint64_t frames = 0;
    int64_t last_pts = 0;
    int last_stream = 0;

    while (av_read_frame(format, &pkt) >= 0) {
        int64_t new_pts = pkt.pts + pkt.duration;

        if (last_pts < new_pts) {
            last_pts = new_pts;
            last_stream = pkt.stream_index;
        }

        if (pkt.stream_index == vstream_idx)
            ++frames;

        av_packet_unref(&pkt);
    }

    AVStream *stream = format->streams[last_stream];
    AVRational dur   = av_mul_q(av_make_q(last_pts - start_time(stream), 1), stream->time_base);

    if (!mediatools_validate_duration(dur))
        return -1;

    AVCodecParameters *vpar = format->streams[vstream_idx]->codecpar;

    printf("%ld %lu %d %d %d %d\n", statbuf.st_size, frames, vpar->width, vpar->height, dur.num, dur.den);

    avformat_close_input(&format);

    return 0;
}
