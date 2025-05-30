# 🎮 Bulls and Cows - 숫자 야구 게임 (FPGA + HPS 기반)

**팀원**  
- 2019265006 권민환  
- 2019265093 이상학  

---

## 📌 프로젝트 개요

본 프로젝트는 **DE1-SoC FPGA 보드**와 **Ubuntu 환경에서 실행되는 C 코드**를 연동하여,  
클래식 숫자 야구 게임(Bulls and Cows)을 구현한 시스템입니다.  
사용자는 3자리 숫자를 입력하고, 시스템은 정답과 비교하여 스트라이크, 볼, 아웃 개수를 LED 및 7-segment로 표시합니다.

---

## 🎯 개발 동기

- 누구나 쉽게 즐길 수 있는 대중적인 게임 선택  
- 단순한 게임에 **FPGA의 입출력 제어 기능**을 접목  
- LED와 7-Segment를 적극 활용하여 게임 반응을 시각적으로 표현

---

## 🕹️ 게임 규칙

- 시스템이 **3자리 무작위 숫자**를 생성  
- 사용자 입력 숫자와 정답 비교  
  - 숫자와 위치가 일치 → **스트라이크**
  - 숫자만 일치, 위치 다름 → **볼**
  - 전부 불일치 → **아웃**
- 총 **9번의 기회** 안에 맞추면 승리, 아니면 패배

---

## ⚙️ 시스템 구성

| 구성 요소 | 역할 |
|-----------|------|
| **Ubuntu 환경** | 게임 실행, 사용자 입력 처리, 정답 생성 |
| **C 코드 (main.c)** | 숫자 비교 로직 처리, 하드웨어 업데이트 |
| **FPGA LED** | 남은 기회 표시 |
| **7-Segment** | Strike/Ball/Out 출력 또는 게임 종료 시 메시지 출력 |
| **Quartus** | FPGA 핀 설정 및 하드웨어 인터페이스 구성 |

---

## 🧠 핵심 코드 요약

### ▪️ 정답 생성 함수
```c
void generate_random_number(int *num) {
    num[0] = rand() % 9 + 1;
    do { num[1] = rand() % 10; } while (num[1] == num[0]);
    do { num[2] = rand() % 10; } while (num[2] == num[0] || num[2] == num[1]);
}
```

### ▪️ 비교 및 결과 출력
```c
void update_hardware(...) {
    if (strike == 3) {
        // 7-segment에 'CLEAR' 출력
    } else if (tries == 9) {
        // 7-segment에 'LOSE' 출력
    } else {
        // S, B, O 개수를 7-segment에 출력
    }
}
```

---

## 📸 실행 결과

- `start` 입력 시 LED 및 세그먼트 초기화
- 게임 종료 후 승리 시 → "CLEAR", 패배 시 → "LOSE" 출력
- 중간 시도 시 → `SxBxOx` 형태로 결과 출력
- UART 통신(Putty)로 결과 및 로그 확인 가능

---

## 📽️ 시연 영상

- 승리 영상 포함
- 패배 영상 포함

---

## 💡 배운 점 및 느낀 점

- FPGA와 7-Segment 제어를 통해 **하드웨어 제어의 기본 개념** 이해
- Quartus 툴 및 핀 설정, Memory Mapping 등 실습
- Ubuntu 환경과 C 코드의 **cross-compilation** 개념 체득
- 단순한 게임을 통해 **FPGA와 HPS 간의 상호작용 구조**에 대한 감각을 익힘

> 제어와 연동을 동시에 고려하며 설계해야 하는 점이 도전이었지만,  
> 기능이 하나씩 동작하며 게임처럼 작동할 때의 재미와 보람이 컸습니다.

---

## 📂 폴더 구성

```
project/
├── Bulls_and_Cows_2019265006_2019265093/  # FPGA 회로 및 출력 구성 (Quartus)
├── 기말프로젝트 파일/
│   ├── main.c                # 게임 로직 처리 C 코드
│   ├── soc_system.qsys       # 시스템 통합 설계 파일
│   ├── HPS_LED_HEX.qpf       # Quartus 프로젝트 파일
```

---

## ✅ 사용 환경

- **보드**: DE1-SoC FPGA (Intel)
- **IDE**: Quartus Prime + Ubuntu (터미널 기반)
- **언어**: C
- **하드웨어 연동**: Memory-Mapped I/O, mmap
- **입출력**: Switch, LED, 7-Segment
