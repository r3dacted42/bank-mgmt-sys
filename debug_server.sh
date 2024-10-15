mkdir -p database
gcc -g server.c -lpthread -lbcrypt -o server
gdb server