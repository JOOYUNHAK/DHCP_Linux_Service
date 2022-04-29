#include <stdint.h>
#pragma pack(1)

struct DHCP_MESSAGE_TYPE{
    u_int8_t opt_type;
    u_int8_t opt_len;
    u_int8_t opt_value; 
};

struct DHCP_SERVER_IDENTIFIER{
    u_int8_t opt_type;
    u_int8_t opt_len;
    u_int32_t opt_value;
    //struct in_addr dhcp_ciaddr;; 
    
};

struct DHCP_IP_LEASE_TIME{
    u_int8_t opt_type;
    u_int8_t opt_len;
    u_int32_t opt_value;
};

struct DHCP_SUBNET{
    u_int8_t opt_type;
    u_int8_t opt_len;
    u_int32_t opt_value;
};

struct DHCP_ROUTER{
    u_int8_t opt_type;
    u_int8_t opt_len;
    u_int32_t opt_value;
};

struct DHCP_DOMAIN{
    u_int8_t opt_type;
    u_int8_t opt_len;
    u_int32_t opt_value;
};


struct DHCP_END{
    u_int8_t opt_type;
};



struct bootp {
    u_int8_t    bp_op;      /* packet opcode type */
    u_int8_t    bp_htype;   /* hardware addr type */
    u_int8_t    bp_hlen;    /* hardware addr length */
    u_int8_t    bp_hops;    /* gateway hops */
    u_int32_t   bp_xid;     /* transaction ID */
    u_int16_t   bp_secs;    /* seconds since boot began */
    u_int16_t   bp_flags;   /* flags: 0x8000 is broadcast */
    u_int32_t   bp_ciaddr;
    u_int32_t   bp_yiaddr;
    struct in_addr  bp_siaddr;  /* server IP address */
    struct in_addr  bp_giaddr;  /* gateway IP address */
    u_int8_t    bp_chaddr[16];  /* client hardware address */
    u_int8_t    bp_sname[64];   /* server host name */
    u_int8_t    bp_file[128];   /* boot file name */
    u_int8_t    bp_vend[4];    /* vendor-specific area */
    struct DHCP_MESSAGE_TYPE dhcp_message;
    struct DHCP_SERVER_IDENTIFIER dhcp_server;
    struct DHCP_IP_LEASE_TIME dhcp_lease;
    struct DHCP_ROUTER dhcp_router;
    struct DHCP_SUBNET dhcp_subnet;
    struct DHCP_DOMAIN dhcp_domain;
    struct DHCP_END dhcp_end;
};

