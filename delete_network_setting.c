#include "dhcp_h.h"

//네트워크 정보 초기화
void delete_network_setting(char * argv[])
{
    char answer;
    if(argv[2] != NULL)
    {
        printf("%s에 관한 네트워크 정보들을 지우겠습니까?(y/n) ", argv[2]);
        scanf("%c", &answer);
        if(answer == 'y' || answer == 'Y')
        {
            sprintf(buf, "DELETE FROM ip_list WHERE NIC = '%s'", argv[2]);
            if(mysql_query(connection, buf) == 0)
            {
                printf("삭제에 성공했습니다.\n");
                exit(1);
            }
            else
            {
                printf("삭제에 실패했습니다. 원인: %s\n", mysql_error(&conn));
                exit(1);
            }
        }
        printf("정보를 계속 유지합니다.\n");
        exit(1);   
    }
    printf("재설정할 네트워크 인터페이스명을 같이 입력해주세요\n");
    exit(1); 
}


