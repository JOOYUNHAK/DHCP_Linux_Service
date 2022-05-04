#include "dhcp_h.h"

int daemon_start(char *argv[])
{
    pid_t pid; int fd;  int pid_value;
    if(dhcp_islive(argv) == ERROR)
    {
        fprintf(stdout, "에러 원인: %s에 대한 DHCP SERVICE가 예상치 못하게 종료된 상태입니다. stop 옵션을 이용하여 종료 후 재시작 하여 주시기 바랍니다\n", argv[1]);
        exit(0);
    }

    if((pid = fork()) < 0)
        return -1;
    else if(pid != 0) 
        exit(0);
    //이미 실행중인지 여부 판단
   sprintf(buf, "SELECT NIC FROM pid_table WHERE NIC = '%s' LIMIT 1", argv[1]);
    mysql_query(connection, buf);
    result = mysql_store_result(connection);
    query = mysql_num_rows(result);
    mysql_free_result(result);
    if( query != 0) 
    {
        printf("이미 %s에 대한 DHCP SERVICE가 동작중입니다\n", argv[1]);
        exit(0);
    }
    
    printf( "DHCP 서비스를 시작합니다\n");
    pid_value = getpid(); //DHCP PROCESS
    setsid();
    chdir("/");
    fd = open("dev/null", O_RDWR);
    dup2(fd,2);dup2(fd,0);
    close(fd);
    umask(0);
    return pid_value;
}


