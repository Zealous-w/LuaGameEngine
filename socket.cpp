#include "socket.h"
#include "log.h"
#include "lua_interface.h"
#include "protocal.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>

using namespace std;

#define SERVER_COMMAND_PACKETVERERR  0x901	//鍗忚鐗堟湰鍑洪敊 鎻愮ず鏇存柊瀹㈡埛绔?
string policystr ="<cross-domain-policy><allow-access-from domain=\"*\" to-ports=\"*\" /></cross-domain-policy>\0" ;

// tgw 
static const char* const TGW_HEADER = "tgw_l7_forward\r\nHost: app100643177.qzoneapp.com:8001\r\n\r\n";

extern lua_State* L;
NETInputPacket in_package;
extern int now;

//鍙戦€佸瓧鑺傛槧灏勮〃
BYTE SocketHandler::m_SendByteMap[256]=				
{
	0x70,0x2F,0x40,0x5F,0x44,0x8E,0x6E,0x45,0x7E,0xAB,0x2C,0x1F,0xB4,0xAC,0x9D,0x91,
	0x0D,0x36,0x9B,0x0B,0xD4,0xC4,0x39,0x74,0xBF,0x23,0x16,0x14,0x06,0xEB,0x04,0x3E,
	0x12,0x5C,0x8B,0xBC,0x61,0x63,0xF6,0xA5,0xE1,0x65,0xD8,0xF5,0x5A,0x07,0xF0,0x13,
	0xF2,0x20,0x6B,0x4A,0x24,0x59,0x89,0x64,0xD7,0x42,0x6A,0x5E,0x3D,0x0A,0x77,0xE0,
	0x80,0x27,0xB8,0xC5,0x8C,0x0E,0xFA,0x8A,0xD5,0x29,0x56,0x57,0x6C,0x53,0x67,0x41,
	0xE8,0x00,0x1A,0xCE,0x86,0x83,0xB0,0x22,0x28,0x4D,0x3F,0x26,0x46,0x4F,0x6F,0x2B,
	0x72,0x3A,0xF1,0x8D,0x97,0x95,0x49,0x84,0xE5,0xE3,0x79,0x8F,0x51,0x10,0xA8,0x82,
	0xC6,0xDD,0xFF,0xFC,0xE4,0xCF,0xB3,0x09,0x5D,0xEA,0x9C,0x34,0xF9,0x17,0x9F,0xDA,
	0x87,0xF8,0x15,0x05,0x3C,0xD3,0xA4,0x85,0x2E,0xFB,0xEE,0x47,0x3B,0xEF,0x37,0x7F,
	0x93,0xAF,0x69,0x0C,0x71,0x31,0xDE,0x21,0x75,0xA0,0xAA,0xBA,0x7C,0x38,0x02,0xB7,
	0x81,0x01,0xFD,0xE7,0x1D,0xCC,0xCD,0xBD,0x1B,0x7A,0x2A,0xAD,0x66,0xBE,0x55,0x33,
	0x03,0xDB,0x88,0xB2,0x1E,0x4E,0xB9,0xE6,0xC2,0xF7,0xCB,0x7D,0xC9,0x62,0xC3,0xA6,
	0xDC,0xA7,0x50,0xB5,0x4B,0x94,0xC0,0x92,0x4C,0x11,0x5B,0x78,0xD9,0xB1,0xED,0x19,
	0xE9,0xA1,0x1C,0xB6,0x32,0x99,0xA3,0x76,0x9E,0x7B,0x6D,0x9A,0x30,0xD6,0xA9,0x25,
	0xC7,0xAE,0x96,0x35,0xD0,0xBB,0xD2,0xC8,0xA2,0x08,0xF3,0xD1,0x73,0xF4,0x48,0x2D,
	0x90,0xCA,0xE2,0x58,0xC1,0x18,0x52,0xFE,0xDF,0x68,0x98,0x54,0xEC,0x60,0x43,0x0F
};

