#include "dhcp_h.h"

//네트워크 정보가 세팅되어 있는지 검사
int is_setting(char *argv[])
{  
    sprintf(buf, "SELECT * FROM ip_list WHERE NIC = '%s'", argv[1]);
    mysql_query(connection, buf);
    result = mysql_store_result(connection);
    query = mysql_num_rows(result);
    mysql_free_result(result);
    return query;         
}


