#include "StdAfx.h"
#include "FtpUpLoad.h"


CFtpUpLoad::CFtpUpLoad()//IFtpUpLoad* pHander)
{
	//m_pFtpmanager = new CFTPManager();
	//m_pHander = pHander;
	m_nStatus = -1;
	m_bStop = true;
}


CFtpUpLoad::~CFtpUpLoad(void)
{
	//delete m_pFtpmanager;
}

CFtpUpLoad* CFtpUpLoad::GetInstance()
{
	if(m_pThis == NULL)
	{
		m_pThis = new CFtpUpLoad();
	}
	return m_pThis;
}

void CFtpUpLoad::Release()
{
	if(m_pThis)
	{
		delete m_pThis;
		m_pThis = NULL;
	}
}

INT32 CFtpUpLoad::InitFTP(std::string ip,std::string username,std::string password,std::string strDirPath)
{
	m_strDirPath = strDirPath;
	m_strIp = ip;
	m_strUserName = username;
	m_strPassWord = password;
	m_bStop = false;
	m_strLocalPath = GetRootPath();
	m_strLocalPath += "\\" + strDirPath + "\\ftp";
	Run();
	return 1;
}

INT32 CFtpUpLoad::UninitFTP()
{
//	printf("UninitFTP");
	m_bStop = true;
	CFTPManager::Close(data_fd);
//	printf("UninitFTP2");
	closesocket(m_cmdSocket);
//	printf("UninitFTP3");
	Stop();
//	printf("UninitFTP4");
	return 1;
}

void CFtpUpLoad::Svc()
{

	while(!m_bStop)
	{
		//登陆
		//InterlockedExchange(&m_nStatus,-1);
		OnEvent(eFTP_BeginLogin,0,"",0);
		if(looplogin())
		{
			printf("ftp 登陆完成\n");
			//InterlockedExchange(&m_nStatus,0);

			OnEvent(eFTP_LoginOk,0,"",0);

			//创建更目录
			//查看目录是否存在
			std::string strPath("\\");
			strPath += m_strDirPath;
			INT32 ret = CD(strPath);
			if(ret != 250)
			{
				ret = CreateDirectory(m_strDirPath);
				if(ret != 257)
				{
					continue;
				}
			}
			//InterlockedExchange(&m_nStatus,1);
			//处理任务
			//查询目录
			/*
			drwxrwxrwx   1 user     group           0 Jun  8 17:28 .
			drwxrwxrwx   1 user     group           0 Jun  8 17:28 ..
			-rw-rw-rw-   1 user     group    54737828 Jun  8 17:28 ys.MOV
			-rw-rw-rw-   1 user     group    134897664 Jun  8 17:38 智能控制系统操控视频V1.0.wmv
			*/
			const std::string strDir = Dir(strPath);
			char* pStr = new char[strDir.length() + 1];
			strcpy(pStr,strDir.c_str());
			std::vector<char*> vcDir;
			SplitString(pStr,'\r',vcDir);
			m_mapFTPHanderLock.Lock();
			m_vcFileInfo.clear();
			for (unsigned int i = 0; i < vcDir.size();i++)
			{
				//不能处理含文件名含空格的情况,正常获取方式是找到':'后偏移4个字符
				char* pDir = vcDir[i];
				std::vector<char*> vcDir2;
				SplitString(pDir,' ',vcDir2);
				FtpFilesInfo info;
				for (unsigned int k = 0;k < vcDir2.size();k++)
				{
					char* pStrInfo = vcDir2[k];
					char* pos = strrchr(pStrInfo,'.');
					if(pos && !(strcmp(info.FileName,".") == 0 || strcmp(info.FileName,"..") == 0))
					{
						strcpy(info.FileName,pStrInfo);
					}
					else
					{
						if(StrIsDecNumA(pStrInfo,strlen(pStrInfo)))
						{
							int filessize  = atoi(pStrInfo);
							if(filessize >100)
							{
								info.nFileSize = filessize;
							}
						}
					}
				}

				if(info.nFileSize > 0 && info.FileName[0] != 0)
				{
					m_vcFileInfo.push_back(info);
				}
			}
			OnEvent(eFTP_Browse,0,"",0);
			m_mapFTPHanderLock.Unlock();
			delete[] pStr;
			printf("ftp 浏览完成\n");
			InterlockedExchange(&m_nStatus,2);
			looptast();
		}
	}
}

