#include "dhcp_h.h"

int main(int argc, char *argv[])
{

    mysql_init(&conn); //초기화
    if((connection = mysql_real_connect(&conn, DB_HOST, DB_USER, DB_PASSWD, DB_NAME, 0, (char *)NULL, 0)) == NULL)
    {
        fprintf(stderr, "MYSQL CONNECTION ERROR: %s\n", mysql_error(&conn));
        exit(1);
    }

    //다시 config file을 설정하고 싶을 때
    if(argv[1] != NULL) {
        if(argv[2] != NULL) {
            if(strcmp(argv[2], "start") == 0)
            {
                int num = is_setting(argv);
                if(num != 0) 
                { 
                    int db_pid;
                    if((db_pid = daemon_start(argv)) != 0)
                    {
                        insert_pid(db_pid, argv);
                        dhcp_start(argv);
                    }
                }
                else
                {
                    printf("%s의 네트워크 정보가 설정되어 있지 않습니다.\n", argv[1]);
                    exit(1);
                }
            }
            else if(strcmp(argv[2], "stop") == 0)
            {
                dhcp_stop(argv);
            }
            else if(strcmp(argv[2], "--status") == 0)
                dhcp_status(argv);
        }



        if(strcmp(argv[1], "-d") == 0)
            delete_network_setting(argv);
        //현재 할당된 IP LIST
        else if(strcmp(argv[1], "-s") == 0)
            search_ip_list(argv);      
        //SEARCH USED IP
        else if(strcmp(argv[1], "-u") ==0)
            search_used_ip();
        //NETWORK SETTING INFO
        else if(strcmp(argv[1], "-n") == 0)
            search_network_setting(argv);
        // 사용법
        else if(strcmp(argv[1], "--help") == 0)
            PrintManual();
        else if(strcmp(argv[1], "-c") == 0)
            network_setting(argv);
    }

    else
        printf("DHCP 서비스를 이용할 인터페이스 카드명을 입력해주세요\n");
    return 0;
}


