# DHCP_Linux_Service

# 1. 시스템 개요

## 1) 시스템 개요

```
 인터넷 사용자와 사용되어지는 컴퓨터 수가 늘어나면서 매번 IP를 할당해주어야 하는 불편함을 해소
하고 자동적으로 IP를 할당해주고 데이터베이스로 관리하면서 효율적인 사용과 관리를 할 수 있는 서비스를 개발하고자 한다.
```

```
 매번 수동으로 IP를 할당받는 번거로움을 덜어줌과 동시에 자동으로 IP를 할당받을 수 있고 DB에 저장된 데이터들을 간단한 명령어만으로도 IP 관리를 할 수 있도록 한다.
```

## 3) 시스템 범위

```
 IP를 할당받아 사용하려는 모든 사용자들을 대상으로 한다. 현재 윈도우나 리눅스를 사용할 때 자체적으로 DHCP서버와 ISC DHCP서비스가 있어 할당을 받아 사용하기 수월했으므로 유사하게 작동하도록 개발하는 것이 주된 내용이다.
```

## 4) 시스템 고려사항

```
**<시스템>**
- 이 프로그램을 실행하면 백그라운드와 유사하게 동작한다.
- 종료를 위해 터미널의 신호를 막지 않는다.
- 백그라운드로 돌고 있는 프로세스를 죽이기 위해 PID를 DB에 네트워크 인터페이스 카드명과 함께 입력하여 해당 프로세스를 죽일 수 있다.
- 표준 입출력을 전부 막는다.
- 프로그램 내에 소켓을 만들어 패킷을 전송한다.
- 전송방식은 UDP 프로토콜을 이용하며 전송한다.
- 수신,송신측은 67번과 68번 포트가 열려있어야 한다.
- 모든 설정과 동작은 COMMAND LINE에서 이루어진다.
- 사용자가 원하면 언제든 서비스를 시작하고 중단할 수 있어야한다.
- 시작하려는 NIC에 대해 이미 실행중이면 실행이 되지 않는다.
- 시작한 DHCP SERVICE가 예상치 못하게 중지된 경우 안내문을 계속해서 출력한다.
- 사용자는 언제든 DHCP의 현재 동작 상태를 체크할 수 있어야 한다.

**<IP 할당 및 관리>**
- 이 서비스를 이용하여 IP를 할당받을 때 이미 할당되어 있는 IP를 할당하여 충돌하는 일이 없도록 한다.
- 임대 기간이 끝날때까지 시간 연장이 되지 않으면 자동으로 IP가 반납되어야 한다.
- 네트워크 대역별로 설정하여도 정상적으로 설정이 되어야 한다.
- IP 할당 이후 임대 시간의 절반이 지난 시점에 자동으로 임대 시간 연장 요청이 오면 ACK패킷을 보내준다.
- 만약 RENEW과정에서 IP 임대시간의 약 90퍼센트가 지나도 갱신이 되지 않으면 IP연결이 해제된다.
- 반납이 된 IP는 다시 재활용 할 수 있어야 한다.
- IP가 할당이 되면 DB에 할당된 IP의 네트워크 정보가 업데이트된다.
- IP 할당 전 중복 방지를 위해 DB에서 할당되어 있는 IP를 미리 검사한다.
- 지정한 범위내의 IP를 모두 사용중이라면 DISCOVER PACKET이 들어왔을 때 NAK PACKET을 송신한다.
- 할당된 IP들은 DB에서 ASSIGNED값을 1로 변경하여 식별한다.
- 네트워크 인터페이스 카드가 여러개인 경우 인터페이스 카드별로 IP를 할당시켜줄 수 있어야 한다.
- 서비스가 중지되어있는 상태더라도 IP할당 해제된 IP들은 LEASE FILE DB와는 별개로 DB에 관리되어 확인할 수 있다.
- 사용되었던 ip들의 정보를 저장하는 DB는 20개의 데이터가 쌓이면 자동적으로 마지막 1개를 삭제하면서 최대 20개로 유지된다.

**<DB>**
- 기존 Confile파일과 lease파일로 관리하던 서비스와는 달리 DB로 통합하여 관리한다.
- 할당 범위를 변경하거나 네트워크 정보를 간편하게 관리하기 위해 default값으로 변경하며 관리한다.
- 사용자가 하나의 네트워크 인터페이스 카드에 대해 정보를 입력하면 자동적으로 고정되어있는 값들은 DB에 추가된다.
- 연결이 끊긴다거나 반납을 하는 경우 DB에서 해당 주소가 삭제되는 것이 아니기 때문에 항상 UPDATE를 하면서 관리한다.
- 할당 시간과 종료 시간은 TIMESTMAP가 아닌 DATETIME형식으로 저장되어진다.
- RELEASE PACKET을 수신하거나 정상적으로 RENEW과정이 이루어지지 않아 해제된 IP들은 backup DB에 관리되며 종료시간으로부터 20분이 지난 뒤에는 자동적으로 삭제된다.
```

