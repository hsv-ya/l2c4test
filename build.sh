#!/bin/bash
echo building...
echo building login server executable
tcc \
    -O0 \
    -DMAX_CONNECTIONS=32 \
    src/l2_string.c \
    src/linux_endian.c \
    src/linux_login_server.c \
    src/linux_network.c \
    src/linux_random.c \
    src/linux_storage.c \
    src/login_server.c \
    src/login_request.c \
    src/login_response.c \
    src/login_session.c \
    src/packet.c \
    src/printf.c \
    src/str.c \
    -lcrypto \
    -o build/login_server

echo building game server library.
tcc \
    -O0 \
    -DMAX_CONNECTIONS=32 \
    src/l2_string.c \
    src/linux_endian.c \
    src/linux_network.c \
    src/linux_random.c \
    src/linux_storage.c \
    src/packet.c \
    src/game_server.c \
    src/game_session.c \
    src/game_request.c \
    src/game_response.c \
    src/printf.c \
    src/str.c \
    -o build/game_server_lib \
    -shared \
    -rdynamic

echo building game server executable.
tcc \
    -O0 \
    -DMAX_CONNECTIONS=32 \
    src/l2_string.c \
    src/packet.c \
    src/linux_endian.c \
    src/linux_network.c \
    src/linux_random.c \
    src/linux_storage.c \
    src/linux_game_server.c \
    src/printf.c \
    src/str.c \
    -o build/game_server \
    -ldl

