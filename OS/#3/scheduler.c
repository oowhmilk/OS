#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <sys/wait.h>
#include <errno.h>

char printElapsedTime[256];
double averageElapsedTime = 0.0;

int status;
int pid[21];

int shm_fd;
char *shm_ptr;
const char *shm_name = "monitor_memory";

void print_menu(); // menu 출력하는 함수
void calculate(); // process 가 실행할 계산하는 함수
char* getElapsedTimeSpec(struct timespec Tstart, struct timespec Tend); // elpased time 구하는 함수
void addElapsedTime(const char *printElapsedTime); // elapsed time shared memory에 저장하는 함수 
void calculateSharedMemory(); // shared memory에 존재하는 값 계산하는 함수 
void clearSharedMemory(); // shared memory 비우는 함수

int main()
{

    print_menu();

    char input[2];
    int menu;

    fgets(input, sizeof(input), stdin);
    menu = atoi(input);

    struct timespec start, end;
    struct tm realstart, realend;

    switch (menu)
    {
        case 0 : // exit
            exit(0);
            break;

        case 1 : // CFS_DEFAULT
            {
                cpu_set_t set;
                CPU_ZERO(&set);    // CPU 코어 집합 초기화
                CPU_SET(0, &set);  // CPU 코어 0을 추가 (다른 코어를 추가하려면 여러 번 호출)

                if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &set) == -1) { // CPU 코어 개수 제한
                    perror("Error setting CPU affinity");
                    return 1;
                }

                for(int i = 0 ; i < 21 ; i++){

                    if((pid[i] = fork()) < 0) { // 자식 프로세스 생성
                        printf("fork error\n");
                        exit(1);
                    }
                    else if(pid[i] == 0) {
                        
                        clock_gettime(CLOCK_REALTIME, &start);

                        calculate();

                        clock_gettime(CLOCK_REALTIME, &end);

                        localtime_r((time_t *)&start, &realstart);
                        localtime_r((time_t *)&end, &realend);
                        
                        printf("PID: %d | Start time : %02d:%02d:%02d.%06ld | End time : %02d:%02d:%02d.%06ld | Elapsed time : %s\n", 
                            getpid(), 
                            realstart.tm_hour,realstart.tm_min, realstart.tm_sec, start.tv_nsec / 1000, 
                            realend.tm_hour,realend.tm_min, realend.tm_sec, end.tv_nsec / 1000, 
                            getElapsedTimeSpec(start, end));

                        addElapsedTime(printElapsedTime);
                        exit(0);
                    }
                }

                for(int i = 0 ; i < 21 ; i++) {
                    pid_t wpid = waitpid(pid[i],&status,0); // 자식 프로세스 종료 상태를 회수
                }

                calculateSharedMemory();

                printf("Scheduling Policy: CFS_DEFAULT | Average elapsed time: %06f\n", averageElapsedTime / 21);

                clearSharedMemory();

                break;
            }

        case 2 : // CFS_NICE
            {
                cpu_set_t set;
                CPU_ZERO(&set);    // CPU 코어 집합 초기화
                CPU_SET(0, &set);  // CPU 코어 0을 추가 (다른 코어를 추가하려면 여러 번 호출)

                if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &set) == -1) { // CPU 코어 개수 제한
                    perror("Error setting CPU affinity");
                    return 1;
                }

                for(int i = 0 ; i < 21 ; i++){
                    if((pid[i] = fork()) < 0) { // 자식 프로세스 생성
                        printf("fork error\n");
                        exit(1);
                    }
                    else if(pid[i] == 0) {

                        if(i < 7) // process 마다 nice 값 부여
                            nice(19); 
                        else if(i < 14)
                            nice(0);
                        else
                            nice(-20);
                        
                        clock_gettime(CLOCK_REALTIME, &start);

                        calculate();

                        clock_gettime(CLOCK_REALTIME, &end);

                        localtime_r((time_t *)&start, &realstart);
                        localtime_r((time_t *)&end, &realend);
                        
                        printf("PID: %d | NICE : %d | Start time : %02d:%02d:%02d.%06ld | End time : %02d:%02d:%02d.%06ld | Elapsed time : %s\n", 
                            getpid(),
                            getpriority(PRIO_PROCESS, getpid()),
                            realstart.tm_hour,realstart.tm_min, realstart.tm_sec, start.tv_nsec / 1000, 
                            realend.tm_hour,realend.tm_min, realend.tm_sec, end.tv_nsec / 1000, 
                            getElapsedTimeSpec(start, end));
                        
                        addElapsedTime(printElapsedTime);
                        exit(0);
                    }
                }

                for(int i = 0 ; i < 21 ; i++) {
                    pid_t wpid = waitpid(pid[i],&status,0); // 자식 프로세스 종료 상태를 회수
                }

                calculateSharedMemory();

                printf("Scheduling Policy: CFS_NICE | Average elapsed time: %06f\n", averageElapsedTime / 21);

                clearSharedMemory();

                break;
            }

        case 3 : // RT_FIFO
            {
                cpu_set_t set;
                CPU_ZERO(&set);    // CPU 코어 집합 초기화
                CPU_SET(0, &set);  // CPU 코어 0을 추가 (다른 코어를 추가하려면 여러 번 호출)

                if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &set) == -1) { // CPU 코어 개수 제한
                    perror("Error setting CPU affinity");
                    return 1;
                }

                for(int i = 0 ; i < 21 ; i++){

                    if((pid[i] = fork()) < 0) { // 자식 프로세스 생성
                        printf("fork error\n");
                        exit(1);
                    }
                    else if(pid[i] == 0) {

                        // FIFO 스케줄러 정책 설정
                        struct sched_param schedParam;
                        schedParam.sched_priority = 50; 
                        int policy = SCHED_FIFO; 

                        if (sched_setscheduler(getpid(), policy, &schedParam) == -1) {
                            perror("Error setting scheduler policy");
                            return 1;
                        }
                        
                        clock_gettime(CLOCK_REALTIME, &start);

                        calculate();

                        clock_gettime(CLOCK_REALTIME, &end);

                        localtime_r((time_t *)&start, &realstart);
                        localtime_r((time_t *)&end, &realend);
                        
                        printf("PID: %d | Start time : %02d:%02d:%02d.%06ld | End time : %02d:%02d:%02d.%06ld | Elapsed time : %s\n", 
                            getpid(), 
                            realstart.tm_hour,realstart.tm_min, realstart.tm_sec, start.tv_nsec / 1000, 
                            realend.tm_hour,realend.tm_min, realend.tm_sec, end.tv_nsec / 1000, 
                            getElapsedTimeSpec(start, end));

                        addElapsedTime(printElapsedTime);
                        exit(0);
                    }
                }

                for(int i = 0 ; i < 21 ; i++) {
                    pid_t wpid = waitpid(pid[i],&status,0); // 자식 프로세스 종료 상태를 회수
                }

                calculateSharedMemory();

                printf("Scheduling Policy: RT_FIFO | Average elapsed time: %06f\n", averageElapsedTime / 21);

                clearSharedMemory();

                break;
            }

        case 4 : // RT_RR
            {

                printf("Input time slice :");

                int num; // time slice 저장하는 변수
                scanf("%d", &num);
                    
                cpu_set_t set;
                CPU_ZERO(&set);    // CPU 코어 집합 초기화
                CPU_SET(0, &set);  // CPU 코어 0을 추가 (다른 코어를 추가하려면 여러 번 호출)

                if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &set) == -1) { // CPU 코어 개수 제한
                    perror("Error setting CPU affinity");
                    return 1;
                }

                FILE *fp = fopen("/proc/sys/kernel/sched_rr_timeslice_ms", "w");
                if (fp == NULL) {
                perror("Error opening file");
                return 1;
                }

                // 파일에 timeslice 값을 쓰고 파일을 닫기
                fprintf(fp, "%d", num);
                fclose(fp); 

                for(int i = 0 ; i < 21 ; i++){
                    if((pid[i] = fork()) < 0) { // 자식 프로세스 생성
                        printf("fork error\n");
                        exit(1);
                    }
                    else if(pid[i] == 0) {

                        // Round-Robin 스케줄러 정책 설정
                        struct sched_param schedParam;
                        schedParam.sched_priority = 50; 
                        int policy = SCHED_RR; 

                        if (sched_setscheduler(getpid(), policy, &schedParam) == -1) {
                            perror("Error setting scheduler policy");
                            return 1;
                        }

                        clock_gettime(CLOCK_REALTIME, &start);

                        calculate();

                        clock_gettime(CLOCK_REALTIME, &end);

                        localtime_r((time_t *)&start, &realstart);
                        localtime_r((time_t *)&end, &realend);
                        
                        printf("PID: %d | Start time : %02d:%02d:%02d.%06ld | End time : %02d:%02d:%02d.%06ld | Elapsed time : %s\n", 
                            getpid(), 
                            realstart.tm_hour,realstart.tm_min, realstart.tm_sec, start.tv_nsec / 1000, 
                            realend.tm_hour,realend.tm_min, realend.tm_sec, end.tv_nsec / 1000, 
                            getElapsedTimeSpec(start, end));

                        addElapsedTime(printElapsedTime);
                        exit(0);
                    }
                }

                for(int i = 0 ; i < 21 ; i++) {
                    pid_t wpid = waitpid(pid[i],&status,0); // 자식 프로세스 종료 상태를 회수
                }

                calculateSharedMemory();

                printf("Scheduling Policy: RT_RR |  Time Quantum: %d ms | Average elapsed time: %06f\n", num, averageElapsedTime / 21);

                clearSharedMemory();

                break;
            }

        default:
            printf("Wrong input\n");
            break;
    }



}