## 5) 시스템 기능

![2.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/2.jpg)

 

- IP 자동 할당
    - 기존 DHCP의 4단계 연결 과정을 거친 후 자동적으로 IP가 할당이 된다.
    - ACK  PACKET을 보낼 때 DB에 저장되어 있던 기본 네트워크 관련 정보들을 같이 전달해준다.
    - IP 주소를 할당 시키기 전 해당 IP가 이미 할당이 되어있는지 여부를 판단하여 할당 시킨다.
    - 네트워크 정보는 사용자가 설정한 대로 반영된다.
    - 여러 대의 PC에서 IP 할당 요청이 있어도 할당이 정상적으로 이루어져야 한다.
    - IP 할당 범위 내에 있는 주소들을 전부 사용했을 때는 할당이 이루어지지 않으며 NAK PACKET을 송신한다.
    - 할당된 Client의 네트워크 정보는 DB에 저장되며 관리된다.
    
- 주소 임대 기간 연장
    - IP를 할당 받은 클라이언트가 사용 도중 시간 연장을 원할 시 정상적으로 시간 연장이 된다.
    - 클라이언트가 직접 임대 요청을 하지 않아도 임대 시간의 50%, 87.5%가 지났을 시점에 연장 요청이 들어오면 ACK PACKET을 송신하여 정상적으로 연장한다.
    - 시간 연장이 완료되면 DB도 함께 업데이트 된다.
- IP 할당 해제
    - 임대 시간이 만료되었거나 클라이언트가 IP를 반납할 경우 할당되었었던 IP 주소는 자동적으로 할당 해제된다.
    - 할당 해제 된 IP주소의 관련된 네트워크 정보들은 전부 사용자가 지정한 초기 값으로 변경 시켜준다.
    - 할당 해제된 IP주소는 바로 요청이 들어왔을 경우 다시 할당이 이루어질 수 있어야 한다.
    - RELEASE PACKET이나 임대 시간 만료로 인한 해제의 경우 backup db에 입력되어 종료 시점으로부터 20분간 할당되었던 정보 확인이 가능하다.

## 6) 제약사항

- 네트워크 환경
    - Client는 IP 할당 받는 방식을 수동 방식이 아닌 DHCP자동 할당 방식으로 설정되어 있어야 한다.
    - 서버 측 환경에서 DHCP 지원 설정이 해제되어 있어야 한다.
- 설정 파일
    - DB에 네트워크 정보를 default값들로 설정하고 이 default값들을 변경할 수 있어야 하며 설정이 완벽히 되지 않은 경우 IP 할당이 정상적으로 이루어지지 않을 수 있다.
    - IP 주소의 범위를 잘못 설정할 경우 네트워크 설정이 되지 않는다.
    - Class별로 해당되는 IP 주소를 입력하지 않은 경우 정상적으로 할당 되지 않는다.
- 방화벽
    - 67번 68번 포트의 방화벽이 해제되어 있어야 한다.

# **2. USER Story**

