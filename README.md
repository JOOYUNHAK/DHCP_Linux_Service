# DHCP Service Project
> 개발인원: 1명
> 개발기간: 2022.04 ~ 2022.05

## Introduce
매번 수동으로 IP를 할당받아야 하는 번거로움을 없애고 자동으로 IP 관리를 해주는 DHCP의 동작과정을 이해하고 DHCP Server를 만들어보고자 진행한 프로젝트입니다.
실제 서비스 되지는 않으며, DHCP에 대해 관심이 있으신 분들은 공부용으로 사용해 보실 수 있으실 겁니다.

## Environment & Stack
<p align='left'>
<img src="https://img.shields.io/badge/Virtual Box-183A61?style=flat-square&logo=VirtualBox&logoColor=white"/></a>&nbsp 
<img src="https://img.shields.io/badge/Wireshark-1679A7?style=flat-square&logo=Wireshark&logoColor=white"/></a>&nbsp
<img src="https://img.shields.io/badge/C-A8B9CC?style=flat-square&logo=C&logoColor=white"/></a>&nbsp 
<img src="https://img.shields.io/badge/MariaDB-003545?style=flat-square&logo=MariaDB&logoColor=white"/></a>&nbsp
</p>

| DHCP Client | DHCP Server |
| ------ | ------ |
| Windows 10 | Centos 7 |

