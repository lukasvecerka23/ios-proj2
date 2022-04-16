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
FILE *file;

// Semaphores
sem_t *sem_oxygen = NULL;
sem_t *sem_hydrogen = NULL;
sem_t *sem_molecule = NULL;
sem_t *sem_message = NULL;


// Shared variables
int *action_id = NULL;
int *molecule_cnt = NULL;
int *oxygen_cnt = NULL;
int *hydrogen_cnt = NULL;
bool *molecule_created = NULL;
bool *end = NULL;

int init(){
    MMAP(molecule_cnt);
    MMAP(oxygen_cnt);
    MMAP(hydrogen_cnt);
    MMAP(molecule_created);
    MMAP(end);
    MMAP(action_id);
    if ((sem_oxygen = sem_open("/xvecer30.semaphore1", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return false;
    if ((sem_hydrogen = sem_open("/xvecer30.semaphore2", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return false;
    if ((sem_molecule = sem_open("/xvecer30.semaphore3", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return false;
    if ((sem_message = sem_open("/xvecer30.semaphore4", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return false;
    return true;
}

void clean_everything(){
    UNMAP(molecule_cnt);
    UNMAP(oxygen_cnt);
    UNMAP(hydrogen_cnt);
    UNMAP(molecule_created);
    UNMAP(end);
    UNMAP(action_id);
    sem_close(sem_oxygen);
    sem_close(sem_hydrogen);
    sem_close(sem_molecule);
    sem_close(sem_message);
    sem_unlink("/xvecer30.semaphore1");
    sem_unlink("/xvecer30.semaphore2");
    sem_unlink("/xvecer30.semaphore3");
    sem_unlink("/xvecer30.semaphore4");
}

void process_oxygen(long TI, int oxygen_id, long TB){
    (void)TI;
    (void)TB;
    sem_wait(sem_message);
    fprintf(file, "%d: H %d: started", ++(*action_id), oxygen_id);
    fflush(file);
    sem_post(sem_message);
    usleep(rand()%TI);
    exit(0);
}

void process_hydrogen(long TI, int hydrogen_id){
    (void)TI;
    sem_wait(sem_message);
    fprintf(file, "%d: O %d: started", ++(*action_id), hydrogen_id);
    fflush(file);
    sem_post(sem_message);
    usleep(rand()%TI);
    exit(0);
}

void oxygen_generator(long NO, long TI, long TB){
    pid_t oxygen;
    for (int i = 0; i < NO; i++){
        oxygen = fork();
        if (oxygen == 0){
            process_oxygen(TI, i+1, TB);
            
        } else if (oxygen < 0){
            fprintf(stderr, "Fork failed\n");
            exit(1);
        }
    }
    exit(0);
}

void hydrogen_generator(long NH, long TI){
    pid_t hydrogen;
    for (int i = 0; i < NH; i++){
        hydrogen = fork();
        if (hydrogen == 0){
            process_hydrogen(TI, i+1);
            
        } else if (hydrogen < 0){
            fprintf(stderr, "Fork failed\n");
            exit(1);
        }
    }
    exit(0);
}

int main(int argc, char **argv){

    // Argument parsing
    char *ptr;
    if (argc != 5){
        fprintf(stderr, "Wrong number of arguments! Usage: ./proj2 NO NH TI TB\n");
        return 1;
    }
    long NO = strtol(argv[1],&ptr, 10);
    if (NO < 0 || *ptr != '\0'){
        fprintf(stderr, "NO must be positive number\n");
        return 1;
    }
    long NH = strtol(argv[2],&ptr, 10);
    if (NH < 0 || *ptr != '\0'){
        fprintf(stderr, "NH must be positive number\n");
        return 1;
    }
    long TI = strtol(argv[3],&ptr, 10);
    if (TI < 0 || TI > 1000 || *ptr != '\0'){
        fprintf(stderr, "TI must be positive number and from interval 0-1000\n");
        return 1;
    }
    long TB = strtol(argv[4],&ptr, 10);
    if (TB < 0 || TB > 1000 || *ptr != '\0'){
        fprintf(stderr, "TB must be positive number and from interval 0-1000\n");
        return 1;
    }

    file = fopen("proj2.out", "w");
    if (file == NULL){
        fprintf(stderr, "Error with file opening\n");
        return 1;
    }

    if(init() == false){
        clean_everything();
        fclose(file);
        fprintf(stderr, "Problem with semaphore initialization\n");
        return 1;
    }
    (*action_id) = 0;
    (*molecule_cnt) = 0;
    (*oxygen_cnt) = 0;
    (*hydrogen_cnt) = 0;
    (*molecule_created) = false;
    (*end) = false;
    pid_t start = fork();
    if (start == 0){
        oxygen_generator(NO, TI, TB);
    } else if (start > 0){
        hydrogen_generator(NH, TI);
    } 
    else if (start < 0){
        fprintf(stderr, "Fork failed\n");
        return 1;
    }

    clean_everything();
    fclose(file);
    exit(0);
    return 0;
}
