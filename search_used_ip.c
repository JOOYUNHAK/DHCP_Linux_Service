#include "dhcp_h.h"

//사용되었던 ip 구하기
void search_used_ip()
{
    checking_ip_time();
    if(mysql_query(connection, "SELECT * FROM backup order by end"))
    {
        fprintf(stderr, "사용한 ip조회 실패: '%s'\n", mysql_error(&conn));
        exit(1);
    }
    result = mysql_store_result(connection);
    query = mysql_num_rows(result); //20개 이상이면 삭제 update
    if(query > 20)
        mysql_query(connection, "DELETE * FROM backup ORDER BY end DESC LIMIT 1");
        
    printf("----------20분 내에 할당 해제된 ip list입니다----------\n");
    printf("%- 3s\t%-15s\t%-15s\t\t%-15s\t\t%-15s\t\t%15s\n", "순서", "NIC", "ip주소", "MAC주소", "시작 시간", "종료 시간");
    int i = 1;
    while(row = mysql_fetch_row(result))
    {
        if(row[9] != NULL)
        {
            sprintf(buf, "SELECT (unix_timestamp(now()) - unix_timestamp(end)) FROM backup WHERE end = '%s'", row[9]);
            mysql_query(connection, buf);
            result2 = mysql_store_result(connection);
            row2 = mysql_fetch_row(result2);
            int p_time = atoi(row2[0]);
            int p_minute = p_time/60;
            int p_second = p_time%60;
            mysql_free_result(result2);

            printf("%- 3d\t%-15s\t%-15s\t\t%-15s\t%-15s\t  %15s  -->  %d분 %d초 전에 ip가 해제되었습니다\n", i, row[0], row[1], row[2], row[8], row[9], p_minute, p_second);     
            i++;
        }
    }
    mysql_free_result(result);
    exit(1);
}