1. 사용자는 자신의 컴퓨터의 IP 할당 방식을 DHCP로 설정해 놓아야 한다.
2. 사용자는 언제든 서비스를 시작하고 중단할 수 있다.
3. 사용자는 다른 컴퓨터와 겹치는 IP 주소를 할당 받는 일이 없어야 한다.
4. 사용자는 IP를 할당 받고 자신이 할당 받은 IP 주소를 비롯하여 설정했던 네트워크 정보까지 확인 할 수 있어야 한다.
5. 사용자는 언제든 설정했던 네트워크 정보를 다시 설정할 수 있다.
6. 사용자는 네트워크 인터페이스 카드 별로 네트워크 설정을 할 수 있어야 한다.
7. 사용자는 네트워크 인터페이스 카드 별로 IP 할당을 받을 수 있어야 한다.
8. 사용자는 IP를 할당 받으면 자신의 MAC주소를 비롯하여 임대 시간과 네트워크 정보를 조회할 수 있어야 한다.
9. 관리자는 IP가 할당되면 DB안에 해당 IP의 네트워크 정보를 업데이트한다.

 <주소 임대 기간 연장>

10. 사용자는 한번 IP 할당을 받고 원할 때에 임대 연장 요청을 할 수 있다. 

11. 사용자가 따로 요청하지 않아도 자체적으로 임대 시간의 50%, 87.5%가 경과할 경우 연장 요청이 와 시간이 연장되어야 한다.

12. 사용자는 스스로가 연결을 끊지 않는 이상 동일한 IP에 대하여 연장이 계속해서 되어야 한다.

13. 관리자는 연결이 갱신 될 때마다 DB의 갱신 시간들을 업데이트 해야 한다.

14. 사용자는 자신이 원할 때에 IP주소를 반납할 수 있어야 한다.

15. 사용자가 반납을 하면 해당 PC에 IP가 할당이 되지 않은 상태여야 한다.

16. 할당 해제된 IP는 다시 바로 할당 될 수 있어야 한다.

1. 사용자는 실시간으로 할당된 IP들의 정보를 조회 할 수 있다.

18. 연결이 끊긴 IP들은 DB에 따로 저장해두고 관리하며 사용자는 이를 조회할 수 있다.

1. 연결이 끊긴 IP들은 종료 시점으로부터 20분 간 유지되며 그 이후에는 자동 삭제 된다.

# 3. Activity Diagram

![activity.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/activity.jpg)

# 4. Sequence Diagram

![sequence.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/sequence.jpg)

# 5. DB Table

- ip_list Table

![DBTABLE.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/DBTABLE.jpg)

- backup Table

![backup.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/backup.jpg)

- 데이터 구조
    - NIC: 네트워크 인터페이스 카드명
    - ip_addr: 사용자가 설정한 범위 내의 IP 주소들로, 네트워크 대역에 따라 범위의 개수를 구해 시작 주소부터 끝 주소까지 입력이 되도록 한다.
    - MAC_addr: IP할당을 받은  Client의 MAC주소
    - network: 사용자가 설정한 네트워크 대역
    - subnet: 사용자가 설정한 subnet mask
    - gateway: 사용자가 설정한 gateway
    - name_server: 사용자가 설정한 DNS
    - default_lease_time: 사용자가 설정한 기본 임대 시간
    - start, end: 각각 할당 받은 시간과 종료 시간
    - Assigned: 해당 IP주소가 누가 사용 중인지 여부를 판단할 수 있는 필드로 0이면 할당 안된 상태, 1이면 할당 된 상태로 1인 주소들은 할당 시키지 않는다.
- pid_table Table
    
    ![pid.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/pid.jpg)
    
- 데이터 구조
    - NIC: 네트워크 인터페이스 카드명
    - PID: 인터페이스 카드가 여러 개일 경우 각각의 인터페이스 카드에 대한 DHCP 서비스를 시작하고 중단하기 위해 해당 프로세스 PID를 저장 시킨다.

# 6.구현 전 기존 서비스 테스트(ISC-DHCP)

