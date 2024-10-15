mkdir -p database
gcc server.c -lpthread -lbcrypt -o server
./server