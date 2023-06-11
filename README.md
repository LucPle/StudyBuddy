# StudyBuddy

## 2023 Spring System Programming and Practice Team Project: StudyBuddy
* 프로젝트 개요: Raspberry Pi 환경에서 Linux의 functions를 활용하여 Sensor – Actuator를 이용하는 프로그램 개발
* 프로젝트 기간: 2023.05.01 ~ 2023.06.12
* 프로젝트 이름: StudyBuddy
* 프로젝트 목표: 사용자의 학습 환경을 지속적으로 피드백하는 프로그램 제작 

## StudyBuddy

### 동작 원리
  1. 사용자가 의자에 앉으면, 타이머로 원하는 시간을 설정한다.
  2. 타이머를 시작하면 지속적으로 사용자의 착석 자세를 트래킹하고, 주변환경을 피드백한다.
  3. 문제가 있는 경우 LED로 즉각적인 피드백을 표시하여 교정 혹은 개선할 수 있도록 도움을 준다.
  4. 타이머가 끝나면 소켓을 닫고 통신을 종료한다.

### 파이 구성
1. 타이머 파이: 버튼을 이용하여 타이머를 설정. 타이머는 1분 단위로 올리거나, 내려서 설정할 수 있고 LCD로 남은 시간을 표시
    - LCD에 남은 시간을 표시, LED에 현재 상황을 표시
2. 자세측정 파이: 사용자의 착석을 감지하고, 초음파 센서를 이용하여 학습 자세가 올비른지 확인하고 피드백
    - 초음파 센서는 4개 사용 (의자의 좌우-상체, 의자-허리, 책상-상체 간의 간격 측정)
3. 얼굴인식 파이: 카메라 센서로부터 실시간 영상을 입력받고, 사용자가 공부에 집중하고 있는지 확인하고 피드백
    - opencv를 이용하여 영상처리
4. 환경측정 파이: 온습도와 조도를 측정하여 사용자의 공부 환경이 쾌적한지 측정하고 피드백

### Signal 정리
- <span style="color:red"> RED: 자세 불량 확인 </span>
- <span style="color:yellow"> YELLOW: 집중 확인 </span>
- <span style="color:white"> WHITE: 조도 확인 </span>
- <span style="color:blue"> BLUE: 온습도 확인 </span>

### 전체 구성
* server: LCD/LED 파이
* client: 카메라, 초음파/압력, 온습도/조도 파이
<img width="600" alt="img1" src="./img/raspi_communication.PNG">

## 컴파일 방법
* 타이머 파이 (lcd 폴더 내 gpio.c, gpio.h, lcd.c, lcd.h, lcd_pi.c)
  ```C
  gcc -o lcd * -lpthread -lwiringPi
  ./lcd [Port 번호]
  ```
* 자세측정 파이
  ```C
  gcc -o ultrasonic_wave ultrasonic_wave.c -lpthread
  ./ultrasonic_wave [IP주소] [Port 번호]
  ```
* 환경측정 파이
  ```C
  gcc -o use_socket use_socket.c -lpthread -lwiringPi
  ./use_socket [IP주소] [Port 번호]
  ```
* 얼굴인식 파이
  ```Python3
  python3 camera.py [IP주소] [Port 번호]
  ```

## 회의 내용

### 5/19 회의
* 시프 proposal 발표 정리
* 피드백 내역
  1. 프로젝트 규모에 따른 다양한 센서 사용
  2. 명확한 프로젝트 목표 및 구현 가능성
  3. 4대가 유기적으로 사용되게 구현 바람 
  4. 적외선 센서 혹은 카메라 센서 도입 검토

* 토론 내역
  1. DC모터를 이용한 바람개비
  2. LED 사용 구상
    * 온도, 습도 각각 하나
    * 조도 하나
    * 초음파 하나
    * 카메라 하나
    
### 5/26 회의
* 파트 정리
  * 준서: 카메라 테스트해보고 가능하면 사용
  * 다인: 초음파(5개)+압력센서(1개) — 압력센서/ 초음파 센서 양 옆 (2)+ y축(3) 
  * 종호: LCD 출력 + LED — 버튼 이용한 타이머 설정(LED 점등) + 타이머 보여주기/  통신 입력
  * 지영: 온습도+ 조도 + 바람개비(온도) -온습도 센서 파악 + 바람개비 설정 / 조도 센서 파악
   
* 통신 정리
  * start: 압력센서 → 카메라, LCD, 온습도/조도에 signal
  * 이후 카메라, 초음파, 온습도/조도 → LED로 signal
  * LCD 측에서 버튼을 누르면 데이터 전송을 중단

### 6/2 회의
* 통신 세부 정리
  * 처음에 앉았을 때 압력센서에 신호 → lcd에 신호
  * 종호님이 준서님이랑 지영님, 다인님 다시 신호

-> 카메라 따로
-> 불쾌 지수 센서(2)/ 조도 센서 신호(1) 둘 (3)
-> 다인님 초음파센서 자세 신호

-> lcd파트에서 LED
-> 타이머 종료되면 다른 파이에 신호
-> 중간에 종료되어도 다른파이에 신호

### 6/7 회의
* server - client 소켓 정리
* server에서 thread를 3개 열어서 초음파, 카메라, 온습도/조도와 소켓 통신
* C - Python간 통신 방법 정리
* 카메라 - opencv + openpose (MobileNet기반) 시도

### 6/9 회의
* 구현 영상 촬영
* docs, ppt 구성 토론 및 작성
