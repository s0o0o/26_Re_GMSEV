#include "game_session.hpp"
#include <iostream>

bool GAMESESSION::registerRecv() {
    std::lock_guard<std::mutex> lock(transportMutex);
    if (transportFailed || disconnectStarted) return false;
    auto owner=weak_from_this().lock();
    if(!owner || _prevRemainData < 0 || _prevRemainData >= BUF_SIZE) {
        transportFailed=true; return false;
    }
    if(recvPending.exchange(true)) return false;
    auto operation=new OVER_EXP;
    operation->owner=std::move(owner);
    operation->_wasBuf.len=BUF_SIZE-_prevRemainData;
    operation->_wasBuf.buf=operation->_sendBuf+_prevRemainData;
    std::memcpy(operation->_sendBuf,_recvOver._sendBuf,_prevRemainData);
    DWORD recv_flag=0;
    const int result=WSARecv(_socket,&operation->_wasBuf,1,0,&recv_flag,&operation->_over,0);
	if (result != SOCKET_ERROR) return true;
	const int error = WSAGetLastError();
	if (error == WSA_IO_PENDING) return true;
    recvPending=false;
    delete operation; // Immediate rejection has no IOCP completion.
	if (!transportFailed.exchange(true))
		std::cerr << "[NET] Receive submission failed session=" << object._id << " error=" << error << std::endl;
	return false;
}

void GAMESESSION::registerSend(void* packet) {
    std::lock_guard<std::mutex> lock(transportMutex);
	if (transportFailed || disconnectStarted) return;
    auto owner=weak_from_this().lock();
    if(!owner) { transportFailed=true; return; }
	OVER_EXP* sessionData = new OVER_EXP{ reinterpret_cast<char*>(packet)};
    sessionData->owner=std::move(owner);
	const int result = WSASend(_socket, &sessionData->_wasBuf, 1, 0, 0, &sessionData->_over, 0);
	const int error = result == SOCKET_ERROR ? WSAGetLastError() : 0;
	if (result == SOCKET_ERROR && error != WSA_IO_PENDING) {
		if (!transportFailed.exchange(true)) {
			std::cerr << "[NET] Send submission failed session=" << object._id
				<< " error=" << error << std::endl;
			// Wake any pending receive so its normal completion path cleans up.
			shutdown(_socket, SD_BOTH);
		}
		// Immediate failure produces no completion to release this allocation.
		delete sessionData;
	}
}

void GAMESESSION::closeTransport() {
    std::lock_guard<std::mutex> lock(transportMutex);
    disconnectStarted=true;
    transportFailed=true;
    if(_socket != INVALID_SOCKET) {
        closesocket(_socket);
        _socket=INVALID_SOCKET;
    }
}
