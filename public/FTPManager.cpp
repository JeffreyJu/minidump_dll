#include "stdafx.h"
#include "FTPManager.h"
#include "string.h"
static int SplitString( std::string strSrc, std::list<std::string> &strArray , std::string strFlag)
{
	int pos = 1; 

	while((pos = (int)strSrc.find_first_of(strFlag.c_str())) > 0) 
	{
		strArray.insert(strArray.end(), strSrc.substr(0 , pos));
		strSrc = strSrc.substr(pos + 1, strSrc.length() - pos - 1); 
	}

	strArray.insert(strArray.end(), strSrc.substr(0, strSrc.length()));

	return 0; 
}

CFTPManager::CFTPManager(void): m_bLogin(false)
{
	m_cmdSocket = socket(AF_INET, SOCK_STREAM, 0);
	m_nFilesize = 0;
}

CFTPManager::~CFTPManager(void)
{
	std::string strCmdLine = parseCommand(FTP_COMMAND_QUIT, "");

	Send(m_cmdSocket, strCmdLine.c_str());
	closesocket(m_cmdSocket);
	m_bLogin = false;
}

INT32 CFTPManager::login2Server(const std::string &serverIP)
{
	std::string strPort;
	int pos = serverIP.find_first_of(":");

	if (pos > 0)
	{
		strPort = serverIP.substr(pos + 1, serverIP.length() - pos);
	}
	else
	{
		pos = serverIP.length();
		strPort = FTP_DEFAULT_PORT;
	}
	m_strServerIP = serverIP.substr(0, pos);
	m_nServerPort = atol(strPort.c_str());

//	trace("IP: %s port: %d\n", m_strServerIP.c_str(), m_nServerPort);

	if (Connect(m_cmdSocket, m_strServerIP, m_nServerPort) < 0)
	{
		
		return -1;
	}
	
	m_strResponse = serverResponse(m_cmdSocket);
	printf("@@@@Response: %s", m_strResponse.c_str());
	return	parseResponse(m_strResponse);
}

INT32 CFTPManager::inputUserName(const std::string &userName)
{
	std::string strCommandLine = parseCommand(FTP_COMMAND_USERNAME, userName);

	m_strUserName = userName;

	if (Send(m_cmdSocket, strCommandLine) < 0)
	{
		return -1;
	}

	m_strResponse = serverResponse(m_cmdSocket);
	printf("Response: %s\n", m_strResponse.c_str());

	return parseResponse(m_strResponse);
}

INT32 CFTPManager::inputPassWord(const std::string &password)
{
	std::string strCmdLine = parseCommand(FTP_COMMAND_PASSWORD, password);

	m_strPassWord = password;
	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		return -1;
	}
	else
	{
		m_bLogin = true;

		m_strResponse = serverResponse(m_cmdSocket);
		//printf("Response: %s\n", m_strResponse.c_str());

		return parseResponse(m_strResponse);
	}
}

INT32 CFTPManager::quitServer(void)
{
	std::string strCmdLine = parseCommand(FTP_COMMAND_QUIT, "");
	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		return -1;
	}
	else
	{
		m_strResponse = serverResponse(m_cmdSocket);
		//printf("Response: %s\n", m_strResponse.c_str());

		return parseResponse(m_strResponse);
	}

}

const std::string CFTPManager::PWD()
{
	std::string strCmdLine = parseCommand(FTP_COMMAND_CURRENT_PATH, "");

	if (Send(m_cmdSocket, strCmdLine.c_str()) < 0)
	{
		return "";
	}
	else
	{
		return serverResponse(m_cmdSocket);
	}
}


INT32 CFTPManager::setTransferMode(type mode)
{
	std::string strCmdLine;

	switch (mode)
	{
	case binary:
		strCmdLine = parseCommand(FTP_COMMAND_TYPE_MODE, "I");
		break;
	case ascii:
		strCmdLine = parseCommand(FTP_COMMAND_TYPE_MODE, "A");
		break;
	default:
		break;
	}

	if (Send(m_cmdSocket, strCmdLine.c_str()) < 0)
	{
		//assert(false);
	}
	else
	{	
		m_strResponse  = serverResponse(m_cmdSocket);
		//printf("@@@@Response: %s", m_strResponse.c_str());

		return parseResponse(m_strResponse);
	}
	return 0;
}


