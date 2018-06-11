
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
    PROXY_TYPE_NONE = 0,            //��ʹ�ô���
    PROXY_TYPE_SOCKS4,              //��֧����֤��Ҳ��֧����������
    PROXY_TYPE_SOCKS4A,             //��֧����֤��֧����������
    PROXY_TYPE_SOCKS5,              //֧����֤��֧����������
    PROXY_TYPE_HTTP11,              //֧����֤��֧����������
}ProxyType;

typedef struct {
    ProxyType   type;               //��������
    char        szIP[128];          //Ŀ���ַ
    int         nPort;              //Ŀ��˿�
    char        szProxyIP[128];     //�����ַ
    int         nProxyPort;         //����˿�
	bool		bAuth;
    char        szUserName[32];    //��֤�û���
    char        szPassword[32];    //��֤����
}ProxyInfo;

#endif // _ATT_VTYPES_DEFINED





