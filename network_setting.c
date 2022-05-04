#include "dhcp_h.h"

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


