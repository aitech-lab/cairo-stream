#include <stdio.h>
#include <stdlib.h>

FILE* 
ffmpeg_in_pipe(
    int w, int h,
    int seek, int length,
    unsigned char* file) {
        
    unsigned char cmd[1024];
    unsigned char* tpl = 
        "ffmpeg"
        " -hide_banner"         // disable deubg
        " -ss %4$d"             // seek
        " -t %5$d"              // length
        " -i videos/lia_00%1$d.mp4"              // input file
        " -vf crop=%2$d:%3$d"   // crop filter
        " -pix_fmt rgb32"       // output format
        " -f rawvideo"          // container format
        " -"                    // pipe output
        " 2>>in.log";           // stderr to file
    //snprintf(cmd, 1024, tpl, file, w, h, seek, length);
    snprintf(cmd, 1024, tpl, rand()%9+1, w, h, seek, length);
    FILE* fp =  popen(cmd, "r");
    if(fp == NULL) 
        fprintf(stderr, "ERR:");
    fprintf(stderr, "%s\n", cmd);
    return fp;
}


FILE* 
ffmpeg_out_pipe(
    int w, int h, 
    unsigned char* key) {

    unsigned char cmd[1024];
    unsigned char* tpl = 
        "ffmpeg"
        " -hide_banner"          // disable debug
        " -f rawvideo"           // container format
        " -pixel_format rgb32"   // pixel format
        " -video_size %1$dx%2$d" // frame size
        " -r 30"                 // frame rate
        " -i -"                  // input pipe

        " -f lavfi"              // silent sound generation
        " -i anullsrc"           //

        " -c:v libx264"          // video codec     
        " -b:v 150k"             // bitrate
        " -crf 23"               // constant rate factor 0-51 
        " -preset fast"          // preset
        " -pix_fmt yuv420p"      //
        " -s %1$dx%2$d"          // scale
        " -r 30"                 // output bitrate
        " -g 30"                 // keyframes
        " -threads 0"            // threads auto
        " -f flv"                // container
        " rtmp://a.rtmp.youtube.com/live2/%3$s"
        " 2>>out.log";

    snprintf(cmd, 1024, tpl, w, h, key);
    FILE* fp =  popen(cmd, "w");
    if(fp == NULL) 
        fprintf(stderr, "ERR:");
    fprintf(stderr, "%s\n", cmd);
    return fp;
}