//鎺ユ敹瀛楄妭鏄犲皠琛?
BYTE SocketHandler::m_RecvByteMap[256]=				
{
	0x51,0xA1,0x9E,0xB0,0x1E,0x83,0x1C,0x2D,0xE9,0x77,0x3D,0x13,0x93,0x10,0x45,0xFF,
	0x6D,0xC9,0x20,0x2F,0x1B,0x82,0x1A,0x7D,0xF5,0xCF,0x52,0xA8,0xD2,0xA4,0xB4,0x0B,
	0x31,0x97,0x57,0x19,0x34,0xDF,0x5B,0x41,0x58,0x49,0xAA,0x5F,0x0A,0xEF,0x88,0x01,
	0xDC,0x95,0xD4,0xAF,0x7B,0xE3,0x11,0x8E,0x9D,0x16,0x61,0x8C,0x84,0x3C,0x1F,0x5A,
	0x02,0x4F,0x39,0xFE,0x04,0x07,0x5C,0x8B,0xEE,0x66,0x33,0xC4,0xC8,0x59,0xB5,0x5D,
	0xC2,0x6C,0xF6,0x4D,0xFB,0xAE,0x4A,0x4B,0xF3,0x35,0x2C,0xCA,0x21,0x78,0x3B,0x03,
	0xFD,0x24,0xBD,0x25,0x37,0x29,0xAC,0x4E,0xF9,0x92,0x3A,0x32,0x4C,0xDA,0x06,0x5E,
	0x00,0x94,0x60,0xEC,0x17,0x98,0xD7,0x3E,0xCB,0x6A,0xA9,0xD9,0x9C,0xBB,0x08,0x8F,
	0x40,0xA0,0x6F,0x55,0x67,0x87,0x54,0x80,0xB2,0x36,0x47,0x22,0x44,0x63,0x05,0x6B,
	0xF0,0x0F,0xC7,0x90,0xC5,0x65,0xE2,0x64,0xFA,0xD5,0xDB,0x12,0x7A,0x0E,0xD8,0x7E,
	0x99,0xD1,0xE8,0xD6,0x86,0x27,0xBF,0xC1,0x6E,0xDE,0x9A,0x09,0x0D,0xAB,0xE1,0x91,
	0x56,0xCD,0xB3,0x76,0x0C,0xC3,0xD3,0x9F,0x42,0xB6,0x9B,0xE5,0x23,0xA7,0xAD,0x18,
	0xC6,0xF4,0xB8,0xBE,0x15,0x43,0x70,0xE0,0xE7,0xBC,0xF1,0xBA,0xA5,0xA6,0x53,0x75,
	0xE4,0xEB,0xE6,0x85,0x14,0x48,0xDD,0x38,0x2A,0xCC,0x7F,0xB1,0xC0,0x71,0x96,0xF8,
	0x3F,0x28,0xF2,0x69,0x74,0x68,0xB7,0xA3,0x50,0xD0,0x79,0x1D,0xFC,0xCE,0x8A,0x8D,
	0x2E,0x62,0x30,0xEA,0xED,0x2B,0x26,0xB9,0x81,0x7C,0x46,0x89,0x73,0xA2,0xF7,0x72
};

SocketHandler::SocketHandler()
	:_fd(-1),
	_is_encrypt(false),
	_is_parse_proto(true),
	_conn_flag(CONNECTION_TYPE_CLIENT),
	_is_closed(false),
	_read(new lgs_event_t),
	_write(new lgs_event_t),
	_live_time(now)
{
	memset(_recvbuf, 0, RECV_BUFFER);
}

SocketHandler::SocketHandler(int socket_fd, 
							 bool parse_protocal, 
							 bool encrypt, 
							 int conn_type)
	:_fd(socket_fd),
	_is_encrypt(encrypt),
	_is_parse_proto(parse_protocal),
	_conn_flag(conn_type),
	_is_closed(false),
	_read(new lgs_event_t),
	_write(new lgs_event_t),
	_live_time(now)
{
	memset(_recvbuf, 0, RECV_BUFFER);
}

SocketHandler::~SocketHandler() 
{
	::close(_fd);
}

