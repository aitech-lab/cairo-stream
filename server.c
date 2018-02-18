#include "mongoose.h"
#include "server.h"

msg_callback_t msg_callback;

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
  char buf[512];
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
      if(msg_callback!=NULL) msg_callback(wm->data, wm->size);
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


void server_start(int* done, msg_callback_t cb) {

    msg_callback = cb;
    
    static struct mg_mgr mgr;
    struct mg_connection *nc;
    mg_mgr_init(&mgr, NULL);
    nc = mg_bind(&mgr, s_http_port, ev_handler);
    mg_set_protocol_http_websocket(nc);
    s_http_server_opts.document_root = ".";  // Serve current directory
    s_http_server_opts.enable_directory_listing = "yes";
    fprintf(stderr, "Started on port %s\n", s_http_port);
    while (!*done) {
        mg_mgr_poll(&mgr, 200);
    }
    mg_mgr_free(&mgr);
}