- 테스트 환경 (Virtual Box)

|  | DHCP SERVER | DHCP Client |
| --- | --- | --- |
| OS | Centos 7 | Windows 10 |
| Kernel | 3.10 | * |
| Network | 196.120.56.1 (196.120.56.0/24) | 196.120.56.100 ~ 196.120.56.150 사이 하나 |
- Virtual Box 환경 설정

```
Virtual Box에 NAT 네트워크를 하나 추가한다.
파일 -> 환경설정 -> 네트워크 -> NAT 네트워크 추가 -> 원하는 CIDR 값과 이름 설정 -> DHCP 지원 해제
```

![NatNetwork.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/NatNetwork.jpg)

```
각 머신에서 어댑터1의 네트워크를 NAT 네트워크로 변경해준다.
각 머신 -> 설정 -> 네트워크 -> 어댑터 1 -> NAT 네트워크
```

![머신설정.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/%EB%A8%B8%EC%8B%A0%EC%84%A4%EC%A0%95.jpg)

```
Server 가상 머신의 네트워크 정보를 수동으로 설정하여도 된다.
CentOS(서버쪽 가상 머신) -> 프로그램 -> 시스템 도구 -> 설정 -> 네트워크 -> 유선(켬으로 변경) 이후 톱니바퀴 -> IPv4
```

![server.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/server.jpg)

- DHCP 서버 설정
    - DHCP서버를 구축하기 위해 패키지를 하나 설치해준다.

```
sudo yum -y install dhcp*
```

![1.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/1.jpg)

- DHCP 서버 IP 확인

```
ifconfig 명령어를 통해 해당 IP 확인
```

![3.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/3.jpg)

```
IP가 잘못 설정되어 있는 경우 - sudo ifconfig enp0s3(인터페이스 이름) 195.180.2.4(설정하고자 하는 IP)
```

- 포트번호 확인

```
cat /etc/services | grep bootp (Client: UDP/68  Server: UDP/67)
```

![portnumber.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/portnumber.jpg)

- 인터페이스 지정

```
    Centos7버전 이후부터 /etc/sysconfig/dhcpd파일 내에서 특정 인터페이스를 지정하면 안된다고 한다.
먼저 /usr/lib/systemd/system/dhcpd.service 파일을 /etc/systemd/system/로 복사를 한 다음, 파일 내에추가하고 싶은 인터페이스명을 다음과 같이 적어준다.
```

![interface.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/interface.jpg)

```
다음 아래 명령어로 데몬을 리로딩하여 재시작한다.
sudo systemctl daemon-reload
sudo systemctl restart dhcpd.service
```

- 포트 열어주기

```
 Client와 Server가 각각 UDP 68번과 67을 사용하므로 방화벽 port를 열어준다.
```

![UDP.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/UDP.jpg)

- Config 파일 설정

```
sudo vi /etc/dhcp/dhcpd.conf
```

![Config.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/Config.jpg)

```
설정을 완료하면 sudo service network restart 명령을 통해 변경사항 반영
```

- DHCP 데몬 시작 및 활성화

```
시작: sudo systemctl start dhcpd (ps -ef | grep dhcpd 명령어를 통해 확인 가능)
```

![ps.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/ps.jpg)

```
활성화: sudo systemctl enable dhcpd
부팅시에도 자동으로 실행되게 하기 위해선 이 명령어 실행(sudo systemctl is-enabled dhcpd를 통해 여부 확인 가능)
```

- 결과 화면 및 패킷 캡쳐

```
위에 과정까지 다 하고 현재 활동 상태를 알고 싶으면 sudo service dhcpd status를 해본다. 시작을 했으므로 현재 상태는 running상태이다.
```

![status.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/status.jpg)

```
클라이언트인 windows 10의 가상머신에 들어가 IP를 확인해보니 정상적으로 할당받은 것을 볼 수 있고 기본 임대 시간인 10분이 잘 주어졌다.
```

![window.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/window.jpg)

- DHCP Discover Packet

![discover.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/discover.jpg)