INT32 CFtpUpLoad::UploadOneFile(const std::string &strLocalFile )
{
	std::string strCmdLine;
	CString Strtemp;
	Strtemp = strLocalFile.c_str();
	std::string strRemoteFile;
	CFile cfile;  
	unsigned long filesize;
	if (cfile.Open(Strtemp, CFile::modeRead))  
	{  
		Strtemp = cfile.GetFileName();
		int iSize = WideCharToMultiByte(CP_ACP, 0, Strtemp, -1, NULL, 0, NULL, NULL);
		char *ch;
		ch = (char*)malloc((iSize+1));
		WideCharToMultiByte(CP_ACP, 0, Strtemp, -1, ch, iSize, NULL, NULL);
		strRemoteFile = "/";
		strRemoteFile += m_strDirPath + "/";
		strRemoteFile += ch;
		filesize = cfile.GetLength();
		cfile.Close();
		free(ch);
	}
	else
	{
		return -1;
	}
	const unsigned long dataLen = FTP_DEFAULT_BUFFER;
	char strBuf[dataLen] = {0};
	long nSize = getFileLength(strRemoteFile);
	unsigned long nLen = 0;
	// 	struct stat sBuf;
	// 
	// 	assert(stat(strLocalFile.c_str(), &sBuf) == 0);
	// 	trace("size: %d\n", sBuf.st_size);

	FILE *pFile = fopen(strLocalFile.c_str(), "rb");  // 以只读方式打开  且文件必须存在
	if(pFile == NULL)
	{
		OnEvent(eFTP_OpenLocalFileErr,GetLastError(),strLocalFile,0);
		fclose(pFile);
		return 0;
	}
	
	data_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(data_fd == -1)
	{
		OnEvent(eFTP_CreateSocketErr,GetLastError(),strLocalFile,0);
		fclose(pFile);
		return -1;
	}

	if (createDataLink(data_fd) < 0)
	{
		CFTPManager::Close(data_fd);
		fclose(pFile);
		return -1;
	}

	if (nSize == -1)
	{
		strCmdLine = parseCommand(FTP_COMMAND_UPLOAD_FILE, strRemoteFile);
	}
	else
	{
		strCmdLine = parseCommand(FTP_COMMAND_APPEND_FILE, strRemoteFile);
	}

	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		CFTPManager::Close(data_fd);
		data_fd = -1;
		fclose(pFile);
		return -1;
	}

	serverResponse(m_cmdSocket).c_str();
	//trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());

	fseek(pFile, nSize, SEEK_SET);
	while (!feof(pFile))
	{
		nLen = fread(strBuf, 1, dataLen, pFile);
		if (nLen < 0)
		{
			CFTPManager::Close(data_fd);
			data_fd = -1;
			fclose(pFile);
			return -1;
		}
		if (Send(data_fd, strBuf,nLen)< 0)
		{
			CFTPManager::Close(data_fd);
			data_fd = -1;
			fclose(pFile);
			return -1;
		}

		int temp = m_nFilesize*1.0/filesize * 100;
		OnEvent(eFTP_UpdateProgress,GetLastError(),strLocalFile,temp);
	}
	OnEvent(eFTP_UpdateEnd,GetLastError(),strLocalFile,0);
	serverResponse(data_fd).c_str();
//	trace("@@@@Response: %s\n", serverResponse(data_fd).c_str());

	CFTPManager::Close(data_fd);
	data_fd = -1;
	//trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());
	serverResponse(m_cmdSocket).c_str();
	fclose(pFile);
	return 0;
}

void CFtpUpLoad::OnEvent( eFtpUpLoadEvent eEvent,DWORD wErrCode,std::string strPath ,int UpdateProgress)
{
	static int iii = 0;
	//printf("%u %d %d\n",m_pHander.size(),eEvent,iii++);
	
	if(eEvent == eFTP_Browse)
	{
		for (unsigned int i = 0; i < m_pHander.size(); i ++)
			m_pHander[i]->OnEvent(eEvent,wErrCode,strPath,UpdateProgress);
	}
	else
	{
		m_mapFTPHanderLock.Lock();
		for (unsigned int i = 0; i < m_pHander.size(); i ++)
			m_pHander[i]->OnEvent(eEvent,wErrCode,strPath,UpdateProgress);
		m_mapFTPHanderLock.Unlock();
	}
	
	

	/*
	else
	{
		m_mapFTPHanderLock.Lock();
		for (unsigned int i = 0; i < m_pHander.size(); i ++)
			m_pHander[i] -> OnEvent(eEvent,wErrCode,strPath,UpdateProgress);
		m_mapFTPHanderLock.Unlock();
	}
	*/
}
void CFtpUpLoad::AddHander( IFtpUpLoad* pHander )
{
	//m_pHander = pHander;
	m_mapFTPHanderLock.Lock();
	m_pHander.push_back(pHander);
	m_mapFTPHanderLock.Unlock();
}

