#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <cairo/cairo.h>
#include <pthread.h>
#include <signal.h>
#include <math.h>

#include "mongoose.h"

FILE* fo;
pthread_t tid;
cairo_surface_t* surface;
cairo_t* cr;
unsigned char* data; 
char buf[500];
  
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

    char text[512];
    cairo_move_to(cr, 10, 30);
    sprintf(text, "%d %.2ffps %.1fms", n++, fps, rt*1000);
    cairo_show_text(cr, text);

    cairo_move_to(cr, x, y);
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

// WEBSOCKET 

static sig_atomic_t s_signal_received = 0;
static const char *s_http_port = "8080";
static struct mg_serve_http_opts s_http_server_opts;

static void signal_handler(int sig_num) {
  signal(sig_num, signal_handler);  // Reinstantiate signal handler
  s_signal_received = sig_num;
}

static int is_websocket(const struct mg_connection *nc) {
  return nc->flags & MG_F_IS_WEBSOCKET;
}

static void broadcast(struct mg_connection *nc, const struct mg_str msg) {
  struct mg_connection *c;
  char addr[32];
  mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr),
                      MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);

  // snprintf(buf, sizeof(buf), "%s %.*s", addr, (int) msg.len, msg.p);
  snprintf(buf, sizeof(buf),"%.*s", (int) msg.len, msg.p);
  fprintf(stderr, "%s\n", buf); /* Local echo. */
  for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) {
    // if (c == nc) continue; /* Don't send to the sender. */
    mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, buf, strlen(buf));
  }
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  switch (ev) {
    case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
      /* New websocket connection. Tell everybody. */
      broadcast(nc, mg_mk_str("++ joined"));
      break;
    }
    case MG_EV_WEBSOCKET_FRAME: {
      struct websocket_message *wm = (struct websocket_message *) ev_data;
      /* New websocket message. Tell everybody. */
      struct mg_str d = {(char *) wm->data, wm->size};
      broadcast(nc, d);
      break;
    }
    case MG_EV_HTTP_REQUEST: {
      mg_serve_http(nc, (struct http_message *) ev_data, s_http_server_opts);
      break;
    }
    case MG_EV_CLOSE: {
      /* Disconnect. Tell everybody. */
      if (is_websocket(nc)) {
        broadcast(nc, mg_mk_str("-- left"));
      }
      break;
    }
  }
}


int main() {

    signal(SIGINT , sigint); 
    signal(SIGTERM, sigint);
    
    // signal(SIGTERM, signal_handler);
    // signal(SIGINT, signal_handler);
    // setvbuf(stdout, NULL, _IOLBF, 0);
    // setvbuf(stderr, NULL, _IOLBF, 0);

    struct mg_mgr mgr;
    struct mg_connection *nc;
    mg_mgr_init(&mgr, NULL);
    nc = mg_bind(&mgr, s_http_port, ev_handler);
    mg_set_protocol_http_websocket(nc);

    s_http_server_opts.document_root = ".";  // Serve current directory
    s_http_server_opts.enable_directory_listing = "yes";

    init();

    pthread_create(&tid, NULL, &thread, NULL);
    // pthread_exit(0);

    fprintf(stderr, "Started on port %s\n", s_http_port);
    while (!done) {
        mg_mgr_poll(&mgr, 200);
    }
    mg_mgr_free(&mgr);

    cleanup();

    return 0;
}
