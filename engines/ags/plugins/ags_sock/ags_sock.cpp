/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/array.h"
#include "common/str.h"
#include "ags/plugins/ags_sock/ags_sock.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSock {

IAGSEngine *AGSSock::_engine;

typedef Common::Array<byte> SockData;

class SockAddr {
public:
	int _port = 0;
	Common::String _address;
	Common::String _ip;
};

class Socket {
public:
};

AGSSock::AGSSock() : PluginBase() {
	_engine = nullptr;

	DLL_METHOD(AGS_GetPluginName);
	DLL_METHOD(AGS_EngineStartup);
}

const char *AGSSock::AGS_GetPluginName() {
	return "AGS Sock";
}

void AGSSock::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(SockData::Create^2, SockData_Create);
	SCRIPT_METHOD_EXT(SockData::CreateEmpty^0, SockData_CreateEmpty);
	SCRIPT_METHOD_EXT(SockData::CreateFromString^1, SockData_CreateFromString);
	SCRIPT_METHOD_EXT(SockData::get_Size, SockData_get_Size);
	SCRIPT_METHOD_EXT(SockData::set_Size, SockData_set_Size);
	SCRIPT_METHOD_EXT(SockData::geti_Chars, SockData_geti_Chars);
	SCRIPT_METHOD_EXT(SockData::seti_Chars, SockData_seti_Chars);
	SCRIPT_METHOD_EXT(SockData::AsString^0, SockData_AsString);
	SCRIPT_METHOD_EXT(SockData::Clear^0, SockData_Clear);
	SCRIPT_METHOD_EXT(SockAddr::Create^1, SockAddr_Create);
	SCRIPT_METHOD_EXT(SockAddr::CreateFromString^2, SockAddr_CreateFromString);
	SCRIPT_METHOD_EXT(SockAddr::CreateFromData^1, SockAddr_CreateFromData);
	SCRIPT_METHOD_EXT(SockAddr::CreateIP^2, SockAddr_CreateIP);
	SCRIPT_METHOD_EXT(SockAddr::CreateIPv6^2, SockAddr_CreateIPv6);
	SCRIPT_METHOD_EXT(SockAddr::get_Port, SockAddr_get_Port);
	SCRIPT_METHOD_EXT(SockAddr::set_Port, SockAddr_set_Port);
	SCRIPT_METHOD_EXT(SockAddr::get_Address, SockAddr_get_Address);
	SCRIPT_METHOD_EXT(SockAddr::set_Address, SockAddr_set_Address);
	SCRIPT_METHOD_EXT(SockAddr::get_IP, SockAddr_get_IP);
	SCRIPT_METHOD_EXT(SockAddr::set_IP, SockAddr_set_IP);
	SCRIPT_METHOD_EXT(SockAddr::GetData^0, SockAddr_GetData);
	SCRIPT_METHOD_EXT(Socket::Create^3, Socket_Create);
	SCRIPT_METHOD_EXT(Socket::CreateUDP^0, Socket_CreateUDP);
	SCRIPT_METHOD_EXT(Socket::CreateTCP^0, Socket_CreateTCP);
	SCRIPT_METHOD_EXT(Socket::CreateUDPv6^0, Socket_CreateUDPv6);
	SCRIPT_METHOD_EXT(Socket::CreateTCPv6^0, Socket_CreateTCPv6);
	SCRIPT_METHOD_EXT(Socket::get_Tag, Socket_get_Tag);
	SCRIPT_METHOD_EXT(Socket::set_Tag, Socket_set_Tag);
	SCRIPT_METHOD_EXT(Socket::get_Local, Socket_get_Local);
	SCRIPT_METHOD_EXT(Socket::get_Remote, Socket_get_Remote);
	SCRIPT_METHOD_EXT(Socket::get_Valid, Socket_get_Valid);
	SCRIPT_METHOD_EXT(Socket::ErrorString^0, Socket_ErrorString);
	SCRIPT_METHOD_EXT(Socket::Bind^1, Socket_Bind);
	SCRIPT_METHOD_EXT(Socket::Listen^1, Socket_Listen);
	SCRIPT_METHOD_EXT(Socket::Connect^2, Socket_Connect);
	SCRIPT_METHOD_EXT(Socket::Accept^0, Socket_Accept);
	SCRIPT_METHOD_EXT(Socket::Close^0, Socket_Close);
	SCRIPT_METHOD_EXT(Socket::Send^1, Socket_Send);
	SCRIPT_METHOD_EXT(Socket::SendTo^2, Socket_SendTo);
	SCRIPT_METHOD_EXT(Socket::Recv^0, Socket_Recv);
	SCRIPT_METHOD_EXT(Socket::RecvFrom^1, Socket_RecvFrom);
	SCRIPT_METHOD_EXT(Socket::SendData^1, Socket_SendData);
	SCRIPT_METHOD_EXT(Socket::SendDataTo^2, Socket_SendDataTo);
	SCRIPT_METHOD_EXT(Socket::RecvData^0, Socket_RecvData);
	SCRIPT_METHOD_EXT(Socket::RecvDataFrom^1, Socket_RecvDataFrom);
	SCRIPT_METHOD_EXT(Socket::GetOption^2, Socket_GetOption);
	SCRIPT_METHOD_EXT(Socket::SetOption^3, Socket_SetOption);

}

