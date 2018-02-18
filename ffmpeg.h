#pragma once

#include <stdio.h>

FILE* ffmpeg_in_pipe (int w, int h, int seek, int length, unsigned char* file);
FILE* ffmpeg_out_pipe(int w, int h, unsigned char* key );
