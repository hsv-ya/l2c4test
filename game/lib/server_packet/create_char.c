#include <headers.h>
#include "create_char.h"

void server_packet_create_char(packet *dest)
{
        assert(dest);

        packet_type type = 0x19;

        byte_t buf[sizeof(server_packet_create_char_t)] = {0};
        byte_t *p = buf;

        server_packet_create_char_t create_char_packet = {0};

        create_char_packet.response = 0x01;
        BYTE_WRITE_VAL(p, create_char_packet.response);

        packet_build(dest, type, buf, (size_t) (p - buf));
}
