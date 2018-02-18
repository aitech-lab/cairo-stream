#include <stdio.h>
#include <unistd.h>

#define W 320
#define H 320
#define BUFFER_SIZE W*H*3

FILE* read_raw(char* file, int from, int length) {
    unsigned char cmd[1024];
    unsigned char* tpl = 
        "ffmpeg"
        " -hide_banner"
        " -ss %d"
        " -t %d"
        " -i %s"
        " -vf crop=%d:%d"
        " -pix_fmt rgb24"
        " -f rawvideo"
        " -"
        " 2>>log.txt";
    snprintf(cmd, 1024, tpl, from, length, file, W, H);
    FILE* fp =  popen(cmd, "r");
    if(fp == NULL) 
        fprintf(stderr, "ERR:", cmd);
    fprintf(stderr, "%s\n", cmd);
    return fp;
}


int close_raw(FILE* fp) {
    pclose(fp);
}

int main(int argc, char** argv) {

    if(argc!=2) return 1;
    
    unsigned char buffer[BUFFER_SIZE];
    FILE* fo = fdopen(dup(fileno(stdout)), "wb");

    char* file =  argv[1];
    int frame=1;
    for(int i=5; i<6; i++) {
        FILE* fp = read_raw(file, i*10, 1);
        if(fp == NULL) continue;
        int len = 0;
        int total = 0;
        while(len = fread(buffer, 1, BUFFER_SIZE, fp)) {
            fwrite(buffer, 1, BUFFER_SIZE, fo);
            fprintf(stderr, "frame %d %d %d\n", frame++, len, total+=len);
        };
        close_raw(fp);
    };
    
    fclose(fo);
}

