#include <headers.h>
#include <character.h>
#include <storage/character.h>
#include <server_packet/char_info.h>
#include "spawn.h"

static void spawn_character(request_t *request, character_t *character)
{
        assert_valid_request(request);
        assert(character);

        packet response[SERVER_PACKET_CHAR_INFO_FULL_SIZE] = {0};
        struct List *close_characters = NULL;
        struct ListEntry *i_close_character = NULL;
        character_t *close_character = NULL;

        close_characters = list_create(request->host->alloc_memory, request->host->dealloc_memory);
        storage_character_close_to(&request->storage->character_storage, &close_characters, character, 1200);
        i_close_character = list_get_iterator(close_characters);

        printf("Player spawned\n");

        while (i_close_character) {
                close_character = list_get_value(i_close_character);
                i_close_character = list_get_next(i_close_character);

                printf("Showing new spawned player to %d\n", close_character->id);

                if (close_character->id == character->id) {
                        printf("Same character, ignoring\n");
                        continue;
                }

                server_packet_char_info(response, character, 0);
                session_encrypt_packet(close_character->session, response, response, (size_t) packet_get_size(response));
                request->host->send_response(close_character->session->socket, response, (size_t) packet_get_size(response));

                memset(response, 0, sizeof(response));
        }

        fflush(stdout);
        list_free(close_characters);
}

void state_machine_character_spawn(request_t *request, character_t *character)
{
        assert_valid_request(request);
        assert(character);
        spawn_character(request, character);
        character_update_state(character, IDLE);
}