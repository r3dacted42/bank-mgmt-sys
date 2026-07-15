mkdir -p database
gcc server.c -lpthread -l:bcrypt.a -o server
./server