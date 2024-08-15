#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_PATH 1024

void makeInputFile(int maxVirtualAddress); // "input.in" file 자동 생성하는 함수
void getVirtualAddress(); // 가상 메모리 값 받는 함수
int calculatePageNo(int num, int startBit, int numBits); // page number 구하는 함수 
void optimal(int* frameTable, int frameEntryNum, int virtualAddressLenght, int pageSize); // optimal 함수
int isPageFault(int* frameTable, int referenceString, int frameEntryNum); // optimal 에서 pagefault 확인하는 함수
int optimalReplace(int* frameTable, int* timeStamp, int check, int frameEntryNum); // optimal 에서 frameTable 바꾸는 함수
void fifo(int* frameTable, int frameEntryNum); // fifo 함수
void LRU(int* frameTable, int frameEntryNum); // LRU 함수
void second_chance(int* frameTable, int frameEntryNum); // second_chance 함수
void printNumber(int num, char *number); // 숫자 형식 맞춰주기

int virtualAddressLenght;
int maxVirtualAddress;
int pageSizeNum;
int pageSize; // offset 
int physicalMemorySizeNum;
int physicalMemorySize;
int algo;
int inputStyle;
char* input_path;
char number[20];

int frameEntryNum;
int virtualAddress[5000];
int referenceString[5000];

int pageNo;
int frameNo;
int physicalAddress; 

int frameTableP = 0; // frameTablePointer 
int pagefaultCnt = 0; // pagefault 횟수 저장 변수 


int main()
{
    printf("A. Simulation에 사용할 가상주소 길이를 선택하시오 (1. 18bits   2. 19bits   3. 20bits): ");
    scanf("%d", &virtualAddressLenght);
    
    printf("\nB. Simulation에 사용할 페이지(프레임)의 크기를 선택하시오 (1. 1KB   2. 2KB   3. 4KB): ");
    scanf("%d", &pageSizeNum);
    
    printf("\nC. Simulation에 사용할 물리메모리의 크기를 선택하시오 (1. 32KB   2. 64KB): ");
    scanf("%d", &physicalMemorySizeNum);
    
    printf("\nD. Simulation에 적용할 Page Replacement 알고리즘을 선택하시오\n");
    printf("(1. Optimal   2. FIFO   3. LRU   4. Second-Chance): ");
    scanf("%d", &algo);
    
    printf("\nE. 가상주소 스트링 입력방식을 선택하시오\n");
    printf("(1. input.in 자동 생성   2. 기존 파일 사용): ");
    scanf("%d", &inputStyle);
    
    switch (virtualAddressLenght)
    {
        case 1:
            maxVirtualAddress = 262143;
            virtualAddressLenght = 18;
            break;

        case 2:
            maxVirtualAddress = 524287;
            virtualAddressLenght = 19;
            break;

        case 3:
            maxVirtualAddress = 1048575;
            virtualAddressLenght = 20;
            break;

        default:
            break;
    }

    switch (pageSizeNum)
    {
        case 1:
            pageSize = 10; // 1KB
            break;

        case 2:
            pageSize = 11; // 2KB
            break;

        case 3:
            pageSize = 12; // 4KB
            break;

        default:
            break;
    }

    switch (physicalMemorySizeNum)
    {
        case 1:
            physicalMemorySize = 15; // 32KB
            break;

        case 2:
            physicalMemorySize =16; // 64KB
            break;

        default:
            break;
    }
    
    frameEntryNum = (int)pow(2, physicalMemorySize - pageSize); // frameTable 크기 저장 변수 
    int frameTable[frameEntryNum];
    for(int i = 0 ; i < frameEntryNum ; i++) 
    {
        frameTable[i] = -1; // 초기화
    }


    switch (inputStyle)
    {
        case 1:
            makeInputFile(maxVirtualAddress);
            input_path = (char*) malloc(sizeof(char) * MAX_PATH);
            strcpy(input_path, "input.in");       
            break;

        case 2:
            getchar();
            printf("\nF. 입력 파일 이름을 입력하시오: ");
            input_path = (char *) malloc(sizeof(char) * MAX_PATH);

            if (fgets(input_path, MAX_PATH, stdin) != NULL) 
            {
                size_t len = strlen(input_path);
                if (len > 0 && input_path[len - 1] == '\n') 
                {
                    input_path[len - 1] = '\0';
                }
            }
            break;
            
        default:
            break;
    }


    switch (algo)
    {
        case 1:
            optimal(frameTable, frameEntryNum, virtualAddressLenght, pageSize);
            break;

        case 2:
            fifo(frameTable, frameEntryNum);
            break;

        case 3:
            LRU(frameTable, frameEntryNum);
            break;

        case 4:
            second_chance(frameTable, frameEntryNum);
            break;

        default:
            break;
    }
    
}

