#include "dhcp_h.h"

//설정한 네트워크 정보 조회
void search_network_setting(char *argv[])
{
    if(argv[2] == NULL)
    {
        printf("조회할 네트워크 인터페이스명을 입력해주세요\n");
        exit(1);
    }
    else
    {
        char *nic_name = argv[2];
        sprintf(buf, "SELECT * FROM ip_list WHERE NIC = '%s' LIMIT 1", argv[2]);
        if(mysql_query(connection, buf))
        {
            fprintf(stderr, "NETWORK 정보 조회 실패: '%s'", mysql_error(&conn));
            exit(1);
        }
        result = mysql_store_result(connection);
        query = mysql_num_rows(result);
        row = mysql_fetch_row(result);
        mysql_free_result(result);
        
        if(query == 0)
        {
            printf("%s에 대한 네트워크 설정이 아직 안되었습니다\n", argv[2]);
            exit(1);
        }

        printf("네트워크 인터페이스명: %s\n", row[0]);
        printf("IP 범위: %s ~ ", row[1]);
        memset(buf,0,sizeof(buf));
        sprintf(buf, "SELECT * FROM ip_list WHERE NIC = '%s' order by ip_addr desc LIMIT 1", argv[2]);
        mysql_query(connection, buf);
        result = mysql_store_result(connection);
        row = mysql_fetch_row(result);
        mysql_free_result(result);
        printf("%s\n", row[1]);
        printf("네트워크 대역: %s\n", row[3]);
        printf("SUBNET MASK: %s\n", row[4]);
        printf("GATEWAY: %s\n", row[5]);
        printf("DOMAIN NAME SERVER: %s\n", row[6]);
        printf("LEASE TIME: %s\n", row[7]);
    }
}