## Features
클라이언트의 요청으로부터 응답까지의 전체적인 과정을 [여기](https://github.com/JOOYUNHAK/DHCP_Linux_Service/blob/master/ActivityDigram.md)에서 간단히 확인하실 수 있습니다.

**ip 자동 할당**
- 클라이언트의 할당 요청에 자동으로 ip가 할당이 됩니다.
- 할당 범위 내에 있는 ip들을 전부 사용했을 때는 `NAK PACKET`을 송신해 할당을 하지 않습니다.
- 할당된 ip와 클라이언트의 정보는 DB에 저장되며 관리됩니다.

**ip 임대 기간 연장**
- ip를 할당 받은 클라이언트의 기간 연장 요청이 있을시 설정된 `lease time`의 절반 시간만큼 연장합니다.
- 클라이언트의 직접 요청과 일정 시간 지난 후 자동으로 오는 요청 전부 동일하게 처리됩니다.

**ip 할당 해제**
- 임대 시간이 만료되었거나 클라이언트가 ip를 직접 반납을 요청할 경우 할당되었던 ip 주소는 자동으로 할당 해제됩니다. 
- 할당 해제된 ip는 바로 다른 할당 요청에 다시 사용될 수 있습니다.
- 정상적으로 ip 임대가 만료되었을 경우 종료 시점으로부터 20분간 할당되었던 정보를 확인할 수 있습니다.
## 

## Installation
```sh
git clone https://github.com/JOOYUNHAK/DHCP_Linux_Service.git
cd DHCP_Linux_Service
make
```
> 만약 mysql.h를 찾을 수 없는 경우 mysql과 libmysqlclient15-dev를 설치해주세요.

이후 아래의 명령을 통해 방화벽 설정을 해주세요.
```sh
sudo firewall-cmd --permanent --zone=public --add-port=67/udp
sudo firewall-cmd --permanent --zone=public --add-port=68/udp
sudo firewall-cmd --reload 
```

## Usage
기본적으로 사용할 수 있는 옵션들을 다음과 같습니다.
| Option | Summary | arg |
| ------ | ------ | ---- |
| -c | Config 파일 설정 | NIC |
| -n | Config 파일 조회 | NIC |
| -d | Config 파일 삭제 | NIC |
| -s | NIC에 현재 할당 ip 조회 | NIC |
| -u | NIC의 과거 할당 ip 조회 | NIC |
| start | DHCP 서버 시작 | NIC |
| stop | DHCP 서버 중지 | NIC |
| --status | DHCP 서버 상태 조회 | NIC |
| --help | 사용자 메뉴얼 |  |

### Example
#### Option "-c"
입력 NIC의 네트워크 정보를 입력합니다. 이 때 ip 범위를 잘못 입력 하시면 설정이 되지 않으며, 입력 도중 종료시키면 처음부터 재설정이 가능합니다.
DB에 입력되기 때문에 범위가 너무 크면 시간이 오래 소요될 수도 있으며, 고정으로 사용하시는 ip는 미리 DB에서 업데이트를 해야 합니다.  

<img src= "https://user-images.githubusercontent.com/99117410/230338107-23f9203c-6e4f-4fdb-be20-f3089e01ec0f.png" width = "600" height = "200" />

#### Option "-n"
입력하신 NIC의 현재 설정되어 있는 네트워크 정보를 출력합니다. 설정을 해야한다는 문구가 나온다면 아직 설정이 되어 있지 않은겁니다.  

<img src = "https://user-images.githubusercontent.com/99117410/230340004-233ba920-1d74-46b8-badc-9139bd66d718.png"  width = "600" height = "200" />

#### Option "-d"
입력한 NIC의 네트워크 정보를 삭제합니다. 설정이 이미 된 NIC의 네트워크 정보를 변경하고 싶으시면 이 옵션으로 삭제 후 재설정이 가능합니다.  

<img src = "https://user-images.githubusercontent.com/99117410/230341299-a06bdf72-001f-4501-b0ba-1597118ca298.png"  />

#### Option "start"
입력한 NIC에 대한 DHCP 서버를 시작합니다.  

<img src = "https://user-images.githubusercontent.com/99117410/230342181-91bcb9d0-a54c-43e5-afd4-825e3d7a066f.png"  />

만약 `ps aux`명령어 입력 시 위와 같이 시작한 프로세스가 보이지 않는다면 `stop` 옵션 입력 후 재시작 할 수 있습니다.  

<img src = "https://user-images.githubusercontent.com/99117410/230342704-f3b7947f-e121-4e3f-a740-5e63087f9ec0.png"  />

#### Option "-s"
현재 할당 되어있는 ip 목록을 조회할 수 있습니다.   

<img src = "https://user-images.githubusercontent.com/99117410/230347900-119856ae-d140-439a-a223-0b048f9dac90.png" />

#### Option "stop"
현재 동작중인 DHCP 서버를 중지할 수 있습니다.  

<img src = "https://user-images.githubusercontent.com/99117410/230344425-a8e060dc-d7b8-429e-b992-53b88bd4b28f.png"  />

#### Option "-u"
과거에 할당 되었었던 ip 목록을 조회합니다. 할당이 종료된 시점을 해당 옵션으로 알 수 있으며 종료된 시점으로부터 20분간 유지된 후 삭제됩니다.  

<img src = "https://user-images.githubusercontent.com/99117410/230347596-1194ebb4-52dd-4af2-9cc5-d2589b5f9956.png"  />

#### Option "--status"
입력한 NIC에 대한 DHCP Server의 상태를 알려줍니다.
정상적으로 실행중이거나 종료 되었을 경우 각각 `RUNNING`과 `DEAD`로 출력이 됩니다.  

<img src = "https://user-images.githubusercontent.com/99117410/230349828-1806a548-ebca-45a2-832f-697732fadff0.png"   />

`ERROR`가 출력 될 경우 현재 시작은 했지만 실제로 프로세스가 동작중이지 않은 상태이므로 `stop` 옵션으로 중지 후 다시 시작할 수 있습니다.  

<img src = "https://user-images.githubusercontent.com/99117410/230349125-7084d09e-f0ef-447e-a97b-1d49f4a3c908.png"  />

#### Option "--help"
옵션과 사용법에 대해 익숙하지 않은 사용자들을 위한 사용자 메뉴얼입니다.  

<img src = "https://user-images.githubusercontent.com/99117410/230348293-e6de5cc6-b766-4a18-a7c8-63c0ddfc7b15.png"  />


