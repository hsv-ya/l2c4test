#include <assert.h>
#include "include/config.h"
#include "include/util.h"
#include "include/log.h"
#include "include/packet.h"
#include "include/conn.h"
#include "include/l2_string.h"
#include "include/gs_session.h"
#include "include/gs_random_id.h"
#include "include/gs_packet_create_char_request.h"
#include "include/gs_packet_move.h"
#include "include/gs_packet_move_request.h"
#include "include/gs_packet_validate_pos.h"
#include "include/gs_packet_validate_pos_request.h"
#include "include/gs_packet_char_info.h"
#include "include/gs_packet_npc_info.h"
#include "include/gs_packet_action_request.h"
#include "include/gs_packet_target_selected.h"
#include "include/gs_character.h"

static gs_character_t *characters = 0;
static size_t *character_count    = 0;

static packet_t response[65536] = { 0 };

static int is_npc(gs_character_t *src)
{
        assert(src);
        return src->session ? 0 : 1;
}

static gs_character_t *find_by_id(u32_t id)
{
        for (size_t i = 0; i < *character_count; i += 1) {
                if (characters[i].id == id) {
                        return &characters[i];
                }
        }

        return 0;
}

static void encrypt_and_send_packet(gs_character_t *from, packet_t *packet)
{
        assert(from);
        assert(packet);

        if (is_npc(from)) {
                return;
        }

        gs_session_encrypt(from->session, packet, packet);
        conn_send_packet(from->session->socket, packet);
}

static void handle_move_request(gs_character_t *character, packet_t *packet)
{
        gs_packet_move_request_t move_request = { 0 };
        gs_packet_move_t move_response        = { 0 };

        assert(character);
        assert(packet);

        gs_packet_move_request_unpack(&move_request, packet);

        character->target_x = move_request.x;
        character->target_y = move_request.y;
        character->target_z = move_request.z;

        gs_packet_move(&move_response, character);

        for (size_t i = 0; i < *character_count; i += 1) {
                bytes_zero(response, sizeof(response));
                gs_packet_move_pack(response, &move_response);
                encrypt_and_send_packet(&characters[i], response);
        }
}

static void
handle_validate_position_request(gs_character_t *character, packet_t *packet)
{
        gs_packet_validate_pos_request_t validate_request = { 0 };
        gs_packet_validate_pos_t validate_response        = { 0 };

        assert(character);
        assert(packet);

        gs_packet_validate_pos_request_unpack(&validate_request, packet);

        // Todo: Perform basic validation.
        // If it's close enough to current location, update. If not, don't.
        character->x       = validate_request.x;
        character->y       = validate_request.y;
        character->z       = validate_request.z;
        character->heading = validate_request.heading;

        bytes_zero(response, sizeof(response));

        gs_packet_validate_pos(&validate_response, character);
        gs_packet_validate_pos_pack(response, &validate_response);

        encrypt_and_send_packet(character, response);
}

static void handle_action_request(gs_character_t *character, packet_t *packet)
{
        gs_packet_action_request_t action = { 0 };

        gs_packet_target_selected_t selected = { 0 };

        gs_character_t *target = 0;

        assert(character);
        assert(packet);

        gs_packet_action_request_unpack(&action, packet);

        target = find_by_id(action.target_id);

        if (!target) {
                log("selected target %d not found, ignoring", action.target_id);
                return;
        }

        character->target_id = action.target_id;

        selected.target_id = action.target_id;
        selected.color     = 0;

        bytes_zero(response, sizeof(response));
        gs_packet_target_selected_pack(response, &selected);
        encrypt_and_send_packet(character, response);
}

static void spawn_random_orc(void)
{
        gs_character_t orc = { 0 };

        gs_random_id(&orc.id);

        orc.x                         = -84023;
        orc.y                         = 244598;
        orc.z                         = -3730;
        orc.heading                   = 2;
        orc.m_attack_speed            = 1;
        orc.p_attack_speed            = 1;
        orc.run_speed                 = 200;
        orc.walk_speed                = 100;
        orc.movement_speed_multiplier = 1;
        orc.collision_radius          = 14;
        orc.collision_height          = 25;

        bytes_cpy_str(orc.name, "Orc", sizeof(orc.name) - 1);
        bytes_cpy_str(orc.title, "Archer", sizeof(orc.title) - 1);

        gs_character_spawn(&orc);
}