const std::string CFTPManager::Pasv()
{
	std::string strCmdLine = parseCommand(FTP_COMMAND_PSAV_MODE, "");

	if (Send(m_cmdSocket, strCmdLine.c_str()) < 0)
	{
		return "";
	}
	else
	{
		m_strResponse = serverResponse(m_cmdSocket);

		return m_strResponse;
	}
}


const std::string CFTPManager::Dir(const std::string &path)
{
	int dataSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (createDataLink(dataSocket) < 0)
	{
		return "";
	}
	// 数据连接成功
	std::string strCmdLine = parseCommand(FTP_COMMAND_DIR, path);

	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
	//	trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());
		serverResponse(m_cmdSocket).c_str();
		closesocket(dataSocket);
		return "";
	}
	else
	{
		//trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());
		std::string str = serverResponse(m_cmdSocket).c_str();
		m_strResponse = serverResponse(dataSocket);

		//trace("@@@@Response: \n%s\n", m_strResponse.c_str());
		closesocket(dataSocket);

		return m_strResponse;
	}
	
}


INT32 CFTPManager::CD(const std::string &path)
{
	assert(m_cmdSocket != INVALID_SOCKET);

	std::string strCmdLine = parseCommand(FTP_COMMAND_CHANGE_DIRECTORY, path);

	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		return -1;
	}
		
	m_strResponse = serverResponse(m_cmdSocket);
	
	//trace("@@@@Response: %s\n", m_strResponse.c_str());
	return parseResponse(m_strResponse);
}

INT32 CFTPManager::DeleteFile(const std::string &strRemoteFile)
{
	assert(m_cmdSocket != INVALID_SOCKET);

	std::string strCmdLine = parseCommand(FTP_COMMAND_DELETE_FILE, strRemoteFile);

	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		return -1;
	}

	m_strResponse = serverResponse(m_cmdSocket);
	//printf("@@@@Response: %s\n", m_strResponse.c_str());
	return parseResponse(m_strResponse);
}

INT32 CFTPManager::DeleteDirectory(const std::string &strRemoteDir)
{
	assert(m_cmdSocket != INVALID_SOCKET);

	std::string strCmdLine = parseCommand(FTP_COMMAND_DELETE_DIRECTORY, strRemoteDir);

	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		return -1;
	}
	
	m_strResponse = serverResponse(m_cmdSocket);

//	trace("@@@@Response: %s\n", m_strResponse.c_str());
	return parseResponse(m_strResponse);
}

INT32 CFTPManager::CreateDirectory(const std::string &strRemoteDir)
{
	assert(m_cmdSocket != INVALID_SOCKET);

	std::string strCmdLine = parseCommand(FTP_COMMAND_CREATE_DIRECTORY, strRemoteDir);

	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		return -1;
	}
	
	m_strResponse = serverResponse(m_cmdSocket);

//	trace("@@@@Response: %s\n", m_strResponse.c_str());
	return parseResponse(m_strResponse);
}

INT32 CFTPManager::Rename(const std::string &strRemoteFile, const std::string &strNewFile)
{
	assert(m_cmdSocket != INVALID_SOCKET);

	std::string strCmdLine = parseCommand(FTP_COMMAND_RENAME_BEGIN, strRemoteFile);
	Send(m_cmdSocket, strCmdLine);
	serverResponse(m_cmdSocket).c_str();
	//trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());

	Send(m_cmdSocket, parseCommand(FTP_COMMAND_RENAME_END, strNewFile));

	m_strResponse = serverResponse(m_cmdSocket);
//	trace("@@@@Response: %s\n", m_strResponse.c_str());
	return parseResponse(m_strResponse);
}

long CFTPManager::getFileLength(const std::string &strRemoteFile)
{
	assert(m_cmdSocket != INVALID_SOCKET);

	std::string strCmdLine = parseCommand(FTP_COMMAND_FILE_SIZE, strRemoteFile);
	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		return -1;
	}

	m_strResponse = serverResponse(m_cmdSocket);

//	trace("@@@@Response: %s\n", m_strResponse.c_str());

	std::string strData = m_strResponse.substr(0, 3);
	unsigned long val = atol(strData.c_str());

	if (val == 213)
	{
		strData = m_strResponse.substr(4);
//		trace("strData: %s\n", strData.c_str());
		val = atol(strData.c_str());

		return val;
	}

	return -1;
}


void CFTPManager::Close(int sock)
{
	shutdown(sock,SD_BOTH );
	closesocket(sock);
	sock = INVALID_SOCKET;
}