- Message type: Client가 DHCP 서버를 찾기 위해 요청이 들어왔으므로 값이 1이다.
- Hardware type: 하드웨어 타입(이더넷)
- Transaction ID: 하나의 request와 reply는 동일한 Transaction ID값을 가진다.
- Bootp flags: broadcast flag가 1이므로 서버가 수신하여 응답할 때도 broadcast로 응답한다. 값이 0이면 unicast를 수행한다.
- Client IP address: 아직 client는 IP할당을 받지 못하였으므로 값이 없다.
- Client MAC address: 클라이언트의 물리 주소를 념겨 준다.
- Parameter Request List: Client가 IP 주소 요청을 할 때 서버한테서 받을 응답에 추가로 받고싶은 정보들을 전달해준다.

- DHCP Offer Packet

![offer.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/offer.jpg)

- Your (client) IP address: 해당 IP주소의 사용이 가능하다는 것을 알려준다.
- Message type: 이번엔 Server가 응답하는 패킷 이기 때문에 값이 2이다. (요청 :1/응답: 2)
- DHCP Server Identifier: Server의 IP주소를 전달해준다.
- IP address lease time: IP 주소 임대 시간을 알려준다.
- Router: 게이트웨이의 주소도 알려준다.
- Domain Name Server: DNS의 정보고 넘겨준다.

- DHCP Request Packet

![Request.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/Request.jpg)

- Requested IP address: Server에서 제안했던 IP주소를 사용하겠다고 요청
- DHCP Server Identifier: Server의 IP주소를 나타낸다.
- Parameter Request List : Server에게 추가로 요청하고 싶은 정보를 패킷에 보낸다.

- DHCP ACK Packet

![ack.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/ack.jpg)

- Bootp flags: 0은 unicast 방식으로 client와 1대1로 주고받는다.
- DHCP Message type: 값이 5인 것으로 보아 ACK 패킷이다.

## 7. 기능별 pseudo-code

Network Setting

```c
if 네트워크 저장할 DB Table 생성이 안되어있으면
		네트워크 저장 DB 생성
사용자로부터 네트워크 정보들을 입력받는다
if ip 주소의 범위가 잘못 설정되었으면
		다시 입력 받는다
for ip 범위만큼 반복하면서
		ip와 네트워크 정보들을 DB에 입력한다.
```

Delete Network Setting

```c
if 인자로 NIC가 주어졌다면	
		인자로 주어진  NIC에 대해 네트워크 정보를 초기화 할지 여부를 묻는다
		사용자로부터 입력을 받는다
				if 사용자가 삭제한다면
						해당 NIC에 저장된 DB의 값들을 전부 default값으로 변경한다.
				else
						DB의 값들을 유지한다.

		
```

Search Network Setting

```c
인자로 주어진 NIC를 변수에 저장한다
DB에서 해당 NIC에 맞는 데이터를 조회한다
if 결과가 하나도 없으면
		조회 실패를 나타낸다
else
		DB에서 추출한 정보들을 배열로 받는다
		추출한 정보를 출력한다
```

Search ip list

```c
checking_ip_time(); //DB에 저장된 ip들의 시간 정보를 주기적으로 검사하는 함수
인자로 주어진 NIC를 변수에 저장한다
해당 NIC와 일치하고 현재 할당되어 있는 ip들의 정보들을 DB에서 가져온다
while 배열로 받을 정보가 더이상 존재하지 않을때까지
		배열로 받아온 정보들을 출력한다

```

Checking ip time

```c
if 할당해제된 ip들의 정보를 저장할 backup Table이 생성 안되어있으면
		backup DB 생성
할당된 ip들의 DB에서 현재 시간보다 해당 ip의 ip할당 종료 시간이 더 빠른 데이터 추출
while 배열로 받을 정보가 더 이상 존재하지 않을 때까지
		배열로 받아온 정보들을 DB의 구조에 맞게 backup Table에 저장한다
할당된 ip들의 DB에서 종료 시간이 이미 경과했는데 남아있는 데이터들을 초기값으로 업데이트한다
backup Table에서 ip 할당 종료 시간으로부터 20분이 넘은 데이터들은 지운다
```

