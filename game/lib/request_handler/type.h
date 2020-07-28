#ifndef REQUEST_TYPE_H
#define REQUEST_TYPE_H

enum request_type
{
        REQUEST_TYPE_PROTOCOL_VERSION = 0x00,
        REQUEST_TYPE_MOVE_BACKWARDS_TO_LOCATION = 0x01,
        REQUEST_TYPE_ACTION = 0x04,
        REQUEST_TYPE_AUTH_REQUEST = 0x08,
        REQUEST_TYPE_NEW_CHAR = 0x0e,
        REQUEST_TYPE_CREATE_CHAR = 0x0b,
        REQUEST_TYPE_SELECTED_CHAR = 0x0d,
        REQUEST_TYPE_REQUEST_AUTO_SS_BSPS = 0xd0,
        REQUEST_TYPE_REQUEST_QUEST_LIST = 0x63,
        REQUEST_TYPE_ENTER_WORLD = 0x03,
        REQUEST_TYPE_RESTART = 0x46,
        REQUEST_TYPE_VALIDATE_POS = 0x48,
        REQUEST_TYPE_SAY = 0x38
};

#endif