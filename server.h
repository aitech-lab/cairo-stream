#pragma once
typedef void (*msg_callback_t)(unsigned char* msg, size_t len);

void server_start(int* done, msg_callback_t cb);
