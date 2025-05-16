#include <stdio.h>
#include <zmq.h>
#include <string.h>

int main() {
    void *context = zmq_ctx_new();
    void *subscriber = zmq_socket(context, ZMQ_SUB);
    zmq_connect(subscriber, "tcp://localhost:5556");
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "auth_result", 11);

    char buffer[256];
    while (1) {
        zmq_recv(subscriber, buffer, 256, 0);
        buffer[255] = '\0';
        printf("[LCD] 📟 %s\n", buffer);
    }

    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    return 0;
}