static void spawn_state(gs_character_t *character, packet_t *packet)
{
        assert(character);
        assert(packet);

        switch (packet_type(packet)) {
        case 0x01: // Move backwards.
                handle_move_request(character, packet);
                break;
        case 0x04: // Action.
                handle_action_request(character, packet);
                break;
        case 0x09: // Logout.
                log("TODO: Logout");
                break;
        case 0x38: // Say.
                log("TODO: Say");
                break;
        case 0x46: // Restart.
                log("TODO: Restart");
                spawn_random_orc();
                break;
        case 0x48: // Validate position.
                handle_validate_position_request(character, packet);
                break;
        case 0xcd: // Show map.
                log("TODO: Show map");
                break;
        default:
                log("Can't handle packet from in world state.");
                break;
        }
}

void gs_character_set(gs_character_t *src, size_t *count)
{
        characters      = src;
        character_count = count;
}

void gs_character_from_request(
        gs_character_t *dest,
        gs_packet_create_char_request_t *src)
{
        assert(dest);
        assert(src);

        l2_string_to_char(dest->name, src->name, sizeof(dest->name));

        dest->race       = src->race;
        dest->_class     = src->_class;
        dest->sex        = src->sex;
        dest->face       = src->face;
        dest->hair_color = src->hair_color;
        dest->hair_style = src->hair_style;
        dest->con        = src->con;
        dest->dex        = src->dex;
        dest->men        = src->men;
        dest->str        = src->str;
        dest->wit        = src->wit;
        dest->_int       = src->_int;

        // Talking Island
        dest->x = -83968;
        dest->y = 244634;
        dest->z = -3730;

        dest->level                     = 1;
        dest->exp                       = 50;
        dest->sp                        = 10;
        dest->hp                        = 400;
        dest->mp                        = 400;
        dest->cp                        = 200;
        dest->max_hp                    = 400;
        dest->max_mp                    = 400;
        dest->max_cp                    = 200;
        dest->p_attack                  = 42;
        dest->m_attack                  = 42;
        dest->p_def                     = 42;
        dest->m_def                     = 42;
        dest->evasion_rate              = 2;
        dest->accuracy                  = 2;
        dest->critical_hit              = 2;
        dest->run_speed                 = 120;
        dest->walk_speed                = 100;
        dest->p_attack_speed            = 20;
        dest->m_attack_speed            = 20;
        dest->movement_speed_multiplier = 1;
        dest->attack_speed_multiplier   = 1;
        dest->collision_radius          = 9;
        dest->collision_height          = 23;
        dest->max_load                  = 42;
        dest->name_color                = 0xFFFFFF;
}

void gs_character_spawn(gs_character_t *src)
{
        // Todo: maybe rename to player_info?
        static gs_packet_char_info_t char_info = { 0 };

        static gs_packet_npc_info_t npc_info = { 0 };

        assert(src);
        assert(src->id);

        log("Spawning and notifying close players.");

        for (size_t i = 0; i < *character_count; i += 1) {
                // Notify player in the world of the new spawning character.
                bytes_zero(response, sizeof(response));

                if (is_npc(src)) {
                        bytes_zero((byte_t *) &npc_info, sizeof(npc_info));
                        gs_packet_npc_info(&npc_info, src);
                        gs_packet_npc_info_pack(response, &npc_info);
                } else {
                        bytes_zero((byte_t *) &char_info, sizeof(char_info));
                        gs_packet_char_info(&char_info, src);
                        gs_packet_char_info_pack(response, &char_info);
                }

                encrypt_and_send_packet(&characters[i], response);

                // Notify the spawning character of characters already
                // in the world (only if it's a player).
                if (is_npc(src)) {
                        continue;
                }

                bytes_zero(response, sizeof(response));

                if (is_npc(&characters[i])) {
                        bytes_zero((byte_t *) &npc_info, sizeof(npc_info));
                        gs_packet_npc_info(&npc_info, &characters[i]);
                        gs_packet_npc_info_pack(response, &npc_info);
                } else {
                        bytes_zero((byte_t *) &char_info, sizeof(char_info));
                        gs_packet_char_info(&char_info, &characters[i]);
                        gs_packet_char_info_pack(response, &char_info);
                }

                encrypt_and_send_packet(src, response);
        }

        characters[*character_count]       = *src;
        characters[*character_count].state = SPAWN;

        *character_count += 1;
}

gs_character_t *gs_character_from_session(gs_session_t *session)
{
        assert(session);

        for (size_t i = 0; i < *character_count; i += 1) {
                if (characters[i].session == session) {
                        return &characters[i];
                }
        }

        return 0;
}

void gs_character_request(gs_character_t *character, packet_t *packet)
{
        switch (character->state) {
        case SPAWN:
                spawn_state(character, packet);
                break;
        case IDLE:
                break;
        case MOVING:
                break;
        default:
                break;
        }
}
