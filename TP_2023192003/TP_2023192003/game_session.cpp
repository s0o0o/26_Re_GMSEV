#include "game_session.hpp"

void GAMESESSION::registerRecv() {
	DWORD recv_flag = 0;
	memset(&_recvOver._over, 0, sizeof(_recvOver._over));
	_recvOver._wasBuf.len = BUF_SIZE - _prevRemainData;
	_recvOver._wasBuf.buf = _recvOver._sendBuf + _prevRemainData;
	WSARecv(_socket, &_recvOver._wasBuf, 1, 0, &recv_flag, &_recvOver._over, 0);
}

void GAMESESSION::registerSend(void* packet) {
	OVER_EXP* sessionData = new OVER_EXP{ reinterpret_cast<char*>(packet)};
	WSASend(_socket, &sessionData->_wasBuf, 1, 0, 0, &sessionData->_over, 0);
}
