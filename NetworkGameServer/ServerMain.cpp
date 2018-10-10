#include "Define.h"

#define SERVERPORT 9000
#define BUFSIZE    512

// 최대 접속가능 클라이언트 수
const int MaxClientNum = 3;
// 접속한 클라이언트 수
int ClientNum = 0;
// 클라이언트에 할당할 번호
int ClientCnt = 1;
// 게임 시작 이벤트
HANDLE GameStart;
// 플레이어 대기 이벤트
HANDLE AcceptMorePlayer;
// 임계 영역 선언
CRITICAL_SECTION cs;
// 클라이언트로부터 받을 패킷데이터
CFromClientData ClientPacket;
// 서버가 보낼 패킷데이터
CFromServerData ServerPacket;
//서버에서 클라이언트로 보낼 패킷데이터
ServerPacketInfo	ServerPlayerInfo[3]; // 플레이어 정보 변수
// 클라이언트로 보낼 스타트 패킷.
StartPacket StartInfo;
// 블럭 데이터
CBlockData BlockInfo;
// 아이템 생성확률
int ItemPercent = 0;
int ExitClientNum = 0;
bool IsGameOver = false;

SOCKET client_sock[MaxClientNum];
SOCKADDR_IN clientaddr[MaxClientNum];
int addrlen[MaxClientNum];

// 서버가 저장하고 있는 플레이어들 정보
ServerPacketInfo PlayerInfo[3];