INT32 CFTPManager::Get(const std::string &strRemoteFile, const std::string &strLocalFile)
{
	return downLoad(strRemoteFile, strLocalFile);
}


INT32 CFTPManager::Put(const std::string &strRemoteFile, const std::string &strLocalFile)
{
	std::string strCmdLine;
	const unsigned long dataLen = FTP_DEFAULT_BUFFER;
	char strBuf[dataLen] = {0};
	long nSize = getFileLength(strRemoteFile);
	long nLen = 0;
// 	struct stat sBuf;
// 
// 	assert(stat(strLocalFile.c_str(), &sBuf) == 0);
// 	trace("size: %d\n", sBuf.st_size);

	FILE *pFile = fopen(strLocalFile.c_str(), "rb");  // 以只读方式打开  且文件必须存在
	assert(pFile != NULL);

	int data_fd = socket(AF_INET, SOCK_STREAM, 0);
	assert(data_fd != -1);

	if (createDataLink(data_fd) < 0)
	{
		return -1;
	}
	
	if (nSize == -1)
	{
		strCmdLine = parseCommand(FTP_COMMAND_UPLOAD_FILE, strRemoteFile);
		nSize = 0;
	}
	else
	{
		strCmdLine = parseCommand(FTP_COMMAND_APPEND_FILE, strRemoteFile);
	}

	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		Close(data_fd);
		return -1;
	}

	serverResponse(m_cmdSocket).c_str();
	//trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());

	fseek(pFile,nSize, SEEK_SET);
	while (!feof(pFile))
	{
		//SendMessage();
		nLen = fread(strBuf, 1, dataLen, pFile);
		if (nLen < 0)
		{
			break;
		}

		if (Send(data_fd, strBuf,nLen) < 0)
		{
			Close(data_fd);
			return -1;
		}
	}

	serverResponse(data_fd).c_str();
	//trace("@@@@Response: %s\n", serverResponse(data_fd).c_str());

	Close(data_fd);
	serverResponse(m_cmdSocket).c_str();
	//trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());
	fclose(pFile);

	return 0;
}

const std::string CFTPManager::parseCommand(const unsigned int command, const std::string &strParam)
{
	if (command < FTP_COMMAND_BASE || command > FTP_COMMAND_END)
	{
		return "";
	}

	std::string strCommandLine;

	m_nCurrentCommand = command;
	m_commandStr.clear();

	switch (command)
	{
	case FTP_COMMAND_USERNAME:
		strCommandLine = "USER ";
		break;
	case FTP_COMMAND_PASSWORD:
		strCommandLine = "PASS ";
		break;
	case FTP_COMMAND_QUIT:
		strCommandLine = "QUIT ";
		break;
	case FTP_COMMAND_CURRENT_PATH:
		strCommandLine = "PWD ";
		break;
	case FTP_COMMAND_TYPE_MODE:
		strCommandLine = "TYPE ";
		break;
	case FTP_COMMAND_PSAV_MODE:
		strCommandLine = "PASV ";
		break;
	case FTP_COMMAND_DIR:
		strCommandLine = "LIST ";
		break;
	case FTP_COMMAND_CHANGE_DIRECTORY:
		strCommandLine = "CWD ";
		break;
	case FTP_COMMAND_DELETE_FILE:
		strCommandLine = "DELE ";
		break;
	case FTP_COMMAND_DELETE_DIRECTORY:
		strCommandLine = "RMD ";
		break;
	case FTP_COMMAND_CREATE_DIRECTORY:
		strCommandLine = "MKD ";
		break;
	case FTP_COMMAND_RENAME_BEGIN:
		strCommandLine = "RNFR ";
		break;
	case FTP_COMMAND_RENAME_END:
		strCommandLine = "RNTO ";
		break;
	case FTP_COMMAND_FILE_SIZE:
		strCommandLine = "SIZE ";
		break;
	case FTP_COMMAND_DOWNLOAD_FILE:
		strCommandLine = "RETR ";
		break;
	case FTP_COMMAND_DOWNLOAD_POS:
		strCommandLine = "REST ";
		break;
	case FTP_COMMAND_UPLOAD_FILE:
		strCommandLine = "STOR ";
		break;
	case FTP_COMMAND_APPEND_FILE:
		strCommandLine = "APPE ";
		break;
	default :
		break;
	}

	strCommandLine += strParam;
	strCommandLine += "\r\n";

	m_commandStr = strCommandLine;
//	trace("parseCommand: %s\n", m_commandStr.c_str());

	return m_commandStr;
}