Search used ip

```c
checking_ip_time();
backup DB에서 ip 임대 종료 시간을 기준으로 내림차순으로 정렬해서 데이터를 추출한다
while 배열로 받을 정보가 더 이상 존재하지 않을 때까지
		if 임대 종료 시간이 존재하면
				현재 시간을 초로 변경한 값에서 임대 종료 시간을 초로 변경한 시간을 빼 변수에 저장
				변수를 60으로 나눈 몫을 분에 해당하는 변수에 저장
				변수를 60으로 나눈 나머지를 초에 해당하는 변수에 저장
	  해제된 ip의 정보와 함께 지금으로부터 몇 분 전에 해제되었는지 함께 출력
```

dhcp start

```c
UDP 소켓을 하나 생성하여 연다
연 소켓에 포트 번호와 주소, 브로드캐스트, 재사용이 가능한 옵션을 설정한다
데이터를 보낼 곳의 통신방식과 주소와 포트번호를 저장
while 사용자가 종료할 때까지
		생성한 UDP 소켓으로부터 데이서 수신
		DHCP 서비스를 시작한 NIC명을 ifreq 구조체의 이름에 저장
		서버 컴퓨터의 해당 NIC의 ip주소를 구조체에 저장
		if 수신 PACKET이 DHCP DISCOVER PACKET이면
				네트워크 정보 DB에서 해당 NIC에 할당되지 않은 ip주소 하나 추출
				if 할당 가능한 ip가 없으면
						NAK PACKET을 송신
				DB에서 추출해 온 정보를 배열에 저장
				데이터를 조합하여 DHCP OFFER PAKCET 조작
				DHCP OFFER PACKET 송신
		if 수신 PACKET이 DHCP Request PACKET이면		
				if 수신된 패킷의 source ip가 0.0.0.0인 경우
						네트워크 정보에서 할당된 ip 중 해당 NIC와 일치하고 ip주소가 일치하는 데이터 추출
						DB에서 추출해 온 정보를 배열에 저장
						수신 데이터에서 Client의 MAC주소 추출
						데이터를 조합하여 DHCP ACK PACKET 조작
						DHCP ACK 송신
						DB에서 해당 ip를 할당 상태로 변경해주고 MAC주소, ip 시작 시간과 종료 시간을 업데이트
				if 수신된 패킷의 source ip가 0.0.0.0이 아닌경우
						네트워크 정보에서 할당된 ip 중 해당 NIC와 일치하고 ip주소가 일치하는 데이터 추출
						DB에서 추출해 온 정보를 배열에 저장
						수신 데이터에서 Client의 MAC주소 추출
						데이터를 조합하여 DHCP ACK PACKET 조작
						데이터를 보낼 곳의 통신방식, 주소, 포트번호 재설정
						DHCP ACK 송신
						DB에서 해당 ip의 ip 종료 시간을 lease time만큼 늘려준다 
		if 수신 PACKET이 DHCP RELEASE PACKET이면
				Client의 MAC주소 변수에 저장
				DB에서 수신된 패킷의 ip와 MAC주소와 일치하는 데이터 추출
				해당 데이터 backup Table에 정보를 그대로 저장
				해당 ip는 기본값으로 초기화
				
		
```

 Insert PID

```c
if NIC에 따른 DHCP 서비스를 시작한 PID를 저장할 DB가 존재하지 않으면
		PID Table 생성
PID Table에 NIC명과 PID 저장
```

DHCP Stop

```c
PID Table에서 해당 NIC와 일치하는 PID 정보 추출
if 추출된 데이터가 없으면
		해당 NIC에 대해 DHCP 서비스가 동작중이 아니라는 문구 출력
else
		kill(PID, SIGTERM);
PID Table에서 죽인 NIC의 정보 삭제
```

# 8. 동작 구현

## 1) Config파일 Setting(네트워크 정보 입력) -c 옵션