// 소켓 함수 오류 출력 후 종료
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{

	// 시간 관련 변수
	DWORD prevTime = 0;
	float elapsedTimePlus = 0.f;
	float CreateBlockTime = 0.f;

	printf("서버 : %d \n", GetCurrentThreadId());
	int retval;

	// 플레이어 구분 번호
	int PlayerID = 0;
	// 데이터 타입 정의해주는 패킷.
	DataType dataTypeInfo;

	// 임계 영역 설정 (초기 데이터 갱신)
	EnterCriticalSection(&cs);

	if (ClientCnt == 1) {
		PlayerID = Player1;
		ServerPlayerInfo[PlayerID].Type = OBJ_PLAYER1;

		client_sock[PlayerID] = (SOCKET)arg;
	}
	else if (ClientCnt == 2) {
		PlayerID = Player2;
		ServerPlayerInfo[PlayerID].Type = OBJ_PLAYER2;

		client_sock[PlayerID] = (SOCKET)arg;
	}
	else if (ClientCnt == 3) {
		PlayerID = Player3;
		ServerPlayerInfo[PlayerID].Type = OBJ_PLAYER3;

		client_sock[PlayerID] = (SOCKET)arg;
	}

	ServerPlayerInfo[PlayerID].Pos = Vec2(-50.0f + 50.0f * PlayerID, -50.0f); 	// 1P 왼쪽 2P 가운데 3P 오른쪽
	ServerPlayerInfo[PlayerID].life = 1;
	ServerPacket.PlayerInfo[PlayerID] = ServerPlayerInfo[PlayerID];
	PlayerInfo[PlayerID] = ServerPlayerInfo[PlayerID];

	// start packet 초기화
	StartInfo.playerNum = PlayerID;

	// 데이터를 넘긴다
	dataTypeInfo = DATA_START;
	retval = send(client_sock[PlayerID], (char *)&dataTypeInfo, sizeof(dataTypeInfo), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}

	retval = send(client_sock[PlayerID], (char *)&StartInfo, sizeof(StartInfo), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}
	else {
		++ClientCnt;
	}
	LeaveCriticalSection(&cs);

	if (ClientCnt > MaxClientNum)
		SetEvent(GameStart); // 클라이언트 수가 충족되면 게임 시작알림

	// 게임시작 신호 받을 때까지 기다림
	WaitForSingleObject(GameStart, INFINITE);


	// 게임 시작 신호 클라이언트에 보내주기
	dataTypeInfo = DATA_PLAYER;
	retval = send(client_sock[PlayerID], (char *)&dataTypeInfo, sizeof(dataTypeInfo), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}

	retval = send(client_sock[PlayerID], (char *)&PlayerInfo, sizeof(PlayerInfo) * 3, 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}

	while (1) {
		DWORD curTime = timeGetTime();
		float elapsedTime = curTime - prevTime;
		prevTime = curTime;

		float elapsedTimePerSencond = elapsedTime / 1000.f;
		elapsedTimePlus += elapsedTimePerSencond;
		elapsedTimePlus = 0.f;

		// 데이터 받기
		retval = recvn(client_sock[PlayerID], (char *)&dataTypeInfo, sizeof(dataTypeInfo), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			ExitClientNum++;
			break;
		}

		// 클라의 총알을 받음. (위치와 총알타입)
		if (dataTypeInfo == DATA_BULLET) {

			// 임계 영역 설정 (데이터 갱신)
			EnterCriticalSection(&cs);

			retval = recvn(client_sock[PlayerID], (char *)&ClientPacket, sizeof(ClientPacket), 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				ExitClientNum++;
				break;
			}


			for (int i = 0; i < 3; i++) {
				dataTypeInfo = DATA_BULLET;
				retval = send(client_sock[i], (char *)&dataTypeInfo, sizeof(dataTypeInfo), 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					ExitClientNum++;
					break;
				}

				retval = send(client_sock[i], (char *)&ClientPacket, sizeof(ClientPacket), 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					ExitClientNum++;
					break;
				}
			}

			// 임계영역 해제
			LeaveCriticalSection(&cs);
		}
		else if (dataTypeInfo == DATA_PLAYER) {

			// 임계 영역 설정 (데이터 갱신)
			EnterCriticalSection(&cs);

			retval = recvn(client_sock[PlayerID], (char *)&ClientPacket, sizeof(ClientPacket), 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				ExitClientNum++;
				break;
			}
			// 서버가 갖고 있을 정보 저장
			PlayerInfo[PlayerID].Pos = ClientPacket.PlayerInfo.Pos;
			PlayerInfo[PlayerID].life = ClientPacket.PlayerInfo.life;

			// 서버에서 클라로 넘길 데이터 갱신
			for (int i = 0, num = 0; i < 2; ++i, ++num) {
				if (i == PlayerID)
					++num;
				ServerPacket.PlayerInfo[i].Pos = PlayerInfo[num].Pos;
				ServerPacket.PlayerInfo[i].life = PlayerInfo[num].life;
			}

			// 패킷 데이터를 send함.
			dataTypeInfo = DATA_PLAYER;
			retval = send(client_sock[PlayerID], (char *)&dataTypeInfo, sizeof(dataTypeInfo), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				ExitClientNum++;
				break;
			}

			retval = send(client_sock[PlayerID], (char *)&ServerPacket, sizeof(ServerPacket), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				ExitClientNum++;
				break;
			}

			// 임계영역 해제
			LeaveCriticalSection(&cs);

		}

		else if (dataTypeInfo == DATA_EXIT) {
			EnterCriticalSection(&cs);
				ExitClientNum++;
			LeaveCriticalSection(&cs);
			break;
		}

		CreateBlockTime += elapsedTimePerSencond;
		if (CreateBlockTime > 5.f)
		{
			for (int i = 0; i < BLOCKPERLINE; i++) {
				BlockInfo.Type[i] = (ObjType)(OBJ_BLOCK1 + (rand() % BLOCKTYPENUM));
			ItemPercent = rand() % 10;
			if(0 <= ItemPercent && 8 > ItemPercent)
				BlockInfo.ItemType[i] = (ObjType)(OBJ_ITEM1);
			else if (8 == ItemPercent)
				BlockInfo.ItemType[i] = (ObjType)(OBJ_ITEM1);
			else if (9 == ItemPercent)
				BlockInfo.ItemType[i] = (ObjType)(OBJ_ITEM2);
			}
			
			dataTypeInfo = DATA_BLOCK;
			retval = send(client_sock[PlayerID], (char *)&dataTypeInfo, sizeof(dataTypeInfo), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				ExitClientNum++;
				break;
			}

			retval = send(client_sock[PlayerID], (char *)&BlockInfo, sizeof(BlockInfo), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				ExitClientNum++;
				break;
			}

			CreateBlockTime = 0.f;
		}
	}

	dataTypeInfo = DATA_EXIT;
	for (int i = 0; i < MaxClientNum; ++i)
	{
			if (i == PlayerID)
				continue;
		retval = send(client_sock[i], (char *)&dataTypeInfo, sizeof(dataTypeInfo), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
		}
	}

	if (ExitClientNum >= MaxClientNum)
	{
		for(int i = 0; i < MaxClientNum; ++i)
			closesocket(client_sock[i]);

		SetEvent(AcceptMorePlayer);
		IsGameOver = true;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	// 임계 영역 초기화
	InitializeCriticalSection(&cs);

	int retval;

	srand(time(NULL));

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	HANDLE hThread;
	int addrlen;

	GameStart = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (GameStart == NULL) return 1;
	AcceptMorePlayer = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (AcceptMorePlayer == NULL) return 1;
	

	while (1) {

		WaitForSingleObject(AcceptMorePlayer, INFINITE); // 인원수가 다차면 더 이상 accept() X
		if (IsGameOver)
			break;
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// 스레드 생성
		hThread = CreateThread(NULL, 0, ProcessClient,
			(LPVOID)client_sock, 0, NULL);

		if (hThread == NULL) { closesocket(client_sock); }
		else {
			CloseHandle(hThread);
			++ClientNum;
			if(ClientNum < MaxClientNum)
				SetEvent(AcceptMorePlayer);
		}
	}

	// 이벤트 제거
	CloseHandle(GameStart);
	CloseHandle(AcceptMorePlayer);

	// 임계영역 삭제
	DeleteCriticalSection(&cs);

	// closesocket()
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();

	return 0;
}

