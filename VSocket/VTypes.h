
// VTypes.h

// RFB V3.0

// Datatypes used by the VGui system

#if (!defined(_ATT_VTYPES_DEFINED))
#define _ATT_VTYPES_DEFINED

////////////////////////////
// Numeric data types

////////////////////////////
// Fixed size (derived from rfb.h)

typedef unsigned int VCard32;
typedef unsigned short VCard16;
typedef unsigned char VCard8;
typedef int VInt32;
typedef short VInt16;
typedef char VInt8;

////////////////////////////
// Variable size
//		These will always be at least as big as the largest
//		fixed-size data-type

typedef VCard32 VCard;
typedef VInt32 VInt;

////////////////////////////
// Useful functions on integers

static inline VInt Max(VInt x, VInt y) {if (x>y) return x; else return y;}
static inline VInt Min(VInt x, VInt y) {if (x<y) return x; else return y;}

////////////////////////////
// Others

typedef char VChar;
#if (!defined(NULL))
#define NULL 0
#endif

////////////////////////////
// Compound data types

// #include "rfbgui/VPoint.h"
// #include "rfbgui/VRect.h"
typedef VChar * VString;

////////////////////////////
// Proxy data types
typedef enum {
    PROXY_TYPE_NONE = 0,            //不使用代理
    PROXY_TYPE_SOCKS4,              //不支持认证，也不支持域名解析
    PROXY_TYPE_SOCKS4A,             //不支持认证，支持域名解析
    PROXY_TYPE_SOCKS5,              //支持认证，支持域名解析
    PROXY_TYPE_HTTP11,              //支持认证，支持域名解析
}ProxyType;

typedef struct {
    ProxyType   type;               //代理类型
    char        szIP[128];          //目标地址
    int         nPort;              //目标端口
    char        szProxyIP[128];     //代理地址
    int         nProxyPort;         //代理端口
	bool		bAuth;
    char        szUserName[32];    //认证用户名
    char        szPassword[32];    //认证密码
}ProxyInfo;

#endif // _ATT_VTYPES_DEFINED