void AGSSock::SockData_Create(ScriptMethodParams &params) {
	PARAMS2(int, size, char, defchar);

	SockData *data = new SockData();
	data->resize(size);
	Common::fill(&(*data)[0], &(*data)[0] + size, defchar);

	params._result = data;
}

void AGSSock::SockData_CreateEmpty(ScriptMethodParams &params) {
	params._result = new SockData();
}

void AGSSock::SockData_CreateFromString(ScriptMethodParams &params) {
	PARAMS1(const char *, str);
	size_t len = strlen(str);

	SockData *data = new SockData();
	data->resize(len + 1);
	Common::copy(str, str + len + 1, &(*data)[0]);

	params._result = data;
}

void AGSSock::SockData_get_Size(ScriptMethodParams &params) {
	PARAMS1(SockData *, sockData);
	params._result = sockData->size();
}

void AGSSock::SockData_set_Size(ScriptMethodParams &params) {
	PARAMS2(SockData *, sockData, size_t, size);
	sockData->resize(size);
}

void AGSSock::SockData_geti_Chars(ScriptMethodParams &params) {
	PARAMS1(SockData *, sockData);
	params._result = &(*sockData)[0];
}

void AGSSock::SockData_seti_Chars(ScriptMethodParams &params) {
	PARAMS2(SockData *, sockData, const byte *, chars);
	Common::copy(chars, chars + sockData->size(), &(*sockData)[0]);
}

void AGSSock::SockData_AsString(ScriptMethodParams &params) {
	PARAMS1(SockData *, sockData);
	params._result = (const char *)&(*sockData)[0];
}

void AGSSock::SockData_Clear(ScriptMethodParams &params) {
	PARAMS1(SockData *, sockData);
	sockData->clear();
}

void AGSSock::SockAddr_Create(ScriptMethodParams &params) {
//	PARAMS1(int, type);
	params._result = new SockAddr();
}

void AGSSock::SockAddr_CreateFromString(ScriptMethodParams &params) {
//	PARAMS2(const char *, address, int, type);
	PARAMS1(const char *, address);

	SockAddr *sock = new SockAddr();
	sock->_address = address;

	params._result = sock;
}

void AGSSock::SockAddr_CreateFromData(ScriptMethodParams &params) {
//	PARAMS1(const SockData *, data);
	params._result = new SockAddr();
}

void AGSSock::SockAddr_CreateIP(ScriptMethodParams &params) {
	PARAMS2(const char *, address, int, port);

	SockAddr *sock = new SockAddr();
	sock->_address = address;
	sock->_port = port;

	params._result = sock;
}

