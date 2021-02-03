#include <request.h>
#include <client_request/move.h>
#include <client_request/validate_position.h>
#include <client_request/say.h>
#include <client_request/show_map.h>
#include <client_packet/type.h>
#include <storage/character.h>
#include <server_packet/say.h>
#include <server_packet/logout.h>
#include <server_packet/restart.h>
#include <server_packet/move.h>
#include <client_request/say.h>
#include "../auth_request.h"
#include "idle.h"

/**
 * The player clicks on logout.
 * Do not confuse with losing the connection.
 */
static void logout(request_t *request, character_t *character)
{
        packet response[SERVER_PACKET_LOGOUT_FULL_SIZE] = {0};

        assert_valid_request(request);
        assert(character);

        server_packet_logout(response);
        session_encrypt_packet(request->session, response, response, (size_t) packet_get_size(response));
        request->host->send_response(request->session->socket, response, (size_t) packet_get_size(response));

        /*
         * Maybe here would be a good moment
         * to persist the character into the database.
         */
        character_update_state(character, SPAWN);

        session_update_state(request->session, PROTOCOL_VERSION);
}

/**
 * The user clicks on restart.
 * This is where we take the user back
 * to the character selection screen.
 */
static void restart(request_t *request, character_t *character)
{
        packet response[SERVER_PACKET_RESTART_FULL_SIZE] = {0};
        packet request_characters_packet[8] = {0};
        request_t request_characters = {0};

        assert_valid_request(request);
        assert(character);

        server_packet_restart(response);
        session_encrypt_packet(request->session, response, response, (size_t) packet_get_size(response));
        request->host->send_response(request->session->socket, response, (size_t) packet_get_size(response));

        character_update_state(character, SPAWN);

        /*
         * Create a dummy request to refetch all
         * characters from a session.
         */
        packet_build(request_characters_packet, (unsigned char) CLIENT_PACKET_TYPE_AUTH_REQUEST, NULL, 0);

        memcpy(&request_characters, request, sizeof(request_characters));
        request_characters.packet = request_characters_packet;

        state_machine_auth_request(&request_characters, 0);
}

/**
 * The player clicks to move to a particular
 * position.
 */
static void move(request_t *request, character_t *character)
{
        client_request_move_t parsed_request = {0};
        packet response[SERVER_PACKET_MOVE_FULL_SIZE] = {0};

        struct List *close_characters = NULL;
        struct ListEntry *i_close_character = NULL;
        character_t *close_character = NULL;

        assert_valid_request(request);
        assert(character);

        client_request_move(&parsed_request, request->packet);

        close_characters = list_create(request->host->alloc_memory, request->host->dealloc_memory);
        storage_character_close_to(&request->storage->character_storage, &close_characters, character, 1200);

        i_close_character = list_get_iterator(close_characters);

        while (i_close_character) {
                close_character = list_get_value(i_close_character);

                server_packet_move(response, character, &parsed_request.position);
                session_encrypt_packet(close_character->session, response, response, (size_t) packet_get_size(response));
                request->host->send_response(close_character->session->socket, response, (size_t) packet_get_size(response)
                );

                memset(response, 0, sizeof(response));

                i_close_character = list_get_next(i_close_character);
        }

        character_move(character, &parsed_request.position);

        list_free(close_characters);
}

/**
 * Send a message to all characters close enough
 * to the speaker.
 */
static void say(request_t *request, character_t *character)
{
        assert_valid_request(request);
        assert(character);

        packet response[SERVER_PACKET_SAY_FULL_SIZE] = {0};
        client_request_say_t say_request = {0};
        struct List *close_characters = NULL;
        struct ListEntry *i_close_character = NULL;
        character_t *close_character = NULL;

        // Hardcoded, not sure if this is the correct maximum
        char raw_message[128] = {0};
        size_t message_len = 0;

        close_characters = list_create(request->host->alloc_memory, request->host->dealloc_memory);

        client_request_say(&say_request, request->packet);
        l2_string_to_char(raw_message, say_request.message, sizeof(raw_message));
        message_len = strlen(raw_message) + 1;

        storage_character_close_to(&request->storage->character_storage, &close_characters, character, CONFIG_SHOUT_RANGE);

        i_close_character = list_get_iterator(close_characters);

        while (i_close_character) {
                close_character = list_get_value(i_close_character);

                server_packet_say(response, character, say_request.type, raw_message, message_len);
                session_encrypt_packet(close_character->session, response, response, (size_t) packet_get_size(response));
                request->host->send_response(close_character->session->socket, response, (size_t) packet_get_size(response));

                memset(response, 0, sizeof(response));

                i_close_character = list_get_next(i_close_character);
        }

        list_free(close_characters);
}

void state_machine_character_idle(request_t *request, character_t *character)
{
        assert_valid_request(request);
        assert(character);

        packet_type type = 0;
        int socket = 0;

        type = packet_get_type(request->packet);
        socket = request->session->socket;

        switch (type) {
        case CLIENT_PACKET_TYPE_MOVE_BACKWARDS_TO_LOCATION:
                move(request, character);
                break;
        case CLIENT_PACKET_TYPE_VALIDATE_POS:
                client_request_validate_position(socket, request->packet, request->session, character, request->host->send_response);
                break;
        case CLIENT_PACKET_TYPE_ACTION:
                printf("Action request -> TODO\n");
                break;
        case CLIENT_PACKET_TYPE_SAY:
                say(request, character);
                break;
        case CLIENT_PACKET_TYPE_SHOW_MAP:
                client_request_show_map(socket, request->session, request->host->send_response);
                break;
        case CLIENT_PACKET_TYPE_RESTART:
                restart(request, character);
                break;
        case CLIENT_PACKET_TYPE_LOGOUT:
                logout(request, character);
                break;
        default:
                printf("Packet %02X can't be handled by state_machine_character_idle.\n", type);
                break;
        }
}
