#pragma once
#include "Thread.h"
#include "FTPManager.h"
#include "strconver.h"
//#include <vector>
enum eFtpUpLoadEvent
{
	eFTP_BeginLogin,
	eFTP_LoginOk,
	eFTP_LoginErr,
	eFTP_TranserFile,
	eFTP_TranserFileErr,
	eFTP_OpenLocalFileErr,
	eFTP_CreateSocketErr,
	eFTP_UpdateProgress,
	eFTP_UpdateEnd,
	eFTP_DownLoadProgress,
	eFTP_DownLoadEnd,
	eFTP_DownLoadNoting,
	eFTP_Browse,
	eFTP_Readines
};

#define WM_FTP_BEGINLOGIN		  (WM_USER+99)
#define WM_FTP_UPDATEPROGRESS (WM_USER+100)
#define WM_FTP_UPLOADEND	  (WM_USER+101)
#define WM_FTP_LOGINOK		  (WM_USER+102)
#define WM_FTP_BROWSER		  (WM_USER+103)
#define WM_FTP_DOWNLOADPROGRESS (WM_USER + 104)
#define WM_FTP_DOWNLOADEND	    (WM_USER + 105)
#define WM_FTP_DOWNLOADNOTING   (WM_USER + 106)
#define WM_FTP_READY		  (WM_USER+107)


class IFtpUpLoad 
{
public:
	virtual void OnEvent(eFtpUpLoadEvent eEvent,DWORD wErrCode,std::string strPath,int UpdateProgress){};
};

struct FtpFilesInfo
{
	char	FileName[MAX_PATH];
	UINT32  nFileSize; 
	FtpFilesInfo()
	{
		memset(this,0,sizeof(FtpFilesInfo));
	}
};

struct tagFTPTast
{
	INT32								nTastId;		//任务id
	INT32								nTastType;		//任务类型0-浏览，1-上传，2-下载
	std::vector<FtpFilesInfo>			vcTastPath;		//
};

class CFtpUpLoad : public CFTPManager,public CVThread,public IFtpUpLoad
{
public:
	CFtpUpLoad();//(IFtpUpLoad* pHander);
	virtual ~CFtpUpLoad(void);
	//CFTPManager* m_pFtpmanager;
public:
	virtual void OnEvent(eFtpUpLoadEvent eEvent,DWORD wErrCode,std::string strPath,int UpdateProgress);
public:
	static CFtpUpLoad*	GetInstance();
	static void			Release();

	INT32				InitFTP(std::string ip,std::string username,std::string password,std::string strDirPath);
	INT32				UninitFTP();

	void				AddHander(IFtpUpLoad* pHander);
	void				RemoveHander(IFtpUpLoad* pHander);

	INT32				UploadOneFile(const std::string &strLocalFile);
	INT32				DownLoadOneFile(const std::string &strRemoteFile,long filesize);

	INT32				GetRemoteFileInfos(std::vector<FtpFilesInfo>& vcInfos);
	bool				GetRemoteFile(const char* szName,FtpFilesInfo& info);

	bool				looplogin();
	bool				login();
	bool				looptast();

	bool				AddTast( const tagFTPTast& tast );
	bool				RemoveTast( INT32 nTastId,tagFTPTast& nextTast);
	bool				GetFirstTast( tagFTPTast& tast);

	bool				Tast(const tagFTPTast& info);
	long				GetStatus();
	virtual void		Svc();

	bool				DeleteFile(const std::string& strRemoteFile);//删除远程文件，同时更新本地文件
	//CPanelManage* mpPanelManage;
private:
	static CFtpUpLoad*			m_pThis;
	std::string					m_strDirPath;
	std::string					m_strLocalPath;
	std::vector<IFtpUpLoad*>	m_pHander;
	long						m_nStatus;
	bool						m_bStop;
	std::vector<tagFTPTast>		m_vcFtpTast;
	std::vector<FtpFilesInfo>	m_vcFileInfo;
	int							data_fd;
	CLock						m_mapFTPHanderLock;

	std::string					m_strIp;
	std::string					m_strUserName;
	std::string					m_strPassWord;
};