int SocketHandler::handle_read()
{
	int nrecved = recv(_fd, _recvbuf, sizeof(_recvbuf), 0);

	// log_debug("nrecved: %d", nrecved);
	
	if (nrecved == 0) {
		return -1;
	} else if (nrecved == -1) {
		if (errno != EINTR || errno != EAGAIN ) return -1;

		return 0; 
	}
	
	//log_debug("TGW Begin.");
	
	//判断接受tgw跳转包头
	if (nrecved == 56) {
		string tgw(_recvbuf, nrecved);

		//log_debug("tgw: %s\n %u", tgw.c_str(), tgw.size());

		string org(TGW_HEADER, nrecved);

		//log_debug("org:%s\n %u", org.c_str(), org.size());

		if (!strncmp(tgw.c_str(), org.c_str(), 48)) {
			return 0;
		}
	}
	
	//log_debug("TGW End.");
		
	if(_is_parse_proto) {
		int ret = OnParser(_recvbuf, nrecved);
		
		if(ret != 0) {
			log_debug("parser error.\n");
			return -1;
		}
	} else {
		lua_getglobal(L, "handle_input");
		lua_pushnumber(L, nrecved);
		lua_pushlstring(L, _recvbuf, nrecved);
		if (lua_pcall(L, 1, 1, 0) != 0) {
			return -1;
		}
        lua_pop(L,-1);
	}

	return 0;
}

int SocketHandler::handle_output()
{
	return 0;
}

int SocketHandler::handle_close()
{
	int ret = 0;
	
	if(_conn_flag == CONNECTION_TYPE_CLIENT) {
		call_lua("handle_client_socket_close", "d>d", _fd, &ret);
	} else {
		call_lua("handle_server_socket_close", "dd>d", _fd, _conn_flag, &ret);
	}
	
    lua_pop(L,-1);
	return ret;
}

void SocketHandler::handle_timeout()
{
//	TRACE("sockethandler handle timeout\n");
}

int SocketHandler::OnParser(char *buf, int len)
{
	if(len == 23 && buf[0] == '<' && buf[1] == 'p') {
		string policy = "<policy-file-request/>";
		for(int i=0; i<23; ++i) {
			if(buf[i] != policy[i]) {
				return -1;
			}
		}

		send(_fd, policystr.c_str(), (int)policystr.size(), 0);
		return -1;
	}

	int ret = ParsePacket(buf, len);
	
	if (ret == -3) return SendPacketVarErr();

	return ret;
} 

/*
BYTE SocketHandler::MapSendByte(BYTE const cbData)
{                                                 
    BYTE cbMap = m_SendByteMap[(BYTE)(cbData)];   
    return cbMap;                                 
}                                                 
                                                  
BYTE SocketHandler::MapRecvByte(BYTE const cbData)
{                                                 
    BYTE cbMap=m_RecvByteMap[cbData];             
    return cbMap;                                 
}                                                 
*/

WORD 
SocketHandler::EncryptBuffer(NETOutputPacket *pPacket)
{	
	if(pPacket->IsWritecbCheckCode()) return 0;

	BYTE *pcbDataBuffer = (BYTE *)pPacket->packet_buf() + NETOutputPacket::PACKET_HEADER_SIZE;
	WORD wDataSize = pPacket->GetBodyLength() - (NETOutputPacket::PACKET_HEADER_SIZE == 14 ? 12 : 7);
	
	BYTE cbCheckCode = 0;

	const unsigned int block = 8;
	unsigned int block_limit = wDataSize / block * block;
	unsigned int i = 0;

	while (i < block_limit) {
		cbCheckCode += pcbDataBuffer[i] + pcbDataBuffer[i + 1] + 
							pcbDataBuffer[i + 2]+ pcbDataBuffer[i + 3] + pcbDataBuffer[i + 4] + 
								pcbDataBuffer[i + 5] + pcbDataBuffer[i + 6]+ pcbDataBuffer[i + 7];

		pcbDataBuffer[i] 	 = MapSendByte(pcbDataBuffer[i]);
		pcbDataBuffer[i + 1] = MapSendByte(pcbDataBuffer[i + 1]);
		pcbDataBuffer[i + 2] = MapSendByte(pcbDataBuffer[i + 2]);
		pcbDataBuffer[i + 3] = MapSendByte(pcbDataBuffer[i + 3]);
		pcbDataBuffer[i + 4] = MapSendByte(pcbDataBuffer[i + 4]);
		pcbDataBuffer[i + 5] = MapSendByte(pcbDataBuffer[i + 5]);
		pcbDataBuffer[i + 6] = MapSendByte(pcbDataBuffer[i + 6]);
		pcbDataBuffer[i + 7] = MapSendByte(pcbDataBuffer[i + 7]);

		i += 8;
	}

	if (i < wDataSize) {
		switch (wDataSize - i) {
			case 7 : cbCheckCode += pcbDataBuffer[i]; pcbDataBuffer[i] = MapSendByte(pcbDataBuffer[i]); i++;
			case 6 : cbCheckCode += pcbDataBuffer[i]; pcbDataBuffer[i] = MapSendByte(pcbDataBuffer[i]); i++;
			case 5 : cbCheckCode += pcbDataBuffer[i]; pcbDataBuffer[i] = MapSendByte(pcbDataBuffer[i]); i++;
			case 4 : cbCheckCode += pcbDataBuffer[i]; pcbDataBuffer[i] = MapSendByte(pcbDataBuffer[i]); i++;
			case 3 : cbCheckCode += pcbDataBuffer[i]; pcbDataBuffer[i] = MapSendByte(pcbDataBuffer[i]); i++;
			case 2 : cbCheckCode += pcbDataBuffer[i]; pcbDataBuffer[i] = MapSendByte(pcbDataBuffer[i]); i++;
			case 1 : cbCheckCode += pcbDataBuffer[i]; pcbDataBuffer[i] = MapSendByte(pcbDataBuffer[i]); 
		}
	}

	pPacket->WritecbCheckCode(~cbCheckCode + 1);

	return wDataSize;
}

