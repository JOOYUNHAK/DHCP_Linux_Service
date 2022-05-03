#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <signal.h>
#include "bootp.h"
#include </usr/include/mysql/mysql.h>


#define BUFF_SIZE 4096
#define TRUE 1
#define FALSE 0
#define DB_HOST "127.0.0.1"
#define DB_USER "root"
#define DB_PASSWD "1234"
#define DB_NAME "IP_Management"

#define DHCP_DISCOVER 0x01
#define DHCP_OFFER 0x02
#define DHCP_REQUEST 0x03
#define DHCP_ACK 0x05
#define DHCP_NAK 0x06
#define DHCP_RELEASE 0x07

#define BOOT_REPLY 0x02
#define DHCP_MESSAGE 0x35
#define DHCP_SUBNET 0x01
#define DHCP_ROUTER 0x03
#define DHCP_DOMAIN 0x06

#define DHCP_END 0xff

MYSQL *connection = NULL; MYSQL conn;
MYSQL_RES *result;  MYSQL_ROW row;  MYSQL_RES *result2; MYSQL_ROW row2;
int query;  char start_ip_addr[20]; char end_ip_addr[20];   char network[20]; char subnet[20]; char gateway[20];
char name_server[20]; char default_lease_time[10]; char max_lease_time[10]; int num; char NIC[20];
unsigned char client_mac_addr[40]; char db_my_ip[20]; char buf[BUFF_SIZE];   
struct ifreq ifr;

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

void PrintManual()
{
    printf("OPTIONS\n");
    printf("\t -c [NETWORK INTERFACE CARD]\n");
    printf("\t\t --입력한 네트워크 인터페이스 카드의 네트워크 정보를 설정한다.\n");
    printf("\t -d [NETWORK INTERFACE CARD]\n");
    printf("\t\t-- 입력한 네트워크 인터페이스 카드의 네트워크 정보를 삭제한다.\n");
    printf("\t -s [NETWORK INTERFACE CARD]\n");
    printf("\t\t --입력한 네트워크 인터페이스 카드에 대해 할당된 주소들을 보여준다.\n"); 
    printf("\t -u \n");
    printf("\t\t --할당되었다가 해제된 ip들의 정보를 보여준다.\n");
    printf("\t -n [NETWORK INTERFACE CARD]\n");
    printf("\t\t --입력한 네트워크 인터페이스 카드에 대해 설정된 네트워크 정보를 보여준다.\n");
    printf("\t [NETWORK INTERFACE CARD] [start | stop] \n");
    printf("\t\t --입력한 네트워크 인터페이스 카드에 대한 DHCP 서비스를 시작하고 중단한다.\n");
    printf("\t [NETWORK INTERFACE CARD] --status \n");
    printf("\t\t --입력한 네트워크 인터페이스 카드에 대한 DHCP 서비스의 동작여부를 알 수 있다.\n");


    exit(1);
}

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

int dhcp_islive(char *argv[])
{
    
    sprintf(buf, "SELECT PID FROM pid_table WHERE NIC = '%s'", argv[1]);
    if(mysql_query(connection, buf));
    result = mysql_store_result(connection);
    row = mysql_fetch_row(result);
    mysql_free_result(result);    

    char process[50];
    sprintf(process, "/proc/%s/status", row[0]);
    FILE *fp = fopen(process, "r");
    if(fp == NULL)
    {
        fprintf(stdout, "'%s'에 대한 DHCP SERVICE가 예상치 못하게 종료되었습니다.\nstop 옵션을 이용하여 정상적으로 종료 후 다시 시작하여 주시기 바랍니다.\n", argv[1]);
        return FALSE;
    }  
    fclose(fp);

    return 2;
}


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

