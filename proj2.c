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
#include <signal.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <time.h>


#define MMAP(pointer) {(pointer) = mmap(NULL, sizeof(*(pointer)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);}
#define UNMAP(pointer) {munmap((pointer), sizeof((pointer)));}
FILE *file;

// Semaphores
sem_t *mutex = NULL;
sem_t *barrier = NULL;
sem_t *sem_oxygen = NULL;
sem_t *sem_hydrogen = NULL;
sem_t *sem_molecule = NULL;
sem_t *sem_message = NULL;


// Shared variables
int *action_id = NULL;
int *molecule_cnt = NULL;
int *oxygen_cnt = NULL;
int *oxygens = NULL;
int *hydrogens = NULL;
int *hydrogen_cnt = NULL;
int *created = NULL;


bool init(){
    MMAP(molecule_cnt);
    MMAP(oxygen_cnt);
    MMAP(hydrogen_cnt);
    MMAP(action_id);
    MMAP(created);
    MMAP(oxygens);
    MMAP(hydrogens);
    if ((sem_oxygen = sem_open("/xvecer30.semaphore1", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return false;
    if ((sem_hydrogen = sem_open("/xvecer30.semaphore2", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return false;
    if ((sem_molecule = sem_open("/xvecer30.semaphore3", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return false;
    if ((sem_message = sem_open("/xvecer30.semaphore4", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) return false;
    if ((mutex = sem_open("/xvecer30.semaphore5", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) return false;
    if ((barrier = sem_open("/xvecer30.semaphore6", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return false;
    return true;
}

void clean_everything(){
    UNMAP(molecule_cnt);
    UNMAP(oxygen_cnt);
    UNMAP(hydrogen_cnt);
    UNMAP(action_id);
    UNMAP(created);
    UNMAP(oxygens);
    UNMAP(hydrogens);
    sem_close(sem_oxygen);
    sem_close(sem_hydrogen);
    sem_close(sem_molecule);
    sem_close(sem_message);
    sem_close(mutex);
    sem_close(barrier);
    sem_unlink("/xvecer30.semaphore1");
    sem_unlink("/xvecer30.semaphore2");
    sem_unlink("/xvecer30.semaphore3");
    sem_unlink("/xvecer30.semaphore4");
    sem_unlink("/xvecer30.semaphore5");
    sem_unlink("/xvecer30.semaphore6");
}

void process_oxygen(long TI, int oxygen_id, long TB, long NH, long NO){
    (void)NH;
    (void)NO;
    // Oxygen started
    sem_wait(sem_message);
    fprintf(file, "%d: O %d: started\n", ++(*action_id), oxygen_id);
    fflush(file);
    sem_post(sem_message);

    // Oxygen going to queue
    srand(time(0)+getpid());
    usleep((rand()%(TI+1))*1000);
    sem_wait(sem_message);
    fprintf(file, "%d: O %d: going to queue\n", ++(*action_id), oxygen_id);
    fflush(file);
    sem_post(sem_message);

    sem_wait(mutex);
    printf("Mutex: Molecule cnt: %d, oxygen id: %d\n", *molecule_cnt,oxygen_id);
    (*oxygen_cnt)++;
    (*oxygens)++;   
    if (*hydrogen_cnt >= 2){
        sem_post(sem_hydrogen);
        sem_post(sem_hydrogen);
        (*hydrogen_cnt)-=2;
        sem_post(sem_oxygen);
        (*oxygen_cnt)--;
    } else {
        sem_post(mutex);
    }

    if (*oxygens*2 <= NH){
        printf("OxyQueue: Molecule cnt: %d, oxygen id: %d\n", *molecule_cnt,oxygen_id);
        sem_wait(sem_oxygen);
    } else {
        sem_wait(sem_message);
        fprintf(file, "%d: O %d: not enough H\n", ++(*action_id), oxygen_id);
        fflush(file);
        sem_post(sem_message);
        exit(0);
    }

    sem_wait(sem_message);
    fprintf(file, "%d: O %d: creating molecule %d\n", ++(*action_id), oxygen_id, *molecule_cnt);
    fflush(file);
    sem_post(sem_message);

    sem_wait(barrier);
    usleep((rand()%(TB+1))*1000);
    sem_post(sem_molecule);
    sem_post(sem_molecule);

    sem_wait(sem_message);
    fprintf(file, "%d: O %d: molecule %d created\n", ++(*action_id), oxygen_id, *molecule_cnt);
    fflush(file);
    sem_post(sem_message);

    sem_wait(barrier);
    (*molecule_cnt)++;
    sem_post(mutex);

    exit(0);
}

void process_hydrogen(long TI, int hydrogen_id, long NH, long NO){
    (void)NH;
    (void)NO;
    // Hydrogen started
    sem_wait(sem_message);
    fprintf(file, "%d: H %d: started\n", ++(*action_id), hydrogen_id);
    fflush(file);
    sem_post(sem_message);

    // Hydrogen going to queue
    srand(time(0)+getpid());
    usleep((rand()%(TI+1))*1000);
    sem_wait(sem_message);
    fprintf(file, "%d: H %d: going to queue\n", ++(*action_id), hydrogen_id);
    fflush(file);
    sem_post(sem_message);

    sem_wait(mutex);
    printf("Mutex: Molecule cnt: %d, hydrogen id: %d\n", *molecule_cnt,hydrogen_id);
    (*hydrogen_cnt)++;
    (*hydrogens)++;
    if (*hydrogen_cnt >= 2 && *oxygen_cnt >= 1){
        sem_post(sem_hydrogen);
        sem_post(sem_hydrogen);
        (*hydrogen_cnt)-=2;
        sem_post(sem_oxygen);
        (*oxygen_cnt)--;
    } else {
        sem_post(mutex);
    }

    if (*hydrogens <= NO*2 && (*hydrogens%2 == 0 || *hydrogens+1 <= NH)){
            printf("HydroQueue: Molecule cnt: %d, hydrogen id: %d\n", *molecule_cnt,hydrogen_id);
            printf("HydroQueue: oxygens: %d, hydrogens: %d\n", *oxygens,*hydrogens);
            sem_wait(sem_hydrogen);
    } else {
            sem_wait(sem_message);
            fprintf(file, "%d: H %d: not enough O or H\n", ++(*action_id), hydrogen_id);
            printf("NotEnough: oxygens: %d, hydrogens: %d\n", *oxygens,*hydrogens);
            fflush(file);
            sem_post(sem_message);
            exit(0);
        }


    sem_wait(sem_message);
    fprintf(file, "%d: H %d: creating molecule %d\n", ++(*action_id), hydrogen_id, *molecule_cnt);
    fflush(file);
    sem_post(sem_message);
    (*created)++;
    if (*created == 2){
        sem_post(barrier);
    }

    sem_wait(sem_molecule);
    sem_wait(sem_message);
    fprintf(file, "%d: H %d: molecule %d created\n", ++(*action_id), hydrogen_id, *molecule_cnt);
    fflush(file);
    sem_post(sem_message);
    (*created)--;
    if (*created == 0){
        sem_post(barrier);
    }

    exit(0);
}

void oxygen_generator(long NO, long TI, long TB, long NH){
    pid_t oxygen;
    for (int i = 0; i < NO; i++){
        oxygen = fork();
        if (oxygen == 0){
            process_oxygen(TI, i+1, TB, NH, NO);
            exit(0);          
        } else if (oxygen < 0){
            fprintf(stderr, "Fork failed\n");
            exit(1);
        }
    }
}

void hydrogen_generator(long NH, long TI, long NO){
    pid_t hydrogen;
    for (int i = 0; i < NH; i++){
        hydrogen = fork();
        if (hydrogen == 0){
            process_hydrogen(TI, i+1, NH, NO);
            exit(0);
        } else if (hydrogen < 0){
            fprintf(stderr, "Fork failed\n");
            exit(1);
        }
    }
    
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
    (*molecule_cnt) = 1;
    (*oxygen_cnt) = 0;
    (*hydrogen_cnt) = 0;
    (*created) = 0;
    (*oxygens) = 0;
    (*hydrogens) = 0;

    pid_t mainproc = fork();
    if (mainproc == 0){
        oxygen_generator(NO, TI, TB, NH);
    } else if (mainproc > 0){
        hydrogen_generator(NH, TI, NO);
    } 
    else if (mainproc < 0){
        fprintf(stderr, "Fork failed\n");
        return 1;
    }

    clean_everything();
    fclose(file);
    return 0;
}