int 
SocketHandler::CrevasseBuffer(NETInputPacket *pPacket)
{
	BYTE *pcbDataBuffer = (BYTE *)pPacket->packet_buf() + NETOutputPacket::PACKET_HEADER_SIZE; 
	
	WORD wDataSize = pPacket->GetBodyLength() - (NETOutputPacket::PACKET_HEADER_SIZE == 14 ? 12 : 7); 

	BYTE cbCheckCode = pPacket->GetcbCheckCode();

	//log_debug("cbCheckCode: %d", cbCheckCode);
	//log_debug("wDataSize: %d", wDataSize);
	//log_debug("Body: %d", pPacket->GetBodyLength());		

	const unsigned int block = 8;
	unsigned int block_limit = wDataSize / block * block;
	unsigned int i = 0;

	//log_debug("block_limit: %d", block_limit);	
	
	while (i < block_limit) {
		pcbDataBuffer[i] 	 = MapRecvByte(pcbDataBuffer[i]);
		pcbDataBuffer[i + 1] = MapRecvByte(pcbDataBuffer[i + 1]);
		pcbDataBuffer[i + 2] = MapRecvByte(pcbDataBuffer[i + 2]);
		pcbDataBuffer[i + 3] = MapRecvByte(pcbDataBuffer[i + 3]);
		pcbDataBuffer[i + 4] = MapRecvByte(pcbDataBuffer[i + 4]);
		pcbDataBuffer[i + 5] = MapRecvByte(pcbDataBuffer[i + 5]);
		pcbDataBuffer[i + 6] = MapRecvByte(pcbDataBuffer[i + 6]);
		pcbDataBuffer[i + 7] = MapRecvByte(pcbDataBuffer[i + 7]);

		cbCheckCode += pcbDataBuffer[i] + pcbDataBuffer[i + 1] + 
							pcbDataBuffer[i + 2]+ pcbDataBuffer[i + 3] + pcbDataBuffer[i + 4] + 
								pcbDataBuffer[i + 5] + pcbDataBuffer[i + 6]+ pcbDataBuffer[i + 7];

		i += 8;
	}

	if (i < wDataSize) {
		switch (wDataSize - i) {
			case 7 : pcbDataBuffer[i] = MapRecvByte(pcbDataBuffer[i]); cbCheckCode += pcbDataBuffer[i]; i++;
			case 6 : pcbDataBuffer[i] = MapRecvByte(pcbDataBuffer[i]); cbCheckCode += pcbDataBuffer[i]; i++;
			case 5 : pcbDataBuffer[i] = MapRecvByte(pcbDataBuffer[i]); cbCheckCode += pcbDataBuffer[i]; i++;
			case 4 : pcbDataBuffer[i] = MapRecvByte(pcbDataBuffer[i]); cbCheckCode += pcbDataBuffer[i]; i++;
			case 3 : pcbDataBuffer[i] = MapRecvByte(pcbDataBuffer[i]); cbCheckCode += pcbDataBuffer[i]; i++;
			case 2 : pcbDataBuffer[i] = MapRecvByte(pcbDataBuffer[i]); cbCheckCode += pcbDataBuffer[i]; i++;
			case 1 : pcbDataBuffer[i] = MapRecvByte(pcbDataBuffer[i]); cbCheckCode += pcbDataBuffer[i]; 
		}
	}

	if (cbCheckCode != 0 ) return -1;
	
	return wDataSize;
}