void CFtpUpLoad::RemoveHander( IFtpUpLoad* pHander )
{
	m_mapFTPHanderLock.Lock();
	std::vector<IFtpUpLoad*>::iterator it = m_pHander.begin();
	for(;it != m_pHander.end();)
	{
		if((*it) == pHander)
			it = m_pHander.erase(it);
		else
			it++;
	}
	m_mapFTPHanderLock.Unlock();
}

INT32 CFtpUpLoad::DownLoadOneFile( const std::string &strRemoteFile,long filesize)
{
	const int pos = 0;
	FILE *file = NULL;
	unsigned long nDataLen = FTP_DEFAULT_BUFFER;
	long long nReceiveLen = 0;
	char strPos[MAX_PATH]  = {0};
	char *dataBuf = NULL;
	data_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (nDataLen > filesize)
	{
		dataBuf = new char[filesize];
		nDataLen = filesize;
	}
	else
	{
		dataBuf = new char[nDataLen];
	}
	
	sprintf(strPos, "%d", pos);
	if (createDataLink(data_fd) < 0)
	{
	//	trace("@@@@ Create Data Link error!!!\n");
		closesocket(data_fd);
		data_fd = -1;
		delete []dataBuf;
		return -1;
	}

	std::string strCmdLine = parseCommand(FTP_COMMAND_DOWNLOAD_POS, strPos);
	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		closesocket(data_fd);
		data_fd = -1;
		delete []dataBuf;
		return -1;
	}
	//trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());
	serverResponse(m_cmdSocket).c_str();

	std::string strRemote("/");
	strRemote += m_strDirPath;
	strRemote += "/" + strRemoteFile;
	strCmdLine = parseCommand(FTP_COMMAND_DOWNLOAD_FILE,strRemote);

	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		closesocket(data_fd);
		data_fd = -1;
		delete []dataBuf;
		return -1;
	}
	serverResponse(m_cmdSocket).c_str();
//	trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());
	std::string s = m_strLocalPath + "\\" + strRemoteFile;
	file = fopen(s.c_str(), "w+b");
	if(file == NULL)
	{
		closesocket(data_fd);
		data_fd = -1;
		delete []dataBuf;
		return -1;
	}

	static int old = -1;
	//
	int len = 0;
	while ((len = getData(data_fd, dataBuf, nDataLen)) > 0)
	{
		nReceiveLen += len;

	//	printf("下载数据 % d %d %d\n",data_fd,len,m_bStop);

		int num = fwrite(dataBuf, 1, len, file);
		memset(dataBuf, 0, sizeof(dataBuf));
		int temp = nReceiveLen*100/filesize;

	//	printf("下载数据2 % d %d %d\n",data_fd,len,m_bStop);

		if(!m_bStop)
		{
			if(old < temp)
			{
				OnEvent(eFTP_DownLoadProgress,GetLastError(),strRemoteFile,temp);
				old = temp;
			}
		}
		
	//	printf("下载数据3 % d %d %d\n",data_fd,len,m_bStop);

		if (nReceiveLen + nDataLen > filesize)
		{
			nDataLen = filesize - nReceiveLen;
			if(nDataLen > 0)
			{
				delete []dataBuf;
				dataBuf = new char[nDataLen];
			}
			else
			{
				len = 0;
				break;
			}
		}
	}

	old = -1;
	//printf("下载数据3 %d\n",len);

	if(len < 0)
	{	//下载失败
		CFTPManager::Close(data_fd);
		data_fd = -1;
		fclose(file);
		delete []dataBuf;
		return -1;
	}

	delete []dataBuf;
	CFTPManager::Close(data_fd);
	data_fd = -1;
	fclose(file);

	if(!m_bStop)
		OnEvent(eFTP_DownLoadEnd,GetLastError(),"",0);
	return 0;
}



