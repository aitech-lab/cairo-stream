#!/bin/bash

./drawing |             \
ffmpeg                  \
    -hide_banner        \
    -f rawvideo         \
    -pixel_format rgb32 \
    -video_size 640x360 \
    -r 30               \
    -i -                \
    -f lavfi            \
    -i anullsrc         \
    -c:v libx264        \
    -b:v 750k           \
    -crf 23             \
    -preset fast        \
    -pix_fmt yuv420p    \
    -s 640x360          \
    -r 30               \
    -g 30               \
    -threads 0          \
    -f flv              \
    rtmp://a.rtmp.youtube.com/live2/8cx1-vv63-peqz-3y6a
