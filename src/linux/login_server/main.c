#include <assert.h>
#include <stdio.h>
#include "../os_io.c"
#include "../../ls_lib.c"

#include <openssl/evp.h>

static void _send_response(struct os_io *socket, void *buf, size_t n)
{
        if (!socket) {
                return;
        }
        os_io_write(socket, buf, n);
}

static void _disconnect(struct os_io *socket)
{
        if (!socket) {
                return;
        }
        ls_lib_disconnect(socket);
        os_io_close(socket);
}

static void
on_request(struct os_io *socket, os_io_event_t event, void *buf, size_t n)
{
        if (!socket) {
                return;
        }

        switch (event) {
        case OS_IO_SOCKET_CONNECTION:
                printf("new connection.\n");
                ls_lib_new_conn(socket);
                break;
        case OS_IO_SOCKET_REQUEST:
                printf("new request.\n");
                ls_lib_new_req(socket, buf, n);
                break;
        case OS_IO_SOCKET_DISCONNECTED:
                printf("disconnect.\n");
                ls_lib_disconnect(socket);
                break;
        default:
                break;
        }

        fflush(stdout);
}

int main(/* int argc, char **argv */)
{
        struct ls_lib ls = { 0 };

        struct os_io *socket = 0;

        socket = os_io_socket_create(2106, 30);

        if (!socket) {
                printf("login server socket couldn't be created.\n");
                return 1;
        }

        ls.send_response = _send_response;
        ls.disconnect    = _disconnect;

        ls_lib_load(&ls);

        if (!os_io_listen(on_request)) {
                printf("login server request can't be handled.\n");
                return 1;
        }

        printf("login server listening for requests.\n");

        return 0;
}
