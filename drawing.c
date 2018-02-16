#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <cairo/cairo.h>
#include <pthread.h>
#include <signal.h>
#include <math.h>

FILE* fo;
pthread_t tid;
cairo_surface_t* surface;
cairo_t* cr;
unsigned char* data; 

int W = 640;
int H = 360;

void init(){
    fo = fdopen(dup(fileno(stdout)), "wb"); 

    surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, 640, 360);
    cr = cairo_create(surface);
    data = cairo_image_surface_get_data(surface);
};

void cleanup(){
    fclose(fo);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
};

double fps = 0.0, tt = 0.0, rt=0.0;
void draw() {
    static int n=0;
    static int x=0;
    static int dx=1;
    static int y=0;
    static int dy=1;

    cairo_set_source_rgba (cr, 0.1, 0.1, 0.1, 0.4);
    cairo_paint(cr);
    
    cairo_select_font_face(
        cr, "serif",
        CAIRO_FONT_SLANT_NORMAL,
        CAIRO_FONT_WEIGHT_BOLD);
    
    cairo_set_font_size(cr, 24.0);
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    cairo_move_to(cr, x, y);
    char buf[128];
    sprintf(buf, "%d %.2ffps %.1fms", n++, fps, rt*1000);
    cairo_show_text(cr, buf);

    for(int i=0; i<5; i++) {   
        cairo_move_to(cr, rand()%W, rand()%H);
        cairo_line_to(cr, rand()%W, rand()%H);
        cairo_stroke(cr);
    }

    x+=dx; y+=dy;
    if(x>W || x<0) dx*=-1;
    if(y>H || y<0) dy*=-1;
   
};
double target_fps = 30.08;
struct timespec 
    delay  = {0, 30*1000000L},
    tstart = {0, 0}, 
    tend   = {0, 0};
int done = 0;
void* thread(void* arg) {
    while(!done) {

        clock_gettime(CLOCK_MONOTONIC, &tstart);
        {
            draw();
            fwrite(data, 1, W*H*4, fo);
        }
        clock_gettime(CLOCK_MONOTONIC, &tend);

        // render time
        rt = 
            ((double)tend.tv_sec   + 1.0e-9*tend.tv_nsec  ) - 
            ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
        delay = (struct timespec)
           { tend.tv_sec  - tstart.tv_sec, 
           1000000000L/target_fps - 
           (tend.tv_nsec - tstart.tv_nsec) };
        nanosleep(&delay, NULL);
        clock_gettime(CLOCK_MONOTONIC, &tend);

        // total time
        tt  = 
            ((double)tend.tv_sec   + 1.0e-9*tend.tv_nsec  ) - 
            ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
        fps = 1.0/tt;
    }
}

void sigint(int dummy) {
    fprintf(stderr, "CTRL-C");
    done = 1;
}

int main() {

    signal(SIGINT, sigint); 

    init();

    pthread_create(&tid, NULL, &thread, NULL);
    pthread_exit(0);

    cleanup();

    return 0;
}