// "input.in" file 자동 생성하는 함수
void makeInputFile(int maxVirtualAddress) {
    // 파일 열기
    FILE *file = fopen("input.in", "w");
    
    if (file == NULL) {
        printf("파일을 열 수 없습니다.\n");
    }

    // 난수 초기화
    srand(time(NULL));

    for (int i = 0; i < 5000; i++) {
        int random_value = rand() % (maxVirtualAddress + 1);
        fprintf(file, "%d\n", random_value); // 파일에 값 저장
    }

    // 파일 닫기
    fclose(file);
}

// 가상 메모리 값 받는 함수
void getVirtualAddress() {

    FILE *file = fopen(input_path, "r");

    int count = 0;
    while (fscanf(file, "%d", &virtualAddress[count]) != EOF) {
        count++;
    }

    fclose(file);
}

// page number 구하는 함수 
int calculatePageNo(int num, int startBit, int numBits) {
    // 추출할 비트 범위를 마스킹하여 원하는 비트를 추출
    int mask = ((1 << numBits) - 1) << (startBit - 1);
    int result = (num & mask) >> (startBit - 1);

    return result;
}

// physical address 구하는 함수
int cacluatePhysicalAddress(int num, int startBit, int numBits, int pageNo) {
    // 기존 값을 clear
    int mask = ~(((1 << numBits) - 1) << (startBit - 1));
    num &= mask;

    // 새로운 값을 set
    num |= (pageNo << (startBit - 1));

    return num;
}

// 숫자 형식 맞춰주기
void printNumber(int num, char *number) {
    char temp[20];
    sprintf(temp, "%d", num); // 숫자를 문자열로 변환

    int length = strlen(temp);
    int comma_count = (length - 1) / 3; // 삽입할 쉼표의 수 계산

    int j = 0;
    for (int i = 0; i < length; i++) {
        number[j++] = temp[i];
        if ((length - i - 1) % 3 == 0 && (length - i - 1) != 0) {
            number[j++] = ','; // 쉼표 삽입
        }
    }
    number[j] = '\0'; // 문자열 끝에 null 문자 추가
}

// optimal 함수
void optimal(int* frameTable, int frameEntryNum, int virtualAddressLenght, int pageSize) {
	
    // 파일 열기
    FILE *file = fopen("output.opt", "w");
    
    if (file == NULL) {
        printf("파일을 열 수 없습니다.\n");
    }

    getVirtualAddress();
    fprintf(file, "%-10s  %-10s  %-10s  %-10s  %-10s  %-10s\n", "No.", "V.A", "Page No.", "Frame No.", "P.A", "Page Fault\n");

    
    int pagefaultCnt = 0;
    int maxPageNumber = (int)pow(2, virtualAddressLenght - pageSize);
    int pagefault = 1;
	int timeStamp[frameEntryNum]; // frameTable 에 들어온 순서 저장 변수
	int pageTable[maxPageNumber]; //page -> frameNumber

    for(int i = 0 ; i < 5000 ; i++)
    {
        referenceString[i] = calculatePageNo(virtualAddress[i], pageSize + 1, virtualAddressLenght - pageSize); // reference string 저장하기
    }

	for(int i = 0; i < frameEntryNum; i++) 
    {
		timeStamp[i] = -1; // timestamp 초기화
	}

	for(int i = 0; i < 5000; i++) 
    {
		if(!isPageFault(frameTable, referenceString[i], frameEntryNum)) // page fault 난 경우 
        {
			int replaceNo = optimalReplace(frameTable, timeStamp, i + 1, frameEntryNum);
			frameTable[replaceNo] = referenceString[i];
			timeStamp[replaceNo] = i;
			frameNo = replaceNo; // frameNo 저장하기
			pageTable[referenceString[i]] = replaceNo; 
            
			pagefaultCnt++;
			pagefault = 1;
		}
		else 
        {
			pagefault = 0;
			frameNo = pageTable[referenceString[i]]; // frameNo 저장하기
		}

        physicalAddress = cacluatePhysicalAddress(virtualAddress[i], pageSize + 1, virtualAddressLenght - pageSize, frameNo);
        
        fprintf(file, "%-10d  %-10d  %-10d  %-10d  %-10d  ", i + 1, virtualAddress[i], referenceString[i], frameNo, physicalAddress);
        
        if(pagefault)
            fprintf(file, "%-10s\n", "F");
        else
            fprintf(file, "%-10s\n", "H");
	}

	printNumber(pagefaultCnt, number);

    fprintf(file, "Total Number of Page Faults: %s\n", number);

    // 파일 닫기
    fclose(file);
}

