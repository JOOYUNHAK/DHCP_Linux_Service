#include "dhcp_h.h"

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
        if(dhcp_islive(argv) == ERROR)
        {
            fprintf(stdout, "에러 원인: %s에 대한 DHCP SERVICE가 예상치 못하게 종료된 상태입니다. stop 옵션을 이용하여 종료 후 재시작 하여 주시기 바랍니다\n", argv[1]);
            continue;
        }

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