bool CFtpUpLoad::looplogin()
{
	while(!m_bStop)
	{
		if(login())
		{
			return true;
		}
		//
	}
	return false;
}

bool CFtpUpLoad::looptast()
{
	while(!m_bStop)
	{
		InterlockedExchange(&m_nStatus,3);
		tagFTPTast tast;
		if(this->GetFirstTast(tast))
		{
			if(this->Tast(tast))
			{
				if(tast.nTastType == 1)
				{
					const std::string strDir = Dir("/" + m_strDirPath);
					char* pStr = new char[strDir.length() + 1];
					strcpy(pStr,strDir.c_str());
					std::vector<char*> vcDir;
					SplitString(pStr,'\r',vcDir);
					m_mapFTPHanderLock.Lock();
					m_vcFileInfo.clear();
					for (unsigned int i = 0; i < vcDir.size();i++)
					{
						char* pDir = vcDir[i];
						std::vector<char*> vcDir2;
						SplitString(pDir,' ',vcDir2);
						FtpFilesInfo info;
						for (unsigned int k = 0;k < vcDir2.size();k++)
						{
							char* pStrInfo = vcDir2[k];
							char* pos = strrchr(pStrInfo,'.');
							if(pos && !(strcmp(info.FileName,".") == 0 || strcmp(info.FileName,"..") == 0))
							{
								strcpy(info.FileName,pStrInfo);
							}
							else
							{
								if(StrIsDecNumA(pStrInfo,strlen(pStrInfo)))
								{
									int filessize  = atoi(pStrInfo);
									if(filessize >100)
									{
										info.nFileSize = filessize;
									}
								}
							}
						}

						if(info.nFileSize > 0 && info.FileName[0] != 0)
						{
							m_vcFileInfo.push_back(info);
						}
					}
					m_mapFTPHanderLock.Unlock();
					delete[] pStr;
					OnEvent(eFTP_Browse,0,"",0);
				}
			}
		}//end GetFirstTast

		this->RemoveTast(tast.nTastId,tast);
		
		while(tast.nTastId > 0)
		{
			if(this->Tast(tast))
			{
				if(tast.nTastType == 1)
				{
					const std::string strDir = Dir("/" + m_strDirPath);
					char* pStr = new char[strDir.length() + 1];
					strcpy(pStr,strDir.c_str());
					std::vector<char*> vcDir;
					SplitString(pStr,'\r',vcDir);
					m_mapFTPHanderLock.Lock();
					m_vcFileInfo.clear();
					for (unsigned int i = 0; i < vcDir.size();i++)
					{
						char* pDir = vcDir[i];
						std::vector<char*> vcDir2;
						SplitString(pDir,' ',vcDir2);
						FtpFilesInfo info;
						for (unsigned int k = 0;k < vcDir2.size();k++)
						{
							char* pStrInfo = vcDir2[k];
							char* pos = strrchr(pStrInfo,'.');
							if(pos && !(strcmp(info.FileName,".") == 0 || strcmp(info.FileName,"..") == 0))
							{
								strcpy(info.FileName,pStrInfo);
							}
							else
							{
								if(StrIsDecNumA(pStrInfo,strlen(pStrInfo)))
								{
									int filessize  = atoi(pStrInfo);
									if(filessize >100)
									{
										info.nFileSize = filessize;
									}
								}
							}
						}

						if(info.nFileSize > 0 && info.FileName[0] != 0)
						{
							m_vcFileInfo.push_back(info);
						}
					}
					m_mapFTPHanderLock.Unlock();
					delete[] pStr;

					OnEvent(eFTP_Browse,0,"",0);
				}
			}
			this->RemoveTast(tast.nTastId,tast);
		}
		InterlockedExchange(&m_nStatus,eFTP_Readines);
		Sleep(100);
	}
	return true;
}



bool CFtpUpLoad::login()
{
	INT32 nret = login2Server(m_strIp);
	if(nret < 0)
		return false;

	nret = inputUserName(m_strUserName);
	if(nret < 0)
		return false;

	nret =  inputPassWord(m_strPassWord);
	if(nret < 0)
		return false;
	nret = setTransferMode(binary);
	if(nret < 0)
		return false;
	return true;
}

