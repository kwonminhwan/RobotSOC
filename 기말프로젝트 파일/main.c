#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

// FPGA base address
#define FPGA_BASE 0xFF200000
#define FPGA_SPAN 0x00200000

// Register offsets
#define LED_PIO_BASE 0x00010040
#define SEG7_IF_BASE 0x00010020

#define MAX_TRIES 9

void generate_random_number(int *num);
void get_user_input(int *num);
void update_hardware(volatile unsigned int *led_base, volatile unsigned int *sevseg_base, int tries, int strike, int ball, int state);

int main() {
    int answer[3], guess[3];
    int strike, ball, tries = 0;
    int state = 0; // 0: ���� ���� ��, 1: ���� ����, 2: ���� ����
    char input[10];

    // FPGA �޸� ����
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

    led_base = (volatile unsigned int *)(virtual_base + LED_PIO_BASE);
    sevseg_base = (volatile unsigned int *)(virtual_base + SEG7_IF_BASE);

    // Start �Է� ���
    printf("Type 'start' to begin the game: ");
    while (1) {
        scanf("%s", input);
        if (strcmp(input, "start") == 0) {
            // LED�� ��� ��
            led_base[0] = 0x1FF; // ��� 9���� LED�� ��
            // ���׸�Ʈ ���÷��̿� "000000" ǥ��
            sevseg_base[5] = 0x3F; // 0
            sevseg_base[4] = 0x3F; // 0
            sevseg_base[3] = 0x3F; // 0
            sevseg_base[2] = 0x3F; // 0
            sevseg_base[1] = 0x3F; // 0
            sevseg_base[0] = 0x3F; // 0
            break;
        }
        printf("Invalid input. Type 'start' to begin the game: ");
    }

    srand(time(0));
    generate_random_number(answer);
    int i;
    int j;

    while (tries < MAX_TRIES && state == 0) {
        printf("Enter your guess (3 digits): ");
        get_user_input(guess);

        strike = ball = 0; // ��Ʈ����ũ�� �� �ʱ�ȭ
        for (i = 0; i < 3; i++) {
            if (guess[i] == answer[i]) {
                strike++;
            } else {
                for (j = 0; j < 3; j++) {
                    if (guess[i] == answer[j]) {
                        ball++;
                    }
                }
            }
        }

        printf("Strike: %d, Ball: %d\n", strike, ball);

        if (strike == 3) {
            printf("You guessed the number! Game Clear!\n");
            state = 1; // ���� ����
        }

        tries++;
        update_hardware(led_base, sevseg_base, tries, strike, ball, state);
    }

    if (tries == MAX_TRIES && state == 0) {
        printf("No more tries left. Game End.\n");
        printf("The secret number was: %d%d%d\n", answer[0], answer[1], answer[2]);
        state = 2; // ���� ����
        update_hardware(led_base, sevseg_base, tries, strike, ball, state);
    }

    munmap(virtual_base, FPGA_SPAN); // �޸� ���� ����
    close(fd); // ���� ��ũ���� �ݱ�

    return 0;
}

void generate_random_number(int *num) {
    num[0] = rand() % 9 + 1;  // ù ��° �ڸ��� 0�� �� �� ����
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

void update_hardware(volatile unsigned int *led_base, volatile unsigned int *sevseg_base, int tries, int strike, int ball, int state) {
    // LED ������Ʈ: ���� ��ȸ�� ���� LED�� �Ѱų� ��
    if (tries == 0) {
        led_base[0] = 0x1FF; // ��� 9���� LED�� ��
    } else {
        // ���� ��ȸ�� �ش��ϴ� LED ��
        led_base[0] = 0x1FF >> tries; // ���� ��ȸ�� ���� LED�� ��
    }

    if (state == 1) {
        // 'CLEAR' ǥ��
        sevseg_base[5] = 0x39; // C
        sevseg_base[4] = 0x38; // L
        sevseg_base[3] = 0x79; // E
        sevseg_base[2] = 0x77; // A
        sevseg_base[1] = 0x50; // R
        sevseg_base[0] = 0x00; // ����
    } else if (state == 2) {
        // 'LOSE' ǥ��
        sevseg_base[5] = 0x38; // L
        sevseg_base[4] = 0x3F; // O
        sevseg_base[3] = 0x6D; // S
        sevseg_base[2] = 0x79; // E
        sevseg_base[1] = 0x00; // ����
        sevseg_base[0] = 0x00; // ����
    } else {
        // 7-���׸�Ʈ ���÷��� ������Ʈ
        // HEX5: 'S', HEX4: Strike, HEX3: 'b', HEX2: Ball, HEX1: 'O', HEX0: Out (remaining tries)
        unsigned int seg_patterns[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

        unsigned int strike_units = strike % 10;
        unsigned int ball_units = ball % 10;
        unsigned int out_units = 3 - (strike + ball);  // 'Out'�� ������ 3���� ��Ʈ����ũ�� ���� ���� �� ��

        sevseg_base[5] = 0x6D; // 'S'
        sevseg_base[4] = seg_patterns[strike_units];
        sevseg_base[3] = 0x7C; // 'b'
        sevseg_base[2] = seg_patterns[ball_units];
        sevseg_base[1] = 0x3F; // 'O'
        sevseg_base[0] = seg_patterns[out_units];
    }
}
