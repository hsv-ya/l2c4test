@echo off
@if not exist "bin" mkdir bin
pushd bin
gcc -Wall -Wextra -Wno-unused-value -o login_server ../login_server.c ../directory.c ../net_windows.c ../wqueue.c -I../openssl-1.1/x64/include -L../openssl-1.1/x64/lib -lcrypto -lpthread -lws2_32
gcc -Wall -Wextra -o game_server.dll -fPIC -shared ../game_server_lib.c ../directory.c ../net_windows.c ../wqueue.c  -lpthread -lws2_32
gcc -Wall -Wextra -o game_server ../game_server.c ../net_windows.c ../library.c -lpthread -lws2_32
popd
copy /Y /B openssl-1.1\x64\bin\libcrypto-1_1-x64.dll bin\* >nul