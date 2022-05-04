#include "dhcp_h.h"

int CountIp(char * start, char * end)
{
    //몇개의 ip갯수를 할당할 수 있는지
    char * s = start;
    char * e = end;
    
    sprintf(buf, "SELECT INET_ATON('%s') - INET_ATON('%s')", e, s);
    mysql_query(connection, buf);
    result = mysql_store_result(connection);
    row = mysql_fetch_row(result);
    mysql_free_result(result);   
    return atoi(row[0]);
}