INT32 CFTPManager::Connect(int socketfd, const std::string &serverIP, unsigned int nPort)
{
	if (socketfd == INVALID_SOCKET)
	{
		return -1;
	}

	unsigned long argp = 1;
	int error = -1;
	int len = sizeof(int);
	struct sockaddr_in  addr;
	bool ret = false;
	timeval stime;
	fd_set  set;

	ioctlsocket(socketfd, FIONBIO, &argp);  //设置为非阻塞模式

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port	= htons(nPort);
	addr.sin_addr.s_addr = inet_addr(serverIP.c_str());
	memset(&(addr.sin_zero), 8,sizeof(addr.sin_zero));

//	trace("Address: %s %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	
	if (connect(socketfd, (struct sockaddr*)&addr, sizeof(struct sockaddr)) == -1)   //若直接返回 则说明正在进行TCP三次握手
	{
		stime.tv_sec = 20;  //设置为1秒超时
		stime.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(socketfd, &set);

		if (select(socketfd + 1, NULL, &set, NULL, &stime) > 0)   ///在这边等待 阻塞 返回可以读的描述符 或者超时返回0  或者出错返回-1
		{
			getsockopt(socketfd, SOL_SOCKET, SO_ERROR, (char*)&error, (int*)&len);
			if (error == 0)
			{
				ret = true;
			}
			else
			{
				ret = false;
			}
		}
	}
	else
	{	
		//trace("Connect Immediately!!!\n");
		ret = true;
	}

	argp = 0;
	ioctlsocket(socketfd, FIONBIO, &argp);

	if (!ret)
	{
		closesocket(socketfd);
		fprintf(stderr, "cannot connect server!!\n");
		return -1;
	}

	//fprintf(stdout, "Connect!!!\n");

	return 0;
}


const std::string CFTPManager::serverResponse(int sockfd)
{
	if (sockfd == INVALID_SOCKET)
	{
		return "";
	}
	
	int nRet = -1;
	char buf[MAX_PATH] = {0};

	m_strResponse.clear();

	while ((nRet = getData(sockfd, buf, MAX_PATH - 1)) > 0)
	{
		buf[nRet] = '\0';
		m_strResponse += buf;
	}

	return m_strResponse;
}

INT32 CFTPManager::getData(int fd, char *strBuf, unsigned long length)
{
	assert(strBuf != NULL);

	if (fd == INVALID_SOCKET)
	{
		return -1;
	}

	memset(strBuf, 0, length);
	timeval stime;
	int nLen;

	stime.tv_sec = 1;
	stime.tv_usec = 0;

	fd_set	readfd;
	FD_ZERO( &readfd );
	FD_SET(fd, &readfd );

	if (select(fd + 1, &readfd, 0, 0, &stime) > 0)
	{
		if ((nLen = recv(fd, strBuf, length, 0)) > 0)
		{
			return nLen;
		}
		else
		{
			return -2;
		}
	}
	return 0;
}

INT32 CFTPManager::Send(int fd, const std::string &cmd)
{
	if (fd == INVALID_SOCKET)
	{
		return -1;
	}

	return Send(fd, cmd.c_str(), cmd.length());
}

INT32 CFTPManager::Send(int fd, const char *cmd, const size_t len)
{
	if((FTP_COMMAND_USERNAME != m_nCurrentCommand) 
		&&(FTP_COMMAND_PASSWORD != m_nCurrentCommand)
		&&(!m_bLogin))
	{
		return -1;
	}

	
	timeval timeout;
	timeout.tv_sec  = 2;
	timeout.tv_usec = 0;

	fd_set  writefd;
	FD_ZERO(&writefd);  
	FD_SET(fd, &writefd);
	
	int ret = select(fd + 1, 0, &writefd , 0 , &timeout);
	if(ret > 0)
	{
		size_t nlen  = len; 
		int nSendLen = 0; 
		while (nlen >0) 
		{
			nSendLen = send(fd, cmd , (int)nlen , 0);
			if(nSendLen == -1) 
				return -2; 
			m_nFilesize += nSendLen;
			nlen = nlen - nSendLen;
			cmd +=  nSendLen;
		}
		return 0;
	}
	return -1;
}