// menu 출력하는 함수
void print_menu() {
    printf("Input the Scheduling Policy to apply :\n");
    printf("1. CFS_DEFAULT\n");
    printf("2. CFS_NICE\n");
    printf("3. RT_FIFO\n");
    printf("4. RT_RR\n");
    printf("0. exit\n");
    printf("Input : ");
}

// process 가 실행할 계산하는 함수
void calculate() {
    int count = 0, k, i, j;
    int result[100][100], A[100][100], B[100][100];

    memset(result, 0, sizeof(result));
    memset(A, 0, sizeof(A));
    memset(B, 0, sizeof(B));

    while(count < 100){
        for(k = 0; k < 100; k++){
            for(i = 0; i < 100; i++) {
                for(j = 0; j < 100; j++) {
                    result[k][j] += A[k][i] * B[i][j];
                }
            }
        }
        count++;
    }
}

// elpased time 구하는 함수
char* getElapsedTimeSpec(struct timespec Tstart, struct timespec Tend) {
    Tend.tv_nsec = Tend.tv_nsec - Tstart.tv_nsec;
    Tend.tv_sec = Tend.tv_sec - Tstart.tv_sec;

    Tend.tv_nsec += (Tend.tv_sec*1000000000);
    
    sprintf(printElapsedTime, "%lf",Tend.tv_nsec / 1000000000.0);
    return printElapsedTime;
}

