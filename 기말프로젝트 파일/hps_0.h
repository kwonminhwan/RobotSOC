#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

// FPGA base address
#define FPGA_BASE 0xFF200000
#define FPGA_SPAN 0x00200000

// Register offsets
#define LED_OFFSET 0x00000000
#define SEVENSEG_OFFSET 0x00000010

#define MAX_TRIES 10

void generate_random_number(int *num);
void get_user_input(int *num);
void check_guess(int *guess, int *answer, int *strike, int *ball);
void update_hardware(volatile unsigned int *base, int tries, int strike, int ball, int state);

int main() {
    int answer[3], guess[3];
    int strike, ball, tries = 0;
    int state = 0; // 0: playing, 1: clear, 2: end

    // FPGA memory mapping
    int fd;
    void *virtual_base;
    volatile unsigned int *led_base;
    volatile unsigned int *sevseg_base;

    if ((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1) {
        perror("Error opening /dev/mem");
        return 1;
    }

    virtual_base = mmap(NULL, FPGA_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, FPGA_BASE);
    if (virtual_base == MAP_FAILED) {
        perror("Error mapping FPGA memory");
        close(fd);
        return 1;
    }

    led_base = (volatile unsigned int *)(virtual_base + LED_OFFSET);
    sevseg_base = (volatile unsigned int *)(virtual_base + SEVENSEG_OFFSET);

    srand(time(0));
    generate_random_number(answer);

    while (tries < MAX_TRIES && state == 0) {
        printf("Enter your guess (3 digits): ");
        get_user_input(guess);

        check_guess(guess, answer, &strike, &ball);
        printf("Strike: %d, Ball: %d\n", strike, ball);

        if (strike == 3) {
            printf("You guessed the number! Game Clear!\n");
            state = 1; // clear
        }

        update_hardware(led_base, tries, strike, ball, state);
        tries++;
    }

    if (tries == MAX_TRIES && state == 0) {
        printf("No more tries left. Game End.\n");
        state = 2; // end
        update_hardware(led_base, tries, strike, ball, state);
    }

    munmap(virtual_base, FPGA_SPAN);
    close(fd);

    return 0;
}

void generate_random_number(int *num) {
    num[0] = rand() % 9 + 1;  // 첫 번째 숫자는 0이 될 수 없음
    do {
        num[1] = rand() % 10;
    } while (num[1] == num[0]);
    do {
        num[2] = rand() % 10;
    } while (num[2] == num[0] || num[2] == num[1]);
}

void get_user_input(int *num) {
    scanf("%1d%1d%1d", &num[0], &num[1], &num[2]);
}

void check_guess(int *guess, int *answer, int *strike, int *ball) {
    *strike = *ball = 0;
    for (int i = 0; i < 3; i++) {
        if (guess[i] == answer[i]) {
            (*strike)++;
        } else {
            for (int j = 0; j < 3; j++) {
                if (guess[i] == answer[j]) {
                    (*ball)++;
                }
            }
        }
    }
}

void update_hardware(volatile unsigned int *base, int tries, int strike, int ball, int state) {
    base[0] = (1 << tries) - 1; // Update LEDs
    base[4] = (strike << 8) | (ball << 4) | state; // Update 7-segment display
}
