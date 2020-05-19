#include <libavformat/avformat.h>
#include <stdio.h>
#include "png.h"

int main(int argc, char *argv[])
{
    AVFormatContext *format = NULL;
    AVCodecContext *vctx = NULL;
    AVStream *vstream = NULL;
    AVCodec *vcodec = NULL;
    AVFrame *frame = NULL;
    AVPacket pkt;

    int found = 0;

    if (argc != 4) {
        printf("Expected an input, a time, and an output\n");
        return -1;
    }

    av_log_set_level(AV_LOG_QUIET);

    const char *input = argv[1];
    const char *output = argv[3];
    AVRational time = av_d2q(atof(argv[2]), INT_MAX);

    if (avformat_open_input(&format, input, NULL, NULL) != 0) {
        printf("Couldn't read file\n");
        return -1;
    }

    if (avformat_find_stream_info(format, NULL) < 0) {
        printf("Couldn't read file \n");
        return -1;
    }

    int vstream_idx = av_find_best_stream(format, AVMEDIA_TYPE_VIDEO, -1, -1, &vcodec, 0);
    if (vstream_idx < 0) {
        printf("Couldn't read file\n");
        return -1;
    }

    vstream = format->streams[vstream_idx];

    // Set up decoding context
    vctx = avcodec_alloc_context3(vcodec);
    if (!vctx) {
        printf("Couldn't read file\n");
        return -1;
    }

    if (avcodec_parameters_to_context(vctx, vstream->codecpar) < 0) {
        printf("Couldn't read file\n");
        return -1;
    }

    if (avcodec_open2(vctx, vcodec, NULL) < 0) {
        printf("Couldn't read file\n");
        return -1;
    }

    frame = av_frame_alloc();
    if (!frame) {
        printf("Couldn't read file\n");
        return -1;
    }

    // Loop until we get to the first video frame past the intended pts,
    // decoding all video frames along the way.

    while (!found && av_read_frame(format, &pkt) >= 0) {
        if (pkt.stream_index == vstream_idx) {
            AVRational cur_time  = { pkt.pts, vstream->time_base.den };
            AVRational next_time = { pkt.pts + pkt.duration, vstream->time_base.den };

            if (avcodec_send_packet(vctx, &pkt) != 0) {
                // Decoder returned an error
                printf("Couldn't read file\n");
                return -1;
            }

            int ret = avcodec_receive_frame(vctx, frame);

            if (ret == AVERROR(EAGAIN)) {
                // Need more data, can't receive frame yet
                continue;
            }

            if (ret != 0) {
                // Decoder returned an error
                printf("Couldn't read file\n");
                return -1;
            }

            // If this is the first frame past the requested time or the
            // current frame contains the requested time, pick this frame.

            if (av_cmp_q(cur_time, time) >= 0 || (av_cmp_q(cur_time, time) <= 0 && av_cmp_q(next_time, time) >= 0)) {
                found = 1;

                // Found the frame; write to the provided file
                if (mediatools_write_frame_to_png(frame, output) < 0) {
                    printf("Couldn't read file\n");
                    return -1;
                }
            }
        }

        av_packet_unref(&pkt);
    }

    if (!found) {
        printf("Couldn't read file\n");
        return -1;
    }

    av_frame_free(&frame);
    avcodec_free_context(&vctx);
    avformat_close_input(&format);

    return 0;
}
