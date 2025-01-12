# Pintos
[CSE4070] Operating System

프로젝트별 개발내용

Prj1-
 User program Project(1) 에서는 함수 호출에 따른 Argument Passing을 구현하여, 커널 영역의 스택에 직접 push하고 이를 확인할 것이다. 또한 사용자 메모리 접근의 유효성을 검증하고, 시스템 콜을 요청 받았을 때 이것을 커널에서 직접 처리하는 것을 구현하여 커널과 사용자 프로그램 영역의 의사소통 개념을 이해하고 운영체제에서의 처리과정을 학습, 확장하는 것을 목표로 한다.

Prj2-
 Project 1에서 개발한 시스콜, 파일 loading을 바탕으로, 더 필요한 System call들을 개발한다. 이 때 개발에 필요한 파일 시스템들에 대해 쓰레드, 프로세스 간 발생할 수 있는 Syncronization 문제를 해결을 목표로 하여 개발한다.
 
Prj3-
1.	Alarm Clock
핀토스의 디폴트 스케줄러는 busy-waiting으로 되어 있다. 이렇게 비효율적으로 구현되어 있는 핀토스의 스케줄러를 tick단위로 계산되는 시간을 활용해 일정 시간마다 thread가 동작하지 않으면 sleep 시키는 스케줄링으로 구현한다.
2.	Priority Scheduling
핀토스의 디폴트는 Round-Robin 방식으로 스케줄링이 구현되어 있다. 이는 실행중인 thread가 ready_list로 들어갈 때 항상 ready_list의 맨 뒤로 fifo 방식으로 push되는 방식이다. 이는 각 thread의 우선순위를 따지지 못하고 항상 번갈아 실행해야하는 단점이 있다. 이 단점을 보완하기 위해 prirority scheduling을 구현하여 preemptive scheduling을 할 수 있도록 한다. 추가적으로 기아 문제를 해결하기 위해 aging도 구현한다.

Prj4-
1.	Page Table & Page Fault Handler
Page fault에 대해서 exit 하지않고 스왑과 페이지 테이블 관리를 통해 reliable하게 구현한다. 적절한 접근인지 확인하는 로직 등을 추가하고 최종적으로 Page Fault Handler가 제 역할을 하게 한다.
2.	Disk Swap
DRAM 메모리가 부족한 상황에서 디스크의 용량을 사용하는 것을 Swap이라고 하는데, 이를 LRU 알고리즘을 구현하여 해결한다.
3.	Stack Growth
스택에 접근하는 중 스택이 꽉 차거나 Page fault가 일어날 때 스택 Growth를 구현하여 크기를 조절하고 적절한 할당으로 효율을 상향하도록 한다.
