CC = gcc
TARGET = dhcp
SQL_COMPILE = -lmysqlclient

$(TARGET) : CountIP.o PrintManual.o checking_ip_time.o daemon_start.o delete_network_setting.o dhcp_islive.o dhcp_start.o dhcp_status.o dhcp_stop.o insert_pid.o is_setting.o network_setting.o search_ip_list.o search_network_setting.o search_used_ip.o main.o
	$(CC) -o $(TARGET) CountIP.o PrintManual.o checking_ip_time.o daemon_start.o delete_network_setting.o dhcp_islive.o dhcp_start.o dhcp_status.o dhcp_stop.o insert_pid.o is_setting.o network_setting.o search_ip_list.o search_network_setting.o search_used_ip.o main.o $(SQL_COMPILE)

main.o : main.c
	$(CC) -c -o main.o main.c $(SQL_COMPILE)

CountIP.o : CountIP.c
	$(CC) -c -o CountIP.o CountIP.c $(SQL_COMPILE)

PrintManual.o : PrintManual.c
	$(CC) -c -o PrintManual.o PrintManual.c

checking_ip_time.o : checking_ip_time.c
	$(CC) -c -o checking_ip_time.o checking_ip_time.c $(SQL_COMPILE)

daemon_start.o : daemon_start.c
	$(CC) -c -o daemon_start.o daemon_start.c $(SQL_COMPILE)

delete_network_setting.o : delete_network_setting.c
	$(CC) -c -o delete_network_setting.o delete_network_setting.c $(SQL_COMPILE)

dhcp_islive.o : dhcp_islive.c
	$(CC) -c -o dhcp_islive.o dhcp_islive.c $(SQL_COMPILE)

dhcp_start.o : dhcp_start.c
	$(CC) -c -o dhcp_start.o dhcp_start.c $(SQL_COMPILE)

dhcp_status.o : dhcp_status.c
	$(CC) -c -o dhcp_status.o dhcp_status.c

dhcp_stop.o : dhcp_stop.c
	$(CC) -c -o dhcp_stop.o dhcp_stop.c $(SQL_COMPILE)

insert_pid.o : insert_pid.c
	$(CC) -c -o insert_pid.o insert_pid.c $(SQL_COMPILE)

is_setting.o : is_setting.c
	$(CC) -c -o is_setting.o is_setting.c $(SQL_COMPILE)

network_setting.o : network_setting.c
	$(CC) -c -o network_setting.o network_setting.c  $(SQL_COMPILE)

search_ip_list.o : search_ip_list.c
	$(CC) -c -o search_ip_list.o search_ip_list.c $(SQL_COMPILE)

search_network_setting.o : search_network_setting.c
	$(CC) -c -o search_network_setting.o search_network_setting.c $(SQL_COMPILE)

search_used_ip.o : search_used_ip.c
	$(CC) -c -o search_used_ip.o search_used_ip.c $(SQL_COMPILE)

clean :
	rm  *.o
