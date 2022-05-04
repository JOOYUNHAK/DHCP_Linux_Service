#include "dhcp_h.h"

//RELEASE PACKET으로 끊기지 않고 비정상적으로 끊겨진 IP들에 대해 데이터베이스에 남아있기 때문에 현재시간보다 종료시간이 빠른 IP들 삭제
void checking_ip_time()
{
    if(mysql_query(connection, "CREATE TABLE IF NOT EXISTS backup(NIC VARCHAR(30) NOT NULL, ip_addr VARCHAR(20) NOT NULL, MAC_addr VARCHAR(20), network VARCHAR(20) NOT NULL, subnet VARCHAR(20) NOT NULL, gateway VARCHAR(20) NOT NULL, name_server VARCHAR(20) NOT NULL, default_lease_time VARCHAR(20) NOT NULL, start DATETIME, end DATETIME)" ))
    {
        fprintf(stderr, "CREATE backup table error: '%s'", mysql_error(&conn));
        exit(1);
    }

    if(mysql_query(connection, "SELECT * FROM ip_list WHERE now() > end"))
    {
        fprintf(stderr, "INSERT backup table error: '%s'\n", mysql_error(&conn));
        exit(1);
    }
    else
    {
        result = mysql_store_result(connection);
        while(row = mysql_fetch_row(result))
        {
            sprintf(buf, "INSERT INTO backup(NIC, ip_addr, MAC_addr, network, subnet, gateway, name_server, default_lease_time, start, end) values('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s','%s')", row[0],row[1], row[2], row[3], row[4], row[5], row[6], row[7],row[8], row[9]);
            mysql_query(connection, buf);
        }
        mysql_free_result(result);
    }

    if(mysql_query(connection, "UPDATE ip_list SET MAC_addr = NULL,start = NULL, end = NULL, Assigned = 0 WHERE now() > end"))
    {
        fprintf(stderr, "UPDATE backup table error: '%s'", mysql_error(&conn));
        exit(1);
    }

    //할당 해제된 IP들의 정보도 종료 시간으로부터 20분이 지나면 삭제
    if(mysql_query(connection, "DELETE FROM backup WHERE now() > DATE_ADD(end, INTERVAL 20 MINUTE)"))
    {
        fprintf(stderr, "DELETE backup table error: '%s'", mysql_error(&conn));
        exit(1);
    }
}