// optimal 에서 pagefault 확인하는 함수
int isPageFault(int* frameTable, int referenceString, int frameEntryNum) { 
	for(int i = 0; i < frameEntryNum; i++) 
    {
		if(frameTable[i] == referenceString) 
            return 1;
	}
	return 0;
}

// optimal 에서 frameTable 바꾸는 함수
int optimalReplace(int* frameTable, int* timeStamp, int check, int frameEntryNum) { 
	
    int maxDist = -1;
	int replaceNo = -1;
	int oldStamp = 10000;
	bool check = true;

	for(int i = 0; i < frameEntryNum; i++) 
    {
		int j;
		for(j = check; j < 5000; j++) 
        {
			if(frameTable[i] == referenceString[j]) // 현 시점 사용하는 pageNo 와 같은게 있을 경우
            {
				if(j > maxDist && check) 
                {
					maxDist = j;
					replaceNo = i;
				}
				break;
			}
		}
		if(j == 5000) // 끝까지 돌았을 경우 다시는 안 나오는 값인 경우 
        {
			if(timeStamp[i] == -1) 
                return i;
			if(timeStamp[i] < oldStamp) // 가장 오래된 값 찾기 
            {
				oldStamp = timeStamp[i];
				replaceNo = i;
				check = false;
			}
		}
	}
	return replace;
}

// fifo 함수 
void fifo(int* frameTable, int frameEntryNum) {

    // 파일 열기
    FILE *file = fopen("output.fifo", "w");
    
    if (file == NULL) {
        printf("파일을 열 수 없습니다.\n");
    }

    getVirtualAddress();
    fprintf(file, "%-10s  %-10s  %-10s  %-10s  %-10s  %-10s\n", "No.", "V.A", "Page No.", "Frame No.", "P.A", "Page Fault\n");

    for(int i = 0 ; i < 5000 ; i ++) 
    {
        int pagefault = 1;
        pageNo = calculatePageNo(virtualAddress[i], pageSize + 1, virtualAddressLenght - pageSize);
        
        for(int j = 0 ; j < frameEntryNum ; j++) 
        {
            if(pageNo == frameTable[j]) // hit 한 경우 
            {
                pagefault = 0;
                frameNo = j;
                break;
            }
        }

        if(pagefault) // page fault 난 경우 
        {
            frameTable[frameTableP] = pageNo;
            frameNo = frameTableP;
            frameTableP = (frameTableP + 1) % frameEntryNum; // frameTablePointer 값 증가
            pagefaultCnt++;
        }

        physicalAddress = cacluatePhysicalAddress(virtualAddress[i], pageSize + 1, virtualAddressLenght - pageSize, frameNo);

        fprintf(file, "%-10d  %-10d  %-10d  %-10d  %-10d  ", i + 1, virtualAddress[i], pageNo, frameNo, physicalAddress);
        if(pagefault)
            fprintf(file, "%-10s\n", "F");
        else
            fprintf(file, "%-10s\n", "H");
    }

    printNumber(pagefaultCnt, number);

    fprintf(file, "Total Number of Page Faults: %s\n", number);

    // 파일 닫기
    fclose(file);
}

