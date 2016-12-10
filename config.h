#ifndef CONFIG_H
#define CONFIG_H

#define BROADCAST_IP       "224.102.228.40"
#define BROADCAST_PORT      7789

#define SELF_PORT  7788
#define ROOM_NO     6
#define ROOM_CTL_UNIT_IP    "192.168.1.21"
#define ROOM_CTL_UNIT_PORT  8880
#define SYS_CONTROLLER_IP "192.168.1.60" //系统控制器服务器
#define SYS_CONTROLLER_PORT   8880
#define CAR_CONTROLLER_IP   "192.168.1.11" //车厢控制器服务器
#define CAR_CONTROLLER_PORT      8880
#define VIDEO_SERVER_IP "192.168.1.20"
//#define VIDEO_SERVER_IP "192.168.1.108"
#ifdef  __MAIN__
const char* VIDEO_SERVER_URL_ARR[]={
    "http://" VIDEO_SERVER_IP  ":8092/test1.sdp",
    "http://" VIDEO_SERVER_IP  ":8094/test2.sdp",
    "http://" VIDEO_SERVER_IP  ":8096/test3.sdp",
    "http://" VIDEO_SERVER_IP  ":8098/test4.sdp",
    "http://" VIDEO_SERVER_IP  ":8100/test5.sdp",
    "http://" VIDEO_SERVER_IP  ":8102/test6.sdp",
    "http://" VIDEO_SERVER_IP  ":8104/test7.sdp",
    "http://" VIDEO_SERVER_IP  ":8106/test8.sdp",
    "http://" "192.168.1.123"  ":8090/test2.sdp",
};
#else
extern const char* VIDEO_SERVER_URL_ARR[];
#endif
#endif // CONFIG_H