//네트워크 정보 초기화
void delete_network_setting(char * argv[])
{
    char answer;
    if(argv[2] != NULL)
    {
        printf("%s에 관한 네트워크 정보들을 지우겠습니까?(y/n) ", argv[2]);
        scanf("%c", &answer);
        if(answer == 'y' || answer == 'Y')
        {
            sprintf(buf, "DELETE FROM ip_list WHERE NIC = '%s'", argv[2]);
            if(mysql_query(connection, buf) == 0)
            {
                printf("삭제에 성공했습니다.\n");
                exit(1);
            }
            else
            {
                printf("삭제에 실패했습니다. 원인: %s\n", mysql_error(&conn));
                exit(1);
            }
        }
        printf("정보를 계속 유지합니다.\n");
        exit(1);   
    }
    printf("재설정할 네트워크 인터페이스명을 같이 입력해주세요\n");
    exit(1); 
}

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

//네트워크 정보 세팅
void network_setting(char *argv[])
{
    //Config file이 존재할때
    if(mysql_query(connection, "CREATE TABLE ip_list(NIC VARCHAR(30) NOT NULL, ip_addr VARCHAR(20) NOT NULL, MAC_addr VARCHAR(20), network VARCHAR(20) NOT NULL, subnet VARCHAR(20) NOT NULL, gateway VARCHAR(20) NOT NULL, name_server VARCHAR(20) NOT NULL, default_lease_time VARCHAR(20) NOT NULL, start DATETIME, end DATETIME, Assigned INT DEFAULT 0, constraint ip_list_PK primary key(NIC, ip_addr))" ))
    {
        if(mysql_query(connection, "CREATE TABLE tmp(ip INT UNSIGNED, NIC VARCHAR(30), constraint tmp_PK PRIMARY KEY(ip, NIC))"))
        {
            mysql_query(connection, "DROP TABLE tmp");
            mysql_query(connection, "CREATE TABLE IF NOT EXISTS tmp(ip INT UNSIGNED , NIC VARCHAR(30), constraint tmp_PK PRIMARY KEY(ip, NIC))"); 
        }

        sprintf(buf, "SELECT NIC, network FROM ip_list WHERE NIC = '%s' LIMIT 1", argv[2]);
        if(mysql_query(connection, buf))
        {   
            fprintf(stderr, "NIC ERROR: %s\n", mysql_error(&conn));
            exit(1);
        }
        memset(buf, 0, sizeof(buf));
        result = mysql_store_result(connection);
        query = mysql_num_rows(result);
        row = mysql_fetch_row(result); 
        mysql_free_result(result);

        if(query == 0)  //설정 도중 설정을 멈춰서 데이터가 정상적으로 입력이 안되었으면
            goto InputNetwork;
        else    //설정 정보가 이미 존재한다면
            fprintf(stderr, "이미 %s에 대한 네트워크 정보 %s가 존재합니다. 동일한 NIC에 대해 재설정을 원하시면 -d옵션을 이용해주세요\n", row[0], row[1]);     
        exit(1);  
    }

    if(mysql_query(connection, "CREATE TABLE tmp(ip INT UNSIGNED, NIC VARCHAR(30), constraint tmp_PK PRIMARY KEY(ip, NIC))"))
    {
        mysql_query(connection, "DROP TABLE tmp");
        mysql_query(connection, "CREATE TABLE IF NOT EXISTS tmp(ip INT UNSIGNED , NIC VARCHAR(30), constraint tmp_PK PRIMARY KEY(ip, NIC))");
    }
InputNetwork: 
    printf("네트워크 대역(ex: 100.20.30.0):");
    scanf("%s", network);    
    //fgets(network, 20, stdin);
    while(fgetc(stdin) != '\n');
    printf("IP 시작 주소(ex: 100.20.30.10):");
    //fgets(start_ip_addr, 20, stdin);
    scanf("%s", start_ip_addr);    
    while(fgetc(stdin) != '\n');
    printf("IP 끝 주소(ex: 100.20.30.50):");
    //fgets(end_ip_addr, 20, stdin);
    scanf("%s", end_ip_addr);    
    while(fgetc(stdin) != '\n');
    printf("SubNet Mask(ex: 255.255.255.0):");
    //fgets(subnet, 20, stdin);
    scanf("%s", subnet);    
    while(fgetc(stdin) != '\n');
    printf("GateWay(ex: 100.20.30.1):");
    scanf("%s", gateway);    
    //fgets(gateway, 20, stdin);
    while(fgetc(stdin) != '\n');
    printf("domain-name-server(ex: 8.8.8.8):");
    //fgets(name_server, 20, stdin);
    scanf("%s", name_server);    
    while(fgetc(stdin) != '\n');
    printf("default_lease_time:");
    //fgets(default_lease_time, 10, stdin);
    scanf("%s", default_lease_time);    
    while(fgetc(stdin) != '\n');

    int count = CountIp(start_ip_addr, end_ip_addr);    //몇개의 ip범위가 나오는지
    if(count < 0)
    {
        //다시 시작할 때 인터페이스 카드 값
        printf("ip범위가 잘못되었습니다. 다시 설정해주세요\n");
        goto InputNetwork;
    }

    //SETTING IP DB INSERT 
    int z;
    for(z = 0; z <= count; z++)
    {
        if(z == 0)
        {
            sprintf(buf, "INSERT INTO ip_list(NIC, ip_addr, network, subnet, gateway, name_server, default_lease_time,Assigned) VALUES" " ('%s', '%s', '%s','%s','%s','%s','%s','%d')",argv[2], start_ip_addr, network, subnet, gateway, name_server, default_lease_time,0);
            if(mysql_query(connection, buf) != 0)
            {
                fprintf(stderr, "MYSQL ERROR %s", mysql_error(&conn));
                exit(1);
            }
            printf("Network 설정이 완료되었습니다\n");
        }
        else
        {

            sprintf(buf, "SELECT INET_NTOA(ip) FROM tmp WHERE NIC = '%s' ORDER BY (ip) DESC LIMIT 1", argv[2]);
            
            if(mysql_query(connection, buf))
            {
                fprintf(stderr, "SELECT ERROR: %s", mysql_error(&conn));
                exit(1);
            }

            result = mysql_store_result(connection);
            row = mysql_fetch_row(result);
            mysql_free_result(result);

            sprintf(buf, "INSERT INTO ip_list(NIC, ip_addr, network, subnet, gateway, name_server, default_lease_time, Assigned) VALUES" " ('%s', '%s', '%s','%s','%s','%s','%s','%d')",argv[2],  row[0], network, subnet, gateway, name_server, default_lease_time,0);
            if(mysql_query(connection, buf) != 0)
            {
                fprintf(stderr, "SECOND INSERT ERROR:%s", mysql_error(&conn));
                exit(1);
            }
        }
 
       // insert ip_list
        sprintf(buf,  "INSERT INTO tmp(ip, NIC) VALUES((SELECT INET_ATON(ip_addr) FROM ip_list WHERE NIC = '%s'  order by inet_aton(ip_addr) DESC LIMIT 1)+ 1, '%s')", argv[2],argv[2]);
        if(mysql_query(connection, buf))
        {
            fprintf(stderr, "INSERT ERROR: %s\n", mysql_error(&conn));
            exit(1);
        }
    }
    //임시 테이블 삭제
    char* delete_query = "DROP TABLE tmp";  
    if(mysql_query(connection, delete_query))
    {
        fprintf(stderr, "DROP ERROR: %s", mysql_error(&conn));
        exit(1);
    }
}