int 
SocketHandler::SendPacketVarErr(void)
{
	NETOutputPacket response;
	response.Begin(SERVER_COMMAND_PACKETVERERR);
	response.oldEnd();
	return send_packet(&response);
}

static int 
parser_parmeters(const char* p, NETInputPacket *pPacket,int count, int &narg)
{
	for(int i=0; i<count; i++) {
        const char * temp = p;
        while(*temp) {
			switch (*temp++) {
     		case 'b':                        //byte
     		{
     			BYTE b = pPacket->ReadByte();

				lua_pushnumber(L, b);
		        narg++;
     			break;
     		}
     		case 'd':						//int
     		{
     			int n = pPacket->ReadInt();
     			lua_pushnumber(L, n);
		        narg++;
                break;
     		}
     		case 'h':						//short
     		{	
				short n = pPacket->ReadShort();
     			lua_pushnumber(L, n);
		        narg++;
     			break;
     		}
     		case 'u':						//unsigned long
     		{	unsigned int u = pPacket->ReadULong();
     			lua_pushnumber(L, u);
		        narg++;
     			break;
     		}
     		case 's':						//string
     		{	
     			string s = pPacket->ReadString();
     			lua_pushstring(L, s.c_str());
		        narg++;
                break;
     		}
     		case 'B':
     		{
     			char binary[4096] = {0};
     			int n = pPacket->ReadBinary(binary, sizeof(binary));
     			lua_pushlstring(L, binary, n);
		        narg++;
     			break;
     		}
            case ']':						//array
     		{
     			//BYTE b = pPacket->ReadByte();
     			//lua_pushnumber(L, b);
		        //narg++;
                break;
                break;
     		}
     		default:
			{
				break;
			}
			}
        }
    }

	//log_debug("narg1: %d\n", narg);
	
    while(*p)
    {
        if(*p++ == ']')
        return 0;
    }

	return 0;
}

int SocketHandler::OnPacketComplete(NETInputPacket *pPacket)
{
	if(_is_encrypt) {
		if(CrevasseBuffer(pPacket) == -1) {
			log_debug("CrevasseBuffer error.");
			return -1;
		}
	}

	short cmd =	pPacket->GetCmdType();	
	Message message = CProtocal::get_message(cmd);
	if (message.cmd == 0) {
		log_error("can't find the message:0x%x\n", cmd);
		return -1;
	} else {
		_live_time = now;
		//log_debug("_live_time: %d", _live_time);	
	}
	
	int ret = 0;
	if (strcmp(message.format, "") == 0) {
		if(call_lua(message.call_back, ">d", &ret) == -1) {
            return -1;
        }
        
		return ret;
	}

	const char* p = message.format;
    lua_getglobal(L, message.call_back);  /* get function */
 
	//lua_pushhandled(L, this);
	lua_pushnumber(L, _fd);

    /* push arguments */
    int narg = 1;
    while (*p) {    /* push arguments */
		//log_debug("p:%s\n", p);
		
		switch (*p++) {
		case 'b':						//byte
		{
			BYTE b = pPacket->ReadByte();
			lua_pushnumber(L, b);
            narg++;
			break;
		}
		case 'd':						//int
		{
			int n = pPacket->ReadInt();
			//printf("n = %d\n", n);
			lua_pushnumber(L, n);
            narg++;
            break;
		}
		case 'h':						//short
		{	short n = pPacket->ReadShort();
			lua_pushnumber(L, n);
            narg++;
			break;
		}
		case 'u':						//unsigned long
		{	unsigned int u = pPacket->ReadULong();
			lua_pushnumber(L, u);
            narg++;
			break;
		}
		case 's':						//string
		{	
			string s = pPacket->ReadString();
			lua_pushstring(L, s.c_str());
            narg++;
            break;
		}
		case 'B':
		{
			char binary[4096] = {0};
			int n = pPacket->ReadBinary(binary, sizeof(binary));
			lua_pushlstring(L, binary, n);
            narg++;
			break;
		}
        
        case 'a':						//array
		{
			BYTE b = pPacket->ReadByte();
			lua_pushnumber(L, b);
            narg++;
            parser_parmeters(p,pPacket,b,narg);
			break;
		}
        
        case 'p':						//array
		{
            in_package.Copy(pPacket->packet_buf(),pPacket->packet_size());
            printf("the packet recv size is%d\n",pPacket->packet_size());
			break;
		}

		default:
		{
			//log_debug(p);
			//log_error("parse param error");
			break;
		}
		}
		luaL_checkstack(L, 1, "too many arguments");
	} 


	//log_debug("narg: %d\n", narg);
	/* do the call */
    if (lua_pcall(L, narg, 1, 0) != 0)  /* do the call */
	{
		log_error("error running function `%s': %s", message.call_back, lua_tostring(L, -1));
		return -1;
	}

    ret = (int)lua_tonumber(L, -1);
    lua_pop(L,-1);
	//TRACE("call lua function:%s, return = %d\n", message.call_back, ret);
	return ret;
}

