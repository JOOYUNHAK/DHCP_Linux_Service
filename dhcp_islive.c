#include "dhcp_h.h"

int dhcp_islive(char *argv[])
{
    
    sprintf(buf, "SELECT PID FROM pid_table WHERE NIC = '%s'", argv[1]);
    if(mysql_query(connection, buf));
    result = mysql_store_result(connection);
    query = mysql_num_rows(result);
    row = mysql_fetch_row(result);
    mysql_free_result(result);    

    if(query != 0)
    {
        char process[50];
        sprintf(process, "/proc/%s/status", row[0]);
        FILE *fp = fopen(process, "r");
        if(fp == NULL)
        {
            return ERROR;
        }  
        fclose(fp);
    return TRUE;
    }
    else if( query == 0)
        return FALSE;
}


