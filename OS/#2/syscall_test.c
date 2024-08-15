#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 1024

#define __NR_print_hello 449
#define __NR_print_reverse 450
#define __NR_plus 451
#define __NR_minus 452

void delete_space(char s[]); // 공백을 제거하는 함수

int main() {

    char input[BUFFER_SIZE]; // 입력 문자열을 저장할 배열
    char num1[BUFFER_SIZE]; // 첫 번째 숫자를 저장할 배열
    char num2[BUFFER_SIZE]; // 두 번째 숫자를 저장할 배열
    char operator; // 연산자를 저장할 변수
    int i,j = 0;

    while(1) {
        printf("Input: ");
        fgets(input, sizeof(input), stdin); // 사용자로부터 입력 받음

        // 개행 문자 제거
        input[strcspn(input, "\n")] = '\0';

        // 공백 제거
        for (i = 0; input[i] != '\0'; i++) {
            if (input[i] != ' ') {
                break;
            }
        }

        // 공백을 제거
        delete_space(input);

        // 개행 입력시 종료
        if(strcmp(&input[0], "") == 0) {
            exit(0);
        }


        int operatorNum = 0;
        // 연산자 개수 찾기
        for (i = 0; input[i] != '\0'; i++) {

            if(isdigit(input[i]) == 0) {
                operatorNum++;
            }
        }
        // 연산자 위치 찾기
        for (i = 0; input[i] != '\0'; i++) {

            if(isdigit(input[i]) == 0) {
                operator = input[i]; 
                break;
            }
        }

        // 연산자를 찾았을 경우
        if (input[i] != '\0') {
            // 숫자 부분을 추출
            for (j = 0; j < i; j++) {
                num1[j] = input[j];
            }
            num1[j] = '\0'; // 문자열 끝을 표시

            // 두 번째 숫자 부분 추출
            for (i = i + 1, j = 0; input[i] != '\0'; i++) {
                num2[j] = input[i];
                j++;
            }
            num2[j] = '\0'; // 문자열 끝을 표시
        }
        // 연산자를 찾지 못한 경우
        else {
            strcpy(num1, input);
            operator = '\0';
            strcpy(num2, "");
        }

        // 문자열을 정수로 변환하여 계산 예시
        int operand1 = atoi(num1);
        int operand2 = atoi(num2);
        int result;
        char resultString[BUFFER_SIZE];


        if( operator == '\0') { // 연산자가 없는 경우
            syscall(__NR_print_reverse,num1,resultString);
            
            // 결과 출력
            printf("Output: %s\n", resultString);
        }
        else { // 연산자가 있는 경우
            if(strcmp(num2, "") == 0) {
                printf("Wrong Input!\n");
            }
	    else if(strcmp(num1, "") == 0) {
		    printf("Wrong Input!\n");
	    }
            else {
                if(operatorNum != 1) {
                    printf("Wrong Input!\n");
                }
                else {
                    if (operator == '+') {
                        syscall(__NR_plus,operand1,operand2,&result);

                        // 결과 출력
                        printf("Output: %d\n", result);
                    } 
                    else if (operator == '-') {
                        syscall(__NR_minus,operand1,operand2,&result);

                        // 결과 출력
                        printf("Output: %d\n", result);
                    } 
                    else {
                        printf("Wrong Input!\n");
                    }
                }
            }
        }
    }

    return 0;
}

void delete_space(char s[]) { //공백을 제거하는 함수
    char tmp[BUFFER_SIZE];
    int i, k = 0;

    for (i = 0; i < (int)strlen(s); i++) {
        if (!isspace((unsigned char)s[i])) { //만약 공백을 만난다면
            tmp[k++] = s[i]; //공백대신 문자를 저장
        }
    }

    tmp[k] = '\0';
    strcpy(s, tmp);
}
