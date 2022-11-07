#include "include/game_server.h"

void request_auth_decode(struct request_auth *dest, struct packet *src)
{
    assert(dest);
    assert(src);
    packet_read_arr(dest->username.buf, src);
    packet_read_u32(&dest->play_ok2, src);
    packet_read_u32(&dest->play_ok1, src);
    packet_read_u32(&dest->login_ok1, src);
    packet_read_u32(&dest->login_ok2, src);
}

void request_create_character_decode(struct request_create_character *dest, struct packet *src)
{
    assert(dest);
    assert(src);
    i32 to_ignore = 0;
    packet_read_l2_str_arr(dest->name.buf, src);
    packet_read_u32(&dest->race_id, src);
    packet_read_u32(&dest->sex, src);
    packet_read_u32(&dest->class_id, src);
    // ignore attributes from the packet since this is a place
    // where the user can cheat and send whatever.
    // 
    // todo: we could make this easier
    // by just: src->read_cursor += sizeof(all the attributes);
    packet_read_i32(&to_ignore, src);
    packet_read_i32(&to_ignore, src);
    packet_read_i32(&to_ignore, src);
    packet_read_i32(&to_ignore, src);
    packet_read_i32(&to_ignore, src);
    packet_read_i32(&to_ignore, src);
    packet_read_u32(&dest->hair_style_id, src);
    packet_read_u32(&dest->hair_color_id, src);
    packet_read_u32(&dest->face_id, src);
}
