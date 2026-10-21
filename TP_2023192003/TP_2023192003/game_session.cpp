#include "game_session.hpp"
#include <iostream>

bool GAMESESSION::registerRecv() {
	if (transportFailed || disconnectStarted) return false;
	DWORD recv_flag = 0;
	memset(&_recvOver._over, 0, sizeof(_recvOver._over));
	_recvOver._wasBuf.len = BUF_SIZE - _prevRemainData;
	_recvOver._wasBuf.buf = _recvOver._sendBuf + _prevRemainData;
	const int result = WSARecv(_socket, &_recvOver._wasBuf, 1, 0, &recv_flag, &_recvOver._over, 0);
	if (result != SOCKET_ERROR) return true;
	const int error = WSAGetLastError();
	if (error == WSA_IO_PENDING) return true;
	if (!transportFailed.exchange(true))
		std::cerr << "[NET] Receive submission failed session=" << object._id << " error=" << error << std::endl;
	return false;
}

void GAMESESSION::registerSend(void* packet) {
	if (transportFailed || disconnectStarted) return;
	OVER_EXP* sessionData = new OVER_EXP{ reinterpret_cast<char*>(packet)};
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
