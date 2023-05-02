#ifndef GAME_SESSION_H
#define GAME_SESSION_H

#include "game_server.h"
#include "coroutine.h"

struct crypt_key {
    byte buf[8];
};

struct game_session {
    u32 id;
    int active;
    int closed;
    size_t read;
    struct packet request;
    struct packet responses[128];
    size_t responses_head;
    size_t responses_count;
    struct username username;
    struct crypt_key encrypt_key;
    struct crypt_key decrypt_key;
    int conn_encrypted;
    struct coroutine state;
    struct l2_character *character;
};

void game_session_decrypt_packet(struct game_session *session, struct packet *packet);
void game_session_encrypt_packet(struct game_session *session, struct packet *src);
struct packet *game_session_get_free_response(struct game_session *session);

#endif
