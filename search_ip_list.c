#include "dhcp_h.h"

void search_ip_list(char *argv[])
{
    checking_ip_time();
    if(argv[2] == NULL)
    {
        printf("조회할 네트워크 인터페이스명을 입력해주세요\n");
        exit(1);
    }
    else
    {
        char *nic_name = argv[2];
        //할당 해제된 IP들 구하기
        sprintf(buf, "SELECT * FROM ip_list WHERE NIC = '%s' AND Assigned = 1", argv[2]);
        if(mysql_query(connection, buf))
        {   
            fprintf(stderr, "ip조회 실패: %s/\n", mysql_error(&conn));
            exit(1);
        }
        printf("----------현재 ip 할당 list입니다----------\n");
        printf("%- 3s\t%-15s\t%-15s\t\t%-15s\t\t%-15s\t\t%15s\n", "순서", "NIC", "ip주소", "MAC주소", "시작 시간", "종료 시간");
        result = mysql_store_result(connection);
        int i = 1;
        while((row = mysql_fetch_row(result)) != NULL)
        {
            printf("%- 3d\t%-15s\t%-15s\t\t%-15s\t%-15s\t  %15s\n", i, row[0], row[1], row[2], row[8], row[9]);     
            i++;
        }
        mysql_free_result(result);
        exit(1);
    }
}


