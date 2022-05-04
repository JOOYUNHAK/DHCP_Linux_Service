#include "dhcp_h.h"

void dhcp_status(char *argv[])
{
    int state = dhcp_islive(argv);
    if(state == FALSE )
        printf("STATUS: \033[33m inActive (DEAD) \033[0m\n");
    else if(state == ERROR)
    {
        printf("STATUS: \033[31m inActive (ERROR) \033[0m\n");
        fprintf(stdout, "에러 원인: %s에 대한 DHCP SERVICE가 예상치 못하게 종료된 상태입니다. stop 옵션을 이용하여 종료 후 재시작 하여 주시기 바랍니다\n", argv[1]);
    }
    else
        printf("STATUS: \033[32m Active (RUNNING) \033[0m\n");
    
    return;
}