INT32 CFTPManager::createDataLink(int data_fd)
{
	assert(data_fd != INVALID_SOCKET);

	std::string strData;
	unsigned long nPort = 0 ;
	std::string strServerIp ; 
	std::list<std::string> strArray ;

	std::string parseStr = Pasv();

	if (parseStr.size() <= 0)
	{
		return -1;
	}

	//trace("parseInfo: %s\n", parseStr.c_str());

	size_t nBegin = parseStr.find_first_of("(");
	size_t nEnd	  = parseStr.find_first_of(")");
	strData		  = parseStr.substr(nBegin + 1, nEnd - nBegin - 1);

	//trace("ParseAfter: %s\n", strData.c_str());
	if( SplitString( strData , strArray , "," ) <0)
		return -1;

	if( ParseString( strArray , nPort , strServerIp) < 0)
		return -1;

	//trace("nPort: %ld IP: %s\n", nPort, strServerIp.c_str());

	if (Connect(data_fd, strServerIp, nPort) < 0)
	{
		return -1;
	}

	return 0;

}

INT32 CFTPManager::ParseString(std::list<std::string> strArray, unsigned long & nPort ,std::string & strServerIp)
{
	if (strArray.size() < 6 )
		return -1 ;

	std::list<std::string>::iterator citor;
	citor = strArray.begin();
	strServerIp = *citor;
	strServerIp += ".";
	citor ++;
	strServerIp += *citor;
	strServerIp += ".";
	citor ++ ;
	strServerIp += *citor;
	strServerIp += ".";
	citor ++ ;
	strServerIp += *citor;
	citor = strArray.end();
	citor--;
	nPort = atol( (*citor).c_str());
	citor--;
	nPort += atol( (*(citor)).c_str()) * 256 ;
	return 0 ; 
}

FILE *CFTPManager::createLocalFile(const std::string &strLocalFile)
{
	return fopen(strLocalFile.c_str(), "w+b");
}

INT32 CFTPManager::downLoad(const std::string &strRemoteFile, const std::string &strLocalFile, const int pos, const unsigned int length)
{
	assert(length >= 0);

	FILE *file = NULL;
	unsigned long nDataLen = FTP_DEFAULT_BUFFER;
	char strPos[MAX_PATH]  = {0};
	int data_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	assert(data_fd != -1);

	if ((length != 0) && (length < nDataLen))
	{
		nDataLen = length;
	}
	char *dataBuf = new char[nDataLen];
	assert(dataBuf != NULL);

	sprintf(strPos, "%d", pos);

	if (createDataLink(data_fd) < 0)
	{
		//trace("@@@@ Create Data Link error!!!\n");
		return -1;
	}

	std::string strCmdLine = parseCommand(FTP_COMMAND_DOWNLOAD_POS, strPos);
	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		return -1;
	}

	 serverResponse(m_cmdSocket).c_str();
	//trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());

	strCmdLine = parseCommand(FTP_COMMAND_DOWNLOAD_FILE, strRemoteFile);

	if (Send(m_cmdSocket, strCmdLine) < 0)
	{
		return -1;
	}
	//trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());
	serverResponse(m_cmdSocket).c_str();
	std::string s = strLocalFile;
	s = s.substr(0,s.length()-1);
	file = createLocalFile(s);//(std::string(FTP_DEFAULT_PATH + strLocalFile));
	assert(file != NULL);
	
	int len = 0;
	int nReceiveLen = 0;
	while ((len = getData(data_fd, dataBuf, nDataLen)) > 0)
	{
		nReceiveLen += len;

		int num = fwrite(dataBuf, 1, len, file);
		memset(dataBuf, 0, sizeof(dataBuf));
	
		//trace("%s", dataBuf);
	//	trace("Num:%d\n", num);
		if (nReceiveLen == length && length != 0)
			break;

		if ((nReceiveLen + nDataLen) > length  && length != 0)
		{
			delete []dataBuf;
			nDataLen = length - nReceiveLen;
			dataBuf = new char[nDataLen];
		}
	}

	Close(data_fd);
	fclose(file);
	delete []dataBuf;

	return 0;
}

INT32 CFTPManager::parseResponse(const std::string &str)
{
	if(!str.empty())
	{
		std::string strData = str.substr(0, 3);
		return atoi(strData.c_str());
	}
	return 0;
}

void CFTPManager::ClearFileSize()
{
	m_nFilesize = 0;
}