![setting.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/setting.jpg)

```
-c옵션으로 설정한 네트워크 정보들은 NIC에 맞는 DB에 저장되게 된다. 
IP 범위가 잘못 설정 되면 DB에 입력이 되지 않으며, 입력 도중 정상적으로 마치지
못하여도 입력이 되지 않아 재입력이 가능하다.
```

## 2) Setting한 Config파일 조회 -n 옵션

![network_search.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/network_search.jpg)

```
인자로 준 NIC에 맞는 네트워크 정보를 조회할 수 있다.
만약 해당 NIC에 아직 설정이 되어 있지 않으면 설정하라는 문구가 나오며
NIC별로 조회가 가능하다.
```

## 3) Setting한 Config파일 삭제 -d 옵션

![delete.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/delete.jpg)

```
설정했던 네트워크 정보에 대해 재설정을 위하여 삭제를 한다.
```

## 4) DHCP 서비스 시작

### (1) 한 대의 컴퓨터 ip 할당 (할당된 ip 조회 -s 옵션)

![START.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/START.jpg)

![DHCP_PS.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/DHCP_PS.jpg)

![ipconfig.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/ipconfig.jpg)

```
DHCP 서비스를 시작하기 위해 주는 옵션이며, 서비스를 시작 할 NIC를 인자로 준다.
설정을 하지 않았을 경우 시작이 되지 않으며, 시작을 한 뒤로는 백그라운드 형태로
동작한다.
또한, 현재까지 해당 NIC에 할당된 IP들을 조회하려면 -s옵션으로 조회할 수 있다.
```

### (2) 두 대의 컴퓨터 ip 할당

![ipconfig2.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/ipconfig2.jpg)

![new.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/new.jpg)

### (3) 여러 개의 인터페이스 카드

![new3.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/new3.jpg)

![new2.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/new2.jpg)

## 5) DHCP 서비스 중단

![stop.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/stop.jpg)

```
DHCP 서비스를 중단하기 할 수 있는 옵션이며, 백그라운드 형태로 동작하던
프로세스가 종료되며 REQUEST 패킷에 응답하지 못한다.
자동적으로 NAK 패킷을 보냄
```

## 6) 현재 DHCP 상태 체크 —status 옵션

### (1) DHCP가 현재 ERROR가 났을 때

![err.jpg](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/err.jpg)

```c
DHCP SERVICE를 시작하면 백그라운드 프로세스 형태로 동작을 하는데 갑자기 프로세스가 죽어버리는 경우가
발생하기에 체크 후 재시작 할 수 있도록 안내한다.
```

### (2) DHCP가 현재 정상적으로 동작 중일 때

![정상.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/%EC%A0%95%EC%83%81.jpg)

```c
DHCP SERVICE가 정상적으로 시작이 되고 프로세스가 죽지 않았다면 상태 체크를 했을 때 RUNNING상태가
된다.
```

### (3) DHCP가 현재 동작 중이 아닐 때

![dead.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/dead.jpg)

```c
DHCP SERVICE가 정상적으로 동작 중이였을 때 해당 SERVICE를 중단을 하고 상태 체크를 하면 
DEAD 상태가 된다.
```

## 7) 연결이 끊긴 ip 조회 -u 옵션

![pass.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/pass.jpg)

```
DHCP 서비스 동작 중 정상적으로 RENEWAL과정이 이루어지지 않았을 때, IP를 할당하고 
RELEASE패킷을 받기 전 SERVER가 끊겼을 때의 상황에서 비정상적으로 종료된 IP들을 
잠시 저장해두고 조회할 수 있다. 
몇분 전에 해제되었는지 알 수 있으며, 이 DB는 해제된 시점으로부터 20분간 조회가
가능하다.
```

## 8) 사용자 메뉴얼 —help 옵션

![user.JPG](DHCP_SERVICE%20fe5350dcba28453d9502299c2c4efae8/user.jpg)

```
사용법이 익숙하지 않은 사용자들에게 알려주는 메뉴얼이다.
```
