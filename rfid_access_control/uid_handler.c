#include <stdio.h>
#include <string.h>

int main() {
    char uid[20] = "04A1BC23D5";
    char cmd[100];

    snprintf(cmd, sizeof(cmd), "python3 -c \"import zmq; s=zmq.Context().socket(zmq.PUB); s.bind('tcp://*:5555'); s.send_string('rfid_auth %s')\"", uid);
    system(cmd);

    printf("[UID 전송] ✅ UID: %s\n", uid);
    return 0;
}