bool CFtpUpLoad::AddTast( const tagFTPTast& tast )
{
	m_mapFTPHanderLock.Lock();
	/*
	if(m_vcFtpTast.size() > 0)
	{
		m_mapFTPHanderLock.Unlock();
		return false;
	}
	*/
	m_vcFtpTast.push_back(tast);
	m_mapFTPHanderLock.Unlock();
	return true;
}
bool CFtpUpLoad::RemoveTast( INT32 nTastId,tagFTPTast& nextTast)
{
	nextTast.nTastId = -1;
	m_mapFTPHanderLock.Lock();
	std::vector<tagFTPTast>::iterator it = m_vcFtpTast.begin();
	for (;it != m_vcFtpTast.end();it++)
	{
		if(it->nTastId == nTastId)
		{
			it = m_vcFtpTast.erase(it);
			if(it != m_vcFtpTast.end())
			{
				nextTast = *it;
			}
			m_mapFTPHanderLock.Unlock();
			return true;
		}
	}
	m_mapFTPHanderLock.Unlock();
	return false;
}

bool	CFtpUpLoad::GetFirstTast( tagFTPTast& tast)
{
	m_mapFTPHanderLock.Lock();
	if(m_vcFtpTast.size() > 0)
	{
		tast = m_vcFtpTast[0];
		m_mapFTPHanderLock.Unlock();
		return true;
	}
	m_mapFTPHanderLock.Unlock();
	return false;
}

bool CFtpUpLoad::Tast( const tagFTPTast& info )
{
	unsigned int i = 0;
	switch(info.nTastType)
	{
		case 1://上传文件
			{
				for (i = 0;i<info.vcTastPath.size() && !m_bStop;i++)
				{
					this->UploadOneFile(info.vcTastPath[i].FileName);
					if(m_bStop)
						break;
				}
			}break;
		case 2://下载文件
			{
				for (i = 0;i < info.vcTastPath.size()  && !m_bStop;i++)
				{
					this->DownLoadOneFile(info.vcTastPath[i].FileName,info.vcTastPath[i].nFileSize);
					if(m_bStop)
						break;
				}

				if(!m_bStop)
					OnEvent(eFTP_DownLoadNoting,0,"",0);
			}break;
		default:
			break;
	}
	return i >= info.vcTastPath.size();
}

INT32 CFtpUpLoad::GetRemoteFileInfos( std::vector<FtpFilesInfo>& vcInfos )
{
	
	m_mapFTPHanderLock.Lock();
	vcInfos = m_vcFileInfo;
	m_mapFTPHanderLock.Unlock();
	return vcInfos.size();
}

bool	CFtpUpLoad::GetRemoteFile(const char* szName,FtpFilesInfo& info)
{
	m_mapFTPHanderLock.Lock();
	std::vector<FtpFilesInfo>::iterator it = m_vcFileInfo.begin();
	for (;it != m_vcFileInfo.end();it++)
	{
		if(strcmp(szName,(*it).FileName) == 0)
		{
			info = *it;
			m_mapFTPHanderLock.Unlock();
			return true;
		}
	}
	m_mapFTPHanderLock.Unlock();
	return false;
}

long CFtpUpLoad::GetStatus()
{
	long ret;
	InterlockedExchange(&ret,m_nStatus);
	return ret;
}

bool CFtpUpLoad::DeleteFile(const std::string & strRemoteFile)
{
	FtpFilesInfo ffi;
	int pos;
	if((pos = strRemoteFile.find_last_of('\\'))==-1)
	{//分隔符错误
		pos = strRemoteFile.find_last_of('/');
	}
	string str_temp(strRemoteFile.substr(pos+1));//截取字符串路径中的文件名
	if(GetRemoteFile(str_temp.c_str(),ffi)){
		int ret = CFTPManager::DeleteFile(strRemoteFile);//父类deletefile的方法
		if( ret == 250 || ret == 550){//删除成功,或者文件已不存在
			//删除成功
			//更新本地文件
			m_mapFTPHanderLock.Lock();
			int i = 0;
			std::vector<FtpFilesInfo>::iterator it=  m_vcFileInfo.begin();
			for (;it !=m_vcFileInfo.end();)
			{//移除已删除文件
				if(str_temp.compare(it->FileName)==0)
				{
					m_vcFileInfo.erase(it);
					break;
				}
				it++;i++;
			}//*/
			m_mapFTPHanderLock.Unlock();
			return true;
		}
	}
	return false;
}


CFtpUpLoad* CFtpUpLoad::m_pThis = NULL;
