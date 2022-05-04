#include "dhcp_h.h"

void PrintManual()
{
    printf("OPTIONS\n");
    printf("\t -c [NETWORK INTERFACE CARD]\n");
    printf("\t\t --입력한 네트워크 인터페이스 카드의 네트워크 정보를 설정한다.\n");
    printf("\t -d [NETWORK INTERFACE CARD]\n");
    printf("\t\t-- 입력한 네트워크 인터페이스 카드의 네트워크 정보를 삭제한다.\n");
    printf("\t -s [NETWORK INTERFACE CARD]\n");
    printf("\t\t --입력한 네트워크 인터페이스 카드에 대해 할당된 주소들을 보여준다.\n"); 
    printf("\t -u \n");
    printf("\t\t --할당되었다가 해제된 ip들의 정보를 보여준다.\n");
    printf("\t -n [NETWORK INTERFACE CARD]\n");
    printf("\t\t --입력한 네트워크 인터페이스 카드에 대해 설정된 네트워크 정보를 보여준다.\n");
    printf("\t [NETWORK INTERFACE CARD] [start | stop] \n");
    printf("\t\t --입력한 네트워크 인터페이스 카드에 대한 DHCP 서비스를 시작하고 중단한다.\n");
    printf("\t [NETWORK INTERFACE CARD] --status \n");
    printf("\t\t --입력한 네트워크 인터페이스 카드에 대한 DHCP 서비스의 동작여부를 알 수 있다.\n");


    exit(1);
}