void dhcp_status(char *argv[])
{
    if(dhcp_islive(argv) == FALSE)
        exit(0);
    else{
    sprintf(buf, "SELECT PID FROM pid_table WHERE NIC = '%s'", argv[1]);
    if(mysql_query(connection, buf))

    {
        fprintf(stderr, "STATUS ERROR: '%s'\n", mysql_error(&conn));
        exit(0);
    }
    result = mysql_store_result(connection);
    query = mysql_num_rows(result);
    
    if(query == 0)
        printf("DHCP IS NOT RUNNING\n");
    else
        printf("DHCP IS RUNNING\n");
    
    }

    return;
}

void dhcp_start(char *argv[])
{
    //DHCP PACKET SETTING
    unsigned char buff_rcv[BUFF_SIZE];           struct bootp *bootp;
    int server_socket,send_socket, option;  int on = 1;
    struct sockaddr_in server_addr, client_addr, send_addr, renewal_addr;
    socklen_t client_size;  int str_len; 

    bootp = (struct bootp *)(buff_rcv);

    //server socket setting
    server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(server_socket == -1) {
        printf("socket 생성 실패");
        exit(1);
    }

    option = TRUE;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(67); //server port number
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //32bit IPV4
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    memset(&send_addr, 0, sizeof(send_addr));
    send_addr.sin_family = AF_INET;
    send_addr.sin_port = htons(68); //server port number
    send_addr.sin_addr.s_addr = inet_addr("255.255.255.255"); //32bit IPV4
    setsockopt(server_socket, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));


    if(bind(server_socket, (struct sockaddr* )&server_addr, sizeof(server_addr))== -1) {
        printf("bind() 실행 에러\n");
        exit(1);
    }  

    int i; //buffer init

    while(1) {    
        //연결이 끊긴 패킷이라도 테이블에 남아있을 수 있기 때문   
        checking_ip_time();
      //  if(dhcp_islive(argv) == FALSE)
         //   continue;

        str_len = recvfrom(server_socket, buff_rcv , BUFF_SIZE, 0, (struct sockaddr *)&client_addr, &client_size);  
        //SETTING NIC DEVICE
        struct ifreq interface;
        strncpy(interface.ifr_ifrn.ifrn_name, argv[1], sizeof(argv[1]));                    
        if(setsockopt(server_socket, SOL_SOCKET, SO_BINDTODEVICE, (char *)&interface, sizeof(interface)))
        {
            printf("BINDTODEVICE ERROR\n");
            exit(1);
        }

        // get my ip address
        char ipstr[40];
        strncpy(ifr.ifr_name, argv[1], IFNAMSIZ);   
        if (ioctl(server_socket, SIOCGIFADDR, &ifr) < 0) {
            printf("Error");
        } else {
            inet_ntop(AF_INET, ifr.ifr_addr.sa_data+2,
                    ipstr,sizeof(struct sockaddr));
        }  
        strcpy(db_my_ip, ipstr);

        if(strcmp(inet_ntoa(client_addr.sin_addr), "0.0.0.0") == 0 && ntohs(client_addr.sin_port) == 0);
        else {
            //DISCOVER PACKET
            if(buff_rcv[240] == DHCP_MESSAGE && buff_rcv[242] == DHCP_DISCOVER)
            {
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "SELECT * FROM ip_list WHERE NIC = '%s' AND Assigned = 0 ORDER BY INET_ATON(ip_addr) LIMIT 1", argv[1]);//할당 되지 않은 ip중 가장 첫번 째  
                mysql_query(connection,buf);
                result = mysql_store_result(connection);
                query = mysql_num_rows(result);
                if(query == 0)
                {
                    //할당 가능 ip가 없으면 NAK메시지 송신
                    bootp->bp_op = BOOT_REPLY;

                    //DHCP MESSAGE TYPE
                    bootp->dhcp_message.opt_type = DHCP_MESSAGE;
                    bootp->dhcp_message.opt_len = 1;
                    bootp->dhcp_message.opt_value= DHCP_NAK;

                    //DHCP SERVER IDENTIFIER 
                    bootp->dhcp_server.opt_type = 54;
                    bootp->dhcp_server.opt_len = 4;
                    bootp->dhcp_server.opt_value = inet_addr(db_my_ip);
                    bootp->dhcp_end.opt_type = DHCP_END;

                    sendto(server_socket, buff_rcv, str_len, 0, (struct sockaddr *)&send_addr, sizeof(send_addr));

                    continue;
                }
                row = mysql_fetch_row(result); 
                mysql_free_result(result);
                char *db_insert_ip = row[1];
                char *db_insert_subnet = row[4];
                char *db_insert_router = row[5];
                char *db_insert_domain = row[6];
                char *db_insert_lease = row[7];


                bootp->bp_op = BOOT_REPLY;
                bootp->bp_yiaddr = inet_addr(db_insert_ip);  

                //DHCP MESSAGE TYPE
                bootp->dhcp_message.opt_type = DHCP_MESSAGE;
                bootp->dhcp_message.opt_len = 1;
                bootp->dhcp_message.opt_value= DHCP_OFFER;

                //DHCP SERVER IDENTIFIER 
                bootp->dhcp_server.opt_type = 54;
                bootp->dhcp_server.opt_len = 4;
                bootp->dhcp_server.opt_value = inet_addr(db_my_ip);

                // DHCP LEASE TIME
                bootp->dhcp_lease.opt_type = 51;
                bootp->dhcp_lease.opt_len = 4;
                bootp->dhcp_lease.opt_value = htonl(atol(db_insert_lease));

                // DHCP SUBNET
                bootp->dhcp_subnet.opt_type = DHCP_SUBNET;
                bootp->dhcp_subnet.opt_len = 4;
                bootp->dhcp_subnet.opt_value = inet_addr(db_insert_subnet);

                // DHCP ROUTER
                bootp->dhcp_router.opt_type = DHCP_ROUTER;
                bootp->dhcp_router.opt_len = 4;
                bootp->dhcp_router.opt_value = inet_addr(db_insert_router);;

                // DHCP DOMAIN
                bootp->dhcp_domain.opt_type = DHCP_DOMAIN;
                bootp->dhcp_domain.opt_len = 4;
                bootp->dhcp_domain.opt_value = inet_addr(db_insert_domain);

                bootp->dhcp_end.opt_type = DHCP_END;

                for(i = 274; i < str_len; i++) //padding bit init
                    buff_rcv[i] = 0x00;
                sendto(server_socket, buff_rcv, str_len, 0, (struct sockaddr *)&send_addr, sizeof(send_addr));
            }

            //DHCP_REQUEST
            if(buff_rcv[240] == DHCP_MESSAGE && buff_rcv[242] == DHCP_REQUEST)
            {   
                memset(buf, 0, sizeof(buf));
                sprintf(client_mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x", buff_rcv[28], buff_rcv[29], buff_rcv[30], buff_rcv[31], buff_rcv[32], buff_rcv[33]);

                //RENEWAL
                if(strcmp(inet_ntoa(client_addr.sin_addr),"0.0.0.0") != 0) 
                {
                    sprintf(buf, "SELECT * FROM ip_list WHERE NIC = '%s' AND Assigned = 1  AND ip_addr = '%s'", argv[1], inet_ntoa(client_addr.sin_addr)); 
                    mysql_query(connection,buf);
                    result = mysql_store_result(connection);
                    query = mysql_num_rows(result);
                    row = mysql_fetch_row(result); 
                    mysql_free_result(result);
                    char *db_insert_ip = row[1];
                    char *db_insert_subnet = row[4];
                    char *db_insert_router = row[5];
                    char *db_insert_domain = row[6];
                    char *db_insert_lease = row[7];



                    bootp->bp_op = BOOT_REPLY;
                    bootp->bp_yiaddr = inet_addr(inet_ntoa(client_addr.sin_addr));    
                    bootp->bp_ciaddr = inet_addr(inet_ntoa(client_addr.sin_addr));

                    //DHCP MESSAGE TYPE
                    bootp->dhcp_message.opt_type = DHCP_MESSAGE;
                    bootp->dhcp_message.opt_len = 1;
                    bootp->dhcp_message.opt_value= DHCP_ACK;

                    //DHCP SERVER IDENTIFIER 
                    bootp->dhcp_server.opt_type = 54;
                    bootp->dhcp_server.opt_len = 4;
                    bootp->dhcp_server.opt_value = inet_addr(db_my_ip);

                    // DHCP LEASE TIME
                    bootp->dhcp_lease.opt_type = 51;
                    bootp->dhcp_lease.opt_len = 4;
                    bootp->dhcp_lease.opt_value = htonl(atol(db_insert_lease));

                    // DHCP SUBNET
                    bootp->dhcp_subnet.opt_type = DHCP_SUBNET;
                    bootp->dhcp_subnet.opt_len = 4;
                    bootp->dhcp_subnet.opt_value = inet_addr(db_insert_subnet);

                    // DHCP ROUTER
                    bootp->dhcp_router.opt_type = DHCP_ROUTER;
                    bootp->dhcp_router.opt_len = 4;
                    bootp->dhcp_router.opt_value = inet_addr(db_insert_router);;

                    // DHCP DOMAIN
                    bootp->dhcp_domain.opt_type = DHCP_DOMAIN;
                    bootp->dhcp_domain.opt_len = 4;
                    bootp->dhcp_domain.opt_value = inet_addr(db_insert_domain);

                    bootp->dhcp_end.opt_type = DHCP_END;


                    for(i = 274; i < str_len; i++)
                        buff_rcv[i] = 0x00;

                    memset(&renewal_addr, 0, sizeof(renewal_addr));
                    renewal_addr.sin_family = AF_INET;
                    renewal_addr.sin_port = htons(68); //server port number
                    renewal_addr.sin_addr.s_addr = inet_addr(inet_ntoa(client_addr.sin_addr)); //32bit IPV4


                    int value = sendto(server_socket, buff_rcv, str_len, 0, (struct sockaddr *)&renewal_addr, sizeof(renewal_addr));
                    if(value > 0) 
                    {
                        sprintf(buf," UPDATE ip_list SET end = DATE_ADD(end, INTERVAL CONVERT((SELECT default_lease_time FROM ip_list where NIC = '%s' LIMIT 1 )/2, UNSIGNED) SECOND) WHERE MAC_addr = '%s' AND  ip_addr = '%s'",argv[1], client_mac_addr, inet_ntoa(client_addr.sin_addr));
                        if(mysql_query(connection, buf))
                        {
                            fprintf(stderr, "UPDATE ERROR: %s\n", mysql_error(&conn));
                            exit(1);

                        }
                        continue; 
                    }
                }
                else if(strcmp(inet_ntoa(client_addr.sin_addr),"0.0.0.0") == 0)
                {
                    //REQUEST
                    sprintf(buf, "SELECT * FROM ip_list WHERE NIC = '%s' AND Assigned = 0 ORDER BY INET_ATON(ip_addr) LIMIT 1", argv[1]);
                    mysql_query(connection,buf);
                    result = mysql_store_result(connection);
                    query = mysql_num_rows(result);
                    row = mysql_fetch_row(result); 
                    mysql_free_result(result);

                    char *db_insert_ip = row[1];
                    char *db_insert_subnet = row[4];
                    char *db_insert_router = row[5];
                    char *db_insert_domain = row[6];
                    char *db_insert_lease = row[7];

                    bootp->bp_op = BOOT_REPLY;
                    bootp->bp_yiaddr = inet_addr(db_insert_ip);  

                    //DHCP MESSAGE TYPE
                    bootp->dhcp_message.opt_type = DHCP_MESSAGE;
                    bootp->dhcp_message.opt_len = 1;
                    bootp->dhcp_message.opt_value= DHCP_ACK;

                    //DHCP SERVER IDENTIFIER 
                    bootp->dhcp_server.opt_type = 54;
                    bootp->dhcp_server.opt_len = 4;
                    bootp->dhcp_server.opt_value = inet_addr(db_my_ip);

                    // DHCP LEASE TIME
                    bootp->dhcp_lease.opt_type = 51;
                    bootp->dhcp_lease.opt_len = 4;
                    bootp->dhcp_lease.opt_value = htonl(atol(db_insert_lease));

                    // DHCP SUBNET
                    bootp->dhcp_subnet.opt_type = DHCP_SUBNET;
                    bootp->dhcp_subnet.opt_len = 4;
                    bootp->dhcp_subnet.opt_value = inet_addr(db_insert_subnet);

                    // DHCP ROUTER
                    bootp->dhcp_router.opt_type = DHCP_ROUTER;
                    bootp->dhcp_router.opt_len = 4;
                    bootp->dhcp_router.opt_value = inet_addr(db_insert_router);;

                    // DHCP DOMAIN
                    bootp->dhcp_domain.opt_type = DHCP_DOMAIN;
                    bootp->dhcp_domain.opt_len = 4;
                    bootp->dhcp_domain.opt_value = inet_addr(db_insert_domain);

                    bootp->dhcp_end.opt_type = DHCP_END;


                    for(i = 274; i < str_len; i++)
                        buff_rcv[i] = 0x00;


                    int value = sendto(server_socket, buff_rcv, str_len, 0, (struct sockaddr *)&send_addr, sizeof(send_addr));
                    //데이터가 성공적으로 보내졌으면 DB 업데이트
                    if(value > 0) 
                    {
                        sprintf(buf," UPDATE ip_list SET MAC_addr = '%s', start = now(),end = DATE_ADD(now(), INTERVAL CONVERT((SELECT default_lease_time FROM ip_list where NIC = '%s' LIMIT 1 ), UNSIGNED) SECOND), Assigned = 1 WHERE NIC = '%s' AND  ip_addr = (SELECT ip_addr FROM ip_list WHERE Assigned = 0  AND NIC = '%s' LIMIT 1)",client_mac_addr,argv[1], argv[1], argv[1]);
                        if(mysql_query(connection, buf))
                        {
                            fprintf(stderr, "UPDATE ERROR: %s\n", mysql_error(&conn));
                            exit(1);
                        }
                    }
                }   
            }
            //RELEASE
            if(buff_rcv[240] == DHCP_MESSAGE && buff_rcv[242] == DHCP_RELEASE)
            {
                sprintf(client_mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x", buff_rcv[28], buff_rcv[29], buff_rcv[30], buff_rcv[31], buff_rcv[32], buff_rcv[33]);//삭제 하기 위해 MAC주소 얻기

                sprintf(buf, "SELECT * FROM ip_list WHERE MAC_addr = '%s' AND ip_addr = '%s'", client_mac_addr, inet_ntoa(client_addr.sin_addr));
                if(mysql_query(connection, buf))
                {
                    fprintf(stderr, "INSERT backup ERROR: '%s;\n", mysql_error(&conn));
                    exit(1);
                } 
        //        memset(buf, 0,sizeof(buf));
                result = mysql_store_result(connection);
                row = mysql_fetch_row(result); 
                mysql_free_result(result);

                sprintf(buf, "INSERT INTO backup(NIC, ip_addr, MAC_addr, network, subnet, gateway, name_server, default_lease_time, start, end) values('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s',now())", row[0],row[1], row[2], row[3], row[4], row[5], row[6], row[7],row[8]);
                mysql_query(connection, buf);
                memset(buf, 0,sizeof(buf));

                sprintf(buf, "UPDATE ip_list SET MAC_addr = NULL, start = NULL, end = NULL, Assigned = 0 WHERE  MAC_addr = '%s' AND ip_addr = '%s'", client_mac_addr, inet_ntoa(client_addr.sin_addr));
                if(mysql_query(connection,buf)) 
                {
                    fprintf(stderr,"RELEASE ERROR: '%s'", mysql_error(&conn));
                    exit(1);
                } 
            }
        }
    }
}

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

int daemon_start(char *argv[])
{
    pid_t pid; int fd;  int pid_value;
    
    if((pid = fork()) < 0)
        return -1;
    else if(pid != 0) 
        exit(0);
    //이미 실행중인지 여부 판단
   // if(dhcp_islive(argv) == FALSE)
      //  exit(0);
    sprintf(buf, "SELECT NIC FROM pid_table WHERE NIC = '%s' LIMIT 1", argv[1]);
    mysql_query(connection, buf);
    result = mysql_store_result(connection);
    query = mysql_num_rows(result);
    mysql_free_result(result);
    if( query != 0) 
    {
        printf("이미 %s에 대한 DHCP SERVICE가 동작중입니다\n", argv[1]);
        exit(0);
    }
    
    printf( "DHCP 서비스를 시작합니다\n");
    pid_value = getpid(); //DHCP PROCESS
    setsid();
    chdir("/");
    fd = open("dev/null", O_RDWR);
    dup2(fd,2);dup2(fd,0);
    close(fd);
    umask(0);
    return pid_value;
}


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


