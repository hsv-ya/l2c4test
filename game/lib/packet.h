#ifndef LIB_PACKET_H
#define LIB_PACKET_H

#include <stddef.h>
#include "types.h"

/**
 * Lineage 2 packets consist of two headers
 * and the content. Headers being:
 *
 * Packet size (unsigned short)
 * Contains the full size in bytes of the packet.
 * It includes the size of both headers.
 *
 * Packet type (unsigned char)
 * Contains the type of the packet.
 *
 * The rest of the content represents the
 * body of the packet.
 */
typedef unsigned char packet;

typedef unsigned short packet_size;

typedef unsigned char packet_type;

/**
 * Packets need to be multiple of 8
 * Not only that, we need to reserve
 * 4 bytes for checksum.
 *
 * Example:
 *
 * packet size = 5 (includes byte from packet type)
 * checksum = 4
 * packet size + checksum = 9
 * padded = PACKET_PADDED_SIZE(packet size + checksum) = 16
 *
 * padded (16) is now safe to use as packet size.
 */
#define PACKET_PADDED_SIZE(size) \
        (((size_t) (size + 4 + 7)) & ((size_t) (~7)))

/**
 * Calculate packet safe size (multiple of 8)
 * from packet content + packet type byte.
 */
#define PACKET_SAFE_SIZE(content) \
        (PACKET_PADDED_SIZE(sizeof(packet_type) + sizeof(content)))

/**
 * Calculate full safe packet size, including
 * packet type + content (these last two multiple of 8) + size
 */
#define PACKET_SAFE_FULL_SIZE(content) \
        (sizeof(packet_size) + PACKET_SAFE_SIZE(content))

/**
 * Build packet from type and content.
 * dest must be long enough to hold the
 * entire information.
 */
void packet_build(packet *dest, packet_type type, byte_t *content, size_t content_size);

/**
 * Get the FULL size from a packet
 * built with packet_build. The packet
 * size includes: size header, type header and
 * the body of the packet itself.
 */
packet_size packet_get_size(packet *src);

/**
 * Get the packet type
 * from packet.
 */
packet_type packet_get_type(packet *src);

/**
 * Get the content of the packet.
 */
byte_t *packet_body(packet *src);

#endif
