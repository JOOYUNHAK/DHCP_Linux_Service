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
#define ERROR 3
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

int CountIp(char *, char *); //몇 개의 ip가 할당되어야 하는지 구하는 함수
void PrintManual(); //사용자 메뉴얼을 출력하는 함수
void checking_ip_time(); //ip list의 종료 시간을 주기적으로 체크해주는 함수
int dhcp_islive(char *argv[]); //DHCP가 현재 살아있는지 체크하는 함수
void search_ip_list(char *argv[]); // 현재 할당되어 있는 ip의 리스트를 출력하는 함수
void search_used_ip(); //할당 해제된 ip의 리스트를 출력하는 함수
void delete_network_setting(char *argv[]); //설정했던 네트워크 정보를 삭제하는 함수
void search_network_setting(char *argv[]); //설정했던 네트워크 정보를 조회하는 함수
void network_setting(char *argv[]); //네트워크 정보를 세팅하는 함수
void dhcp_status(char *argv[]); //현재 DHCP SERVICE 상태를 알려주는 함수
void dhcp_start(char *argv[]); // DHCP SERVICE를 시작하는 함수
int is_setting(char *argv[]); //네트워크 정보가 세팅되어 있는지 체크하는 함수
int daemon_start(char *argv[]); //백그라운드 형태로 프로세스를 돌리는 함수
void insert_pid(int , char *argv[]); //프로세스의 pid를 저장하는 함수
void dhcp_stop(char *argv[]); //DHCP SERVICE를 중단하는 함수


MYSQL *connection; MYSQL conn;
MYSQL_RES *result;  MYSQL_ROW row;  MYSQL_RES *result2; MYSQL_ROW row2;
int query;  char start_ip_addr[20]; char end_ip_addr[20];   char network[20]; char subnet[20]; char gateway[20];
char name_server[20]; char default_lease_time[10]; char max_lease_time[10]; int num; char NIC[20];
unsigned char client_mac_addr[40]; char db_my_ip[20]; char buf[BUFF_SIZE];   
struct ifreq ifr;

    
