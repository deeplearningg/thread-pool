#g++ main.cpp -I../include -DTP -L../include -lmultithread -pthread -Wl,-rpath,../include -o mt
g++ test.cpp help.h -g -I../include -DTP -L../include -lmultithread -pthread -Wl,-rpath,../include -o test