// elapsed time shared memory에 저장하는 함수 
void addElapsedTime(const char *printElapsedTime) {
    // shared memory 세그먼트를 열거나 생성
    shm_fd = shm_open(shm_name, O_RDWR | O_CREAT, 0644);
    if (shm_fd == -1) {
        perror("shm_open");
        return;
    }

    // 새 데이터를 수용할 수 있도록 shared memory 세그먼트의 크기를 조정
    struct stat st;
    if (fstat(shm_fd, &st) == -1) {
        perror("fstat");
        close(shm_fd);
        return;
    }
    off_t new_size = st.st_size + strlen(printElapsedTime) + 1;
    if (ftruncate(shm_fd, new_size) == -1) {
        perror("ftruncate");
        close(shm_fd);
        return;
    }

    // shared memory 세그먼트를 프로세스의 메모리 공간에 매핑
    shm_ptr = mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        return;
    }

    // `printElapsedTime`을 shared memory 에 추가
    strcpy(shm_ptr + st.st_size, printElapsedTime);
    shm_ptr[new_size - 1] = '\n'; // 끝에 개행 추가

    // 매핑 해제 및 shared memory 세그먼트 닫기
    munmap(shm_ptr, new_size);
    close(shm_fd);
}

// shared memory에 존재하는 값 계산하는 함수 
void calculateSharedMemory() {
    int shm_fd;
    char *shm_ptr, *line;
    const char *shm_name = "monitor_memory";

    // shared memory 세그먼트 열기
    shm_fd = shm_open(shm_name, O_RDONLY, 0);
    if (shm_fd == -1) {
        perror("shm_open");
        return;
    }

    // shared memory 세그먼트의 크기 얻기
    struct stat st;
    if (fstat(shm_fd, &st) == -1) {
        perror("fstat");
        close(shm_fd);
        return;
    }

    // shared memory 세그먼트를 프로세스의 메모리 공간에 매핑
    shm_ptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        return;
    }

    // 임시 버퍼에 메모리의 내용 복사 (strtok가 원본을 수정하기 때문에)
    char tempBuffer[st.st_size + 1];
    strncpy(tempBuffer, shm_ptr, st.st_size);
    tempBuffer[st.st_size] = '\0'; // 문자열의 끝을 표시

    // shared memory 세그먼트의 내용을 개행별로 잘라서 출력
    line = strtok(tempBuffer, "\n");
    while(line != NULL) {
        averageElapsedTime += strtod(line, NULL);
        line = strtok(NULL, "\n");
    }

    // 매핑 해제 및 shared memory 세그먼트 닫기
    munmap(shm_ptr, st.st_size);
    close(shm_fd);
}

// shared memory 비우는 함수
void clearSharedMemory() {
    // shared memory 세그먼트 삭제
    if (shm_unlink(shm_name) == -1) {
        perror("shm_unlink");
        return;
    }
}

