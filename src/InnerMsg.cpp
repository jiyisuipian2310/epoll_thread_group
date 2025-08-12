#include "InnerMsg.h"
#include "AppManager.h"
#include "log.h"

void AcceptMsg::ProcessInnerMsg(IInnerMsg* pInnerMsg, const string& strThreadName) {
     AppManager::instance()->process_accept_message(reinterpret_cast<AcceptMsg*>(pInnerMsg), strThreadName);
}

void TimeoutDisconnectMsg::ProcessInnerMsg(IInnerMsg* pInnerMsg, const string& strThreadName) {
     AppManager::instance()->process_timeout_disconnect_message(reinterpret_cast<TimeoutDisconnectMsg*>(pInnerMsg), strThreadName);
}

void TimeoutLogRecordMsg::ProcessInnerMsg(IInnerMsg* pInnerMsg, const string& strThreadName) {
     AppManager::instance()->process_timeout_logrecord_message(reinterpret_cast<TimeoutLogRecordMsg*>(pInnerMsg), strThreadName);
}

void ReleaseResourceMsg::ProcessInnerMsg(IInnerMsg* pInnerMsg, const string& strThreadName) {
     AppManager::instance()->process_release_resource_message(reinterpret_cast<ReleaseResourceMsg*>(pInnerMsg), strThreadName);
}

void ClientMsg::ProcessInnerMsg(IInnerMsg* pInnerMsg, const string& strThreadName) {
     AppManager::instance()->process_client_message(reinterpret_cast<ClientMsg*>(pInnerMsg), strThreadName);
}

void ServerMsg::ProcessInnerMsg(IInnerMsg* pInnerMsg, const string& strThreadName) {
     AppManager::instance()->process_server_message(reinterpret_cast<ServerMsg*>(pInnerMsg), strThreadName);
}