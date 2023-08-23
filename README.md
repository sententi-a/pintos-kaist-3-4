## Memory Management
<div style="color: red">Goal</div>
가상 메모리 시스템의 필수 요소인 page(virtual), frame(physical) 관련 함수를 구현해 메모리 공간을 효율적으로 관리한다.
![image](https://github.com/sententi-a/pintos-kaist-3-4/assets/77879373/81f6385e-30ef-4753-ab5e-0f9182f03b15)


## Anonymous Page
<div style="color: red">Goal</div>
프로세스 생명 주기(exec, fork, exit)에 따라 페이지를 할당하고, 프레임과 매핑하고(lazy load), 삭제(destroy)한다.
![image](https://github.com/sententi-a/pintos-kaist-3-4/assets/77879373/124f7820-83dc-4279-af35-ca8d33b954ae)

## Stack Growth
<div style="color: red">Goal</div>
단일 페이지의 스택이 부족할 때 생성되는 page fault를 핸들링하여, 적법한 access인지 확인한다. 그리고 추가적인 스택 공간을 위한 1개 이상의 페이지(최대 1MB)를 할당한다.
![image](https://github.com/sententi-a/pintos-kaist-3-4/assets/77879373/6da4aabe-d260-4f63-aafe-323791133b86)


## Memory Mapped Files
<div style="color: red">Goal</div>
가상 메모리 영역을 디스크의 파일과 연결해 초기화 하는, 메모리 매핑을 위한 mmap과 munmap을 구현한다. 
