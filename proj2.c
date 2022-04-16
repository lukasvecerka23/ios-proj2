// IOS Projekt 2 - Building H2O
// Lukas Vecerka xvecer30
// 1 BIT ak. rok 2021/22

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>


#define MMAP(pointer) {(pointer) = mmap(NULL, sizeof(*(pointer)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);}
#define UNMAP(pointer) {munmap((pointer), sizeof((pointer)));}

int init(){
    return 0;
}

void clean_everything(){

}

void process_molecule(){

}

void process_oxygen(){

}

void process_hydrogen(){

}

void oxygen_generator(){

}

void hydrogen_generator(){

}

int main(int argc, char **argv){
    char *ptr;
    if (argc != 5){
        fprintf(stderr, "Wrong number of arguments! Usage: ./proj2 NO NH TI TB");
        return 1;
    }

    long NO = strtol(argv[1],&ptr, 10);
    if (NO < 0 || *ptr != '\0'){
        fprintf(stderr, "NO must be positive number");
        return 1;
    }
    long NH = strtol(argv[2],&ptr, 10);
    if (NH < 0 || *ptr != '\0'){
        fprintf(stderr, "NH must be positive number");
        return 1;
    }
    long TI = strtol(argv[3],&ptr, 10);
    if (TI < 0 || TI > 1000 || *ptr != '\0'){
        fprintf(stderr, "TI must be positive number and from interval 0-1000");
        return 1;
    }
    long TB = strtol(argv[4],&ptr, 10);
    if (TB < 0 || TB > 1000 || *ptr != '\0'){
        fprintf(stderr, "TB must be positive number and from interval 0-1000");
        return 1;
    }
    return 0;
}
