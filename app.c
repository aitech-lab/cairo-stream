#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <math.h>
#include <string.h>

#include "app.h"
#include "draw.h"
#include "ffmpeg.h"
#include "server.h"

pthread_t read_tid;
pthread_t write_tid;

unsigned char* key;

unsigned char* file;
unsigned char* key;


double target_fps = 30.08;
int done = 0;

void msleep(unsigned int ms) {
     nanosleep(&((struct timespec){ms/1000, ms%1000*1000000L}), NULL);
}

void* 
read_thread(void* arg) {
    while (!done) {
        FILE* in_pipe  = ffmpeg_in_pipe(w, h, rand()%100*100, 5, file); 
        while(!done) {
            
            if (read_count - write_count >= buffer_length ) {
                msleep(30); 
                continue;
            }
            size_t size = fread(&buffer[read_frame*frame_size], 1, frame_size, in_pipe); 
            if(!size) break;
            read_count++; 
            read_frame = (read_frame+1) % buffer_length;
        }
        int status = pclose(in_pipe);
        fprintf(stderr, "read end %d\n", status);
   } 
}

void*
write_thread(void* arg) {

    static double tt, rt;

    FILE* out_pipe = ffmpeg_out_pipe(w, h, key);

    while(!done) {

        if (read_count - write_count < 30 ) {
            msleep(30); 
            continue;
        }

        struct timespec start, end, delay;
        
        clock_gettime(CLOCK_MONOTONIC, &start);
        unsigned char* frame = &buffer[write_frame*frame_size];
        draw();
        fwrite(frame, 1, frame_size, out_pipe);
        clock_gettime(CLOCK_MONOTONIC, &end);

        // render time
        rt = ((double)end.tv_sec   + 1.0e-9*end.tv_nsec  ) - 
             ((double)start.tv_sec + 1.0e-9*start.tv_nsec);
        delay = (struct timespec)
           { end.tv_sec - start.tv_sec, 
           1000000000L/target_fps - 
           ( end.tv_nsec - start.tv_nsec) };
        nanosleep(&delay, NULL);

        // Calculate FPS
        clock_gettime(CLOCK_MONOTONIC, &end);
        // total time
        tt = ((double)end.tv_sec   + 1.0e-9*end.tv_nsec  ) - 
             ((double)start.tv_sec + 1.0e-9*start.tv_nsec);
        fps = 1.0/tt;
        write_count+=1;
        write_frame = (write_frame+1) % buffer_length;
    }
    pclose(out_pipe);
}


void sigint(int dummy) {
    fprintf(stderr, "CTRL-C");
    done = 1;
}

void on_message(unsigned char* msg, size_t len) {
    // set speed here
    char* str = strndup(msg, len);
    int write_step = atoi(str);
    printf("%d\n", write_step);
    free(str);
}

int main(int argc, char** argv) {
    
    file = argv[1];
    w    = atoi(argv[2]);
    h    = atoi(argv[3]);
    key  = argv[4];
    
    frame_size  = w * h * 4;
    buffer_length = 250;
    buffer_size = frame_size * buffer_length;
    buffer = calloc(buffer_size, 1);
    
    signal(SIGINT , sigint); 
    signal(SIGTERM, sigint);
    
    pthread_create(&read_tid , NULL, &read_thread , NULL);
    pthread_create(&write_tid, NULL, &write_thread, NULL);
    server_start(&done, on_message);
    fprintf(stderr, "done %d\n", done);
    return 0;
}
