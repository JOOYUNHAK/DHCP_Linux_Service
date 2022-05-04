#include "dhcp_h.h"

void dhcp_stop(char *argv[])
{
    sprintf(buf, "SELECT PID FROM pid_table WHERE NIC = '%s' LIMIT 1", argv[1]);
    mysql_query(connection, buf);
    result = mysql_store_result(connection);
    query = mysql_num_rows(result);
    row = mysql_fetch_row(result);
    mysql_free_result(result);

    if(query == 0)
    {
        printf("%s에 대한 DHCP 서비스가 실행중이 아닙니다\n", argv[1]);
        exit(0);
    }
    printf("DHCP 서비스를 중단합니다\n");
    char *str = (char *)row[0];
    kill( atoi(str), SIGTERM);
    sprintf(buf, "delete from pid_table where NIC = '%s'", argv[1]);
    mysql_query(connection, buf);
}