// LRU 함수 
void LRU(int* frameTable, int frameEntryNum) {

    // 파일 열기
    FILE *file = fopen("output.lru", "w");
    
    if (file == NULL) {
        printf("파일을 열 수 없습니다.\n");
    }

    getVirtualAddress();
    fprintf(file, "%-10s  %-10s  %-10s  %-10s  %-10s  %-10s\n", "No.", "V.A", "Page No.", "Frame No.", "P.A", "Page Fault\n");

    int lruarr[frameEntryNum];
    for (int k = 0; k < frameEntryNum; k++) 
    { 
        lruarr[k] = -1; // 초기화
    }

    for(int i = 0 ; i < 5000 ; i ++) 
    {
        int pagefault = 1;
        int isFullFrame = 1;
        pageNo = calculatePageNo(virtualAddress[i], pageSize + 1, virtualAddressLenght - pageSize);
        
        for(int j = 0 ; j < frameEntryNum ; j++)
        {
            lruarr[j]++;
        }

        for(int j = 0 ; j < frameEntryNum ; j++) 
        {
            if(frameTable[j] == -1) // frameTable 이 가득차지 않은 경우 
                isFullFrame = 0;
            if(pageNo == frameTable[j])  // hit 한 경우 
            {
                pagefault = 0;
                frameNo = j;
                lruarr[j] = 0;
                break;
            }
        }

        if(pagefault) // page fault 난 경우 
        {
            if(isFullFrame) // frameTable 이 가득찬 경우 
            {
                for(int k = 0 ; k < frameEntryNum ; k++)
                {
                    if(lruarr[k] > lruarr[frameTableP]) // 사용한지 가장 오래된 값 찾기
                        frameTableP = k;
                }
            }

            frameTable[frameTableP] = pageNo;
            lruarr[frameTableP] = 0; // 새로 들어와서 0으로 저장하기
            frameNo = frameTableP;
            frameTableP = (frameTableP + 1) % frameEntryNum;
            pagefaultCnt++;
        }

        physicalAddress = cacluatePhysicalAddress(virtualAddress[i], pageSize + 1, virtualAddressLenght - pageSize, frameNo);

        fprintf(file, "%-10d  %-10d  %-10d  %-10d  %-10d  ", i + 1, virtualAddress[i], pageNo, frameNo, physicalAddress);
        if(pagefault)
            fprintf(file, "%-10s\n", "F");
        else
            fprintf(file, "%-10s\n", "H");
    }

    printNumber(pagefaultCnt, number);

    fprintf(file, "Total Number of Page Faults: %s\n", number);

    // 파일 닫기
    fclose(file);
}

// second_chance 함수
void second_chance(int* frameTable, int frameEntryNum) {

    // 파일 열기
    FILE *file = fopen("output.sc", "w");
    
    if (file == NULL) {
        printf("파일을 열 수 없습니다.\n");
    }

    getVirtualAddress();
    fprintf(file, "%-10s  %-10s  %-10s  %-10s  %-10s  %-10s\n", "No.", "V.A", "Page No.", "Frame No.", "P.A", "Page Fault\n");

    int scarr[frameEntryNum]; // second_chance 확인하는 변수
    for (int k = 0; k < frameEntryNum; k++) 
    { 
        scarr[k] = 0; // 초기화
    }

    for (int i = 0 ; i < 5000 ; i++)
    {
        int pagefault = 1;
        int isFullFrame = 1;
        pageNo = calculatePageNo(virtualAddress[i], pageSize + 1, virtualAddressLenght - pageSize);
        
        for(int j = 0 ; j < frameEntryNum ; j++) 
        {
            if(frameTable[j] == -1) // frameTable 이 가득차지 않은 경우 
                isFullFrame = 0;
            if(pageNo == frameTable[j]) // hit 한 경우 
            {
                pagefault = 0;
                frameNo = j;
                scarr[j] = 1;
                break;
            }
        }

        if(pagefault) // page fault 난 경우 
        {
            while(scarr[frameTableP] == 1) // scarr 값이 1 인 경우
            {
                scarr[frameTableP] = 0; // scarr 값을 0 으로 바꾸기
                frameTableP = (frameTableP + 1) % frameEntryNum;
            }

            frameTable[frameTableP] = pageNo; // scarr 값이 0 인 값이 나온 경우 frameTable 값 바꾸기
            frameNo = frameTableP;
            frameTableP = (frameTableP + 1) % frameEntryNum;
            pagefaultCnt++;
            
        }

        physicalAddress = cacluatePhysicalAddress(virtualAddress[i], pageSize + 1, virtualAddressLenght - pageSize, frameNo);

        fprintf(file, "%-10d  %-10d  %-10d  %-10d  %-10d  ", i + 1, virtualAddress[i], pageNo, frameNo, physicalAddress);
        if(pagefault)
            fprintf(file, "%-10s\n", "F");
        else
            fprintf(file, "%-10s\n", "H");
    }

    printNumber(pagefaultCnt, number);

    fprintf(file, "Total Number of Page Faults: %s\n", number);

    // 파일 닫기
    fclose(file);
}
