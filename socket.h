#ifndef BOYAA_SOCKET_HANDLER_H_20110312
#define BOYAA_SOCKET_HANDLER_H_20110312

#include "PacketBase.h"
#include "wtypedef.h"
#include <stdint.h>

#ifdef TCP_BUFFER_SIZE
#define RECV_BUFFER TCP_BUFFER_SIZE 
#else
#define RECV_BUFFER 4096
#endif

#define CONNECTION_TYPE_CLIENT	0


typedef struct lgs_event_s lgs_event_t;
struct lgs_event_s {
	unsigned char instance:1;
	unsigned char active:1;
	unsigned char ready:1;
	unsigned char :0;	
};


class SocketHandler : public PacketParser<NETInputPacket> {
public:
	SocketHandler();
	SocketHandler(int sockt_fd, 
				  bool parse_protocal = true, 
				  bool encrypt = true, 
				  int conn_type = 0);
	~SocketHandler();

public:
	int handle_read();
	int handle_output();
	int handle_close();

	int fd() { return _fd; }
	static void handle_timeout();
	
	inline void SetClosed(bool b) { _is_closed = b; }
	inline bool IsServerClosed() { return _is_closed;}

	int SendPacketVarErr(void);
	int OnParser(char *buf, int nLen);
	virtual int  OnPacketComplete(NETInputPacket *);

	void clean(bool IsClosefd);
	void assign(int sockt_fd, 
		   		bool parse_protocal = true, 
		   		bool encrypt = true, 
		   		int conn_type = 0);
		   		
	lgs_event_t* readev() { return _read; }
	lgs_event_t* writeev() { return _write; }

	int GetLiveTime() { return _live_time; }
public:
	static BYTE	m_SendByteMap[256];		//字节映射表
	static BYTE	m_RecvByteMap[256];		//字节映射表
	static WORD EncryptBuffer(NETOutputPacket *pPacket);
	static int  CrevasseBuffer(NETInputPacket *pPacket);

	static inline BYTE MapSendByte(BYTE const cbData) { BYTE cbMap = m_SendByteMap[cbData]; return cbMap; }
	static inline BYTE MapRecvByte(BYTE const cbData) { BYTE cbMap = m_RecvByteMap[cbData]; return cbMap; }
	
	int send_packet(NETOutputPacket *pPacket);
	void build_package(NETOutputPacket* pOutPack, short nCmdType, const char* pszFmt, ...);
private:
	int  _fd;
	char _recvbuf[RECV_BUFFER];
	char _sendbuf[RECV_BUFFER]; /* 发送缓冲 */
	
	bool _is_encrypt;
	bool _is_parse_proto;	//是否由Server解析协议
	int  _conn_flag;		//连接标识, 通过flag来区别连接类型, 如为0表示客户端连接，可用其他数字区别大厅，后端连接

	bool _is_closed;

	lgs_event_t* _read;
	lgs_event_t* _write;

	int	_live_time;
};

#endif

