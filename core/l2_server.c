#ifndef L2AUTH_L2_SERVER_C
#define L2AUTH_L2_SERVER_C

#include <assert.h>
#include <stdlib.h>
#include <core/l2_client.h>
#include <core/l2_socket.h>
#include <socket_strategy/socket_strategy_linux.h>
#include <core/l2_server.h>

struct L2Server {
        struct L2Socket socket;
        struct L2SocketStrategy socket_strategy;
        size_t accepted_clients;

        /*
         * (franco.montenegro)
         * I'm sure there is a better way of pre-allocating
         * a bunch of structs with unkown type, but for the time
         * being, this will do
         */
        void* clients;
};

void l2_server_free(struct L2Server* server)
{
        assert(server);
        if (server->clients) free(server->clients);
        free(server);
}

struct L2Server* l2_server_create(size_t max_players)
{
        struct L2Server* server = calloc(1, sizeof(struct L2Server));

        if (server == NULL) {
                return NULL;
        }

        server->accepted_clients = 0;
        server->clients = calloc(max_players, l2_client_struct_size());

        if (server->clients == NULL) {
                l2_server_free(server);
                return NULL;
        }

        return server;
}

void l2_server_listen(struct L2Server* server, unsigned short port)
{
        assert(server);
        /*
         * (franco.montenegro)
         * I don't like how we are forcing sockets to be
         * linux, why does the server knows about this?
         */
        socket_strategy_linux(&server->socket_strategy);
        l2_socket_connect(&server->socket, &server->socket_strategy);
        l2_socket_bind(&server->socket, port);
        l2_socket_listen(&server->socket);
}

struct L2Client* l2_server_get_client
(
        struct L2Server* server,
        size_t index
)
{
        assert(server);
        return server->clients + (index * l2_client_struct_size());
}

void l2_server_accept_client
(
        struct L2Server* server,
        l2_server_accept_conn_handler handler
)
{
        assert(server);
        assert(handler);

        struct L2Client* client = l2_server_get_client(
                server,
                server->accepted_clients
        );

        l2_client_init(client);
        l2_client_accept(client, &server->socket);

        server->accepted_clients += 1;
        handler(server, client);
        server->accepted_clients -= 1;
        l2_client_close(client);
}

void l2_server_start(
        struct L2Server* server,
        unsigned short port,
        l2_server_accept_conn_handler handler
)
{
        assert(server);
        assert(handler);
        l2_server_listen(server, port);
        while(1) l2_server_accept_client(server, handler);
}

#endif