void AGSSock::SockAddr_CreateIPv6(ScriptMethodParams &params) {
	//PARAMS2(const char *, address, int, port);
	PARAMS1(const char *, address);

	SockAddr *sock = new SockAddr();
	sock->_address = address;

	params._result = sock;
}

void AGSSock::SockAddr_get_Port(ScriptMethodParams &params) {
	PARAMS1(const SockAddr *, sockAddr);
	params._result = sockAddr->_port;
}

void AGSSock::SockAddr_set_Port(ScriptMethodParams &params) {
	PARAMS2(SockAddr *, sockAddr, int, port);
	sockAddr->_port = port;
}

void AGSSock::SockAddr_get_Address(ScriptMethodParams &params) {
	PARAMS1(const SockAddr *, sockAddr);
	params._result = sockAddr->_address.c_str();
}

void AGSSock::SockAddr_set_Address(ScriptMethodParams &params) {
	PARAMS2(SockAddr *, sockAddr, const char *, address);
	sockAddr->_address = address;
}

void AGSSock::SockAddr_get_IP(ScriptMethodParams &params) {
	PARAMS1(const SockAddr *, sockAddr);
	params._result = sockAddr->_ip.c_str();
}

void AGSSock::SockAddr_set_IP(ScriptMethodParams &params) {
	PARAMS2(SockAddr *, sockAddr, const char *, IP);
	sockAddr->_ip = IP;
}

void AGSSock::SockAddr_GetData(ScriptMethodParams &params) {
//	PARAMS1(const SockAddr *, sockAddr);
	params._result = new SockData();
}


void AGSSock::Socket_Create(ScriptMethodParams &params) {
	//PARAMS3(int, domain, int, type, int, protocol);
	params._result = new Socket();
}

void AGSSock::Socket_CreateUDP(ScriptMethodParams &params) {
}

void AGSSock::Socket_CreateTCP(ScriptMethodParams &params) {
}

void AGSSock::Socket_CreateUDPv6(ScriptMethodParams &params) {
}

void AGSSock::Socket_CreateTCPv6(ScriptMethodParams &params) {
}

void AGSSock::Socket_get_Tag(ScriptMethodParams &params) {
}

void AGSSock::Socket_set_Tag(ScriptMethodParams &params) {
}

void AGSSock::Socket_get_Local(ScriptMethodParams &params) {
}

void AGSSock::Socket_get_Remote(ScriptMethodParams &params) {
}

void AGSSock::Socket_get_Valid(ScriptMethodParams &params) {
}

void AGSSock::Socket_ErrorString(ScriptMethodParams &params) {
}

void AGSSock::Socket_Bind(ScriptMethodParams &params) {
}

void AGSSock::Socket_Listen(ScriptMethodParams &params) {
}

void AGSSock::Socket_Connect(ScriptMethodParams &params) {
}

void AGSSock::Socket_Accept(ScriptMethodParams &params) {
}

void AGSSock::Socket_Close(ScriptMethodParams &params) {
}

void AGSSock::Socket_Send(ScriptMethodParams &params) {
}

void AGSSock::Socket_SendTo(ScriptMethodParams &params) {
}

void AGSSock::Socket_Recv(ScriptMethodParams &params) {
}

void AGSSock::Socket_RecvFrom(ScriptMethodParams &params) {
}

void AGSSock::Socket_SendData(ScriptMethodParams &params) {
}

void AGSSock::Socket_SendDataTo(ScriptMethodParams &params) {
}

void AGSSock::Socket_RecvData(ScriptMethodParams &params) {
}

void AGSSock::Socket_RecvDataFrom(ScriptMethodParams &params) {
}

void AGSSock::Socket_GetOption(ScriptMethodParams &params) {
}

void AGSSock::Socket_SetOption(ScriptMethodParams &params) {
}

} // namespace AGSSock
} // namespace Plugins
} // namespace AGS3