int SocketHandler::send_packet(NETOutputPacket *pPacket)
{
	if(_is_encrypt)
	{
		EncryptBuffer(pPacket);
	}
	return send(_fd, pPacket->packet_buf(), pPacket->packet_size(), 0);
}

void SocketHandler::build_package(NETOutputPacket* pOutPack, short nCmdType, const char* pszFmt, ...)
{
	pOutPack->Begin(nCmdType);

	if (pszFmt == NULL)	//鍐呭涓虹┖鐨?灏辩洿鎺ユ瀯閫犱竴涓彧鏈夊懡浠ゅご鐨勬暟鎹寘
	{
		pOutPack->End();
		return;
	}

	va_list ap; 
	va_start (ap, pszFmt); 
	const char* p = NULL;

	// BUG:
	// 璋冪敤涓€涓笉甯﹀師鍨嬪０鏄庣殑鍑芥暟鏃讹細
	// 璋冪敤鑰呬細瀵规瘡涓弬鏁版墽琛屸€滈粯璁ゅ疄闄呭弬鏁版彁鍗?default argument promotions)鈥濄€?
	// 鎵€浠?'h' 涓簲璇ユ槸 int val = va_arg(ap, int)銆傜劧鍚庡己鍒惰浆鎹€?
	// 鍙傝€?man 鍜?http://www.cppblog.com/ownwaterloo/archive/2009/04/21/unacceptable_type_in_va_arg.html
	for (p= pszFmt; *p; p++) 
	{ 
		if (*p != '%') 
		{
			continue; 
		}

		switch (*++p) 
		{ 
		case 'd':	//int
			{
				int nVal= va_arg(ap, int);
				pOutPack->WriteInt(nVal);
				break;
			}
		case 'h':	//short
			{
				short shVal = va_arg(ap, int);
				pOutPack->WriteShort(shVal);
				break;
			}
		case 'u':	//unsigned long
			{
				unsigned long dwVal = va_arg(ap, unsigned long);
				pOutPack->WriteULong(dwVal);
				break;
			}
		case 's':	//char*
			{
				char* pVal = va_arg(ap, char*);
				pOutPack->WriteString(pVal);
				break;
			}
		}
	}
	pOutPack->End();
}

void
SocketHandler::clean(bool IsClosefd)
{
	if (IsClosefd) ::close(_fd);

	_fd = -1;
	_is_encrypt = true;
	_is_parse_proto = true;
	_conn_flag = CONNECTION_TYPE_CLIENT;
	_live_time = 0;
	reset();
}

void
SocketHandler::assign(int sockt_fd, 
					  bool parse_protocal, 
					  bool encrypt, 
					  int conn_type)
{
	_fd = sockt_fd;
	_is_encrypt = encrypt;
	_is_parse_proto = parse_protocal;
	_conn_flag = conn_type;
	_live_time = now;
	memset(_recvbuf, 0, RECV_BUFFER);
}
