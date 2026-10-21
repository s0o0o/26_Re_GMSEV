#include "game_session.hpp"
#include <iostream>

void GAMESESSION::registerRecv() {
	DWORD recv_flag = 0;
	memset(&_recvOver._over, 0, sizeof(_recvOver._over));
	_recvOver._wasBuf.len = BUF_SIZE - _prevRemainData;
	_recvOver._wasBuf.buf = _recvOver._sendBuf + _prevRemainData;
	WSARecv(_socket, &_recvOver._wasBuf, 1, 0, &recv_flag, &_recvOver._over, 0);
}

void GAMESESSION::registerSend(void* packet) {
	OVER_EXP* sessionData = new OVER_EXP{ reinterpret_cast<char*>(packet)};
	const int result = WSASend(_socket, &sessionData->_wasBuf, 1, 0, 0, &sessionData->_over, 0);
	if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
		std::cerr << "[NET] Send submission failed session=" << object._id
			<< " error=" << WSAGetLastError() << std::endl;
		// Immediate failure produces no completion to release this allocation.
		delete sessionData;
	}
}
