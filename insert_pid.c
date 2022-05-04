#include "dhcp_h.h"

//해당 인터페이스 카드의 PID를 넣어놓는다.서비스를 종룔할 때 해당 PID값을 죽임
void insert_pid(int value, char *argv[])
{

    if(mysql_query(connection, "CREATE TABLE IF NOT EXISTS pid_table(NIC VARCHAR(30), PID INT)" ))
    {
        fprintf(stderr, "INSERT PID ERROR: '%s'", mysql_error(&conn));
        exit(1);
    }
    sprintf(buf, "INSERT INTO pid_table(NIC, PID) VALUES ('%s','%d')",argv[1], value);
    mysql_query(connection, buf);
    return;
}


