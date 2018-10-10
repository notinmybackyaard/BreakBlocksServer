#include "Define.h"

#define SERVERPORT 9000
#define BUFSIZE    512

// �ִ� ���Ӱ��� Ŭ���̾�Ʈ ��
const int MaxClientNum = 3;
// ������ Ŭ���̾�Ʈ ��
int ClientNum = 0;
// Ŭ���̾�Ʈ�� �Ҵ��� ��ȣ
int ClientCnt = 1;
// ���� ���� �̺�Ʈ
HANDLE GameStart;
// �÷��̾� ��� �̺�Ʈ
HANDLE AcceptMorePlayer;
// �Ӱ� ���� ����
CRITICAL_SECTION cs;
// Ŭ���̾�Ʈ�κ��� ���� ��Ŷ������
CFromClientData ClientPacket;
// ������ ���� ��Ŷ������
CFromServerData ServerPacket;
//�������� Ŭ���̾�Ʈ�� ���� ��Ŷ������
ServerPacketInfo	ServerPlayerInfo[3]; // �÷��̾� ���� ����
// Ŭ���̾�Ʈ�� ���� ��ŸƮ ��Ŷ.
StartPacket StartInfo;
// �� ������
CBlockData BlockInfo;
// ������ ����Ȯ��
int ItemPercent = 0;
int ExitClientNum = 0;
bool IsGameOver = false;

SOCKET client_sock[MaxClientNum];
SOCKADDR_IN clientaddr[MaxClientNum];
int addrlen[MaxClientNum];

// ������ �����ϰ� �ִ� �÷��̾�� ����
ServerPacketInfo PlayerInfo[3];

// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
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

// Ŭ���̾�Ʈ�� ������ ���
DWORD WINAPI ProcessClient(LPVOID arg)
{

	// �ð� ���� ����
	DWORD prevTime = 0;
	float elapsedTimePlus = 0.f;
	float CreateBlockTime = 0.f;

	printf("���� : %d \n", GetCurrentThreadId());
	int retval;

	// �÷��̾� ���� ��ȣ
	int PlayerID = 0;
	// ������ Ÿ�� �������ִ� ��Ŷ.
	DataType dataTypeInfo;

	// �Ӱ� ���� ���� (�ʱ� ������ ����)
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

	ServerPlayerInfo[PlayerID].Pos = Vec2(-50.0f + 50.0f * PlayerID, -50.0f); 	// 1P ���� 2P ��� 3P ������
	ServerPlayerInfo[PlayerID].life = 1;
	ServerPacket.PlayerInfo[PlayerID] = ServerPlayerInfo[PlayerID];
	PlayerInfo[PlayerID] = ServerPlayerInfo[PlayerID];

	// start packet �ʱ�ȭ
	StartInfo.playerNum = PlayerID;

	// �����͸� �ѱ��
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
		SetEvent(GameStart); // Ŭ���̾�Ʈ ���� �����Ǹ� ���� ���۾˸�

	// ���ӽ��� ��ȣ ���� ������ ��ٸ�
	WaitForSingleObject(GameStart, INFINITE);


	// ���� ���� ��ȣ Ŭ���̾�Ʈ�� �����ֱ�
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

		// ������ �ޱ�
		retval = recvn(client_sock[PlayerID], (char *)&dataTypeInfo, sizeof(dataTypeInfo), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			ExitClientNum++;
			break;
		}

		// Ŭ���� �Ѿ��� ����. (��ġ�� �Ѿ�Ÿ��)
		if (dataTypeInfo == DATA_BULLET) {

			// �Ӱ� ���� ���� (������ ����)
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

			// �Ӱ迵�� ����
			LeaveCriticalSection(&cs);
		}
		else if (dataTypeInfo == DATA_PLAYER) {

			// �Ӱ� ���� ���� (������ ����)
			EnterCriticalSection(&cs);

			retval = recvn(client_sock[PlayerID], (char *)&ClientPacket, sizeof(ClientPacket), 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				ExitClientNum++;
				break;
			}
			// ������ ���� ���� ���� ����
			PlayerInfo[PlayerID].Pos = ClientPacket.PlayerInfo.Pos;
			PlayerInfo[PlayerID].life = ClientPacket.PlayerInfo.life;

			// �������� Ŭ��� �ѱ� ������ ����
			for (int i = 0, num = 0; i < 2; ++i, ++num) {
				if (i == PlayerID)
					++num;
				ServerPacket.PlayerInfo[i].Pos = PlayerInfo[num].Pos;
				ServerPacket.PlayerInfo[i].life = PlayerInfo[num].life;
			}

			// ��Ŷ �����͸� send��.
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

			// �Ӱ迵�� ����
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
	// �Ӱ� ���� �ʱ�ȭ
	InitializeCriticalSection(&cs);

	int retval;

	srand(time(NULL));

	// ���� �ʱ�ȭ
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

	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	HANDLE hThread;
	int addrlen;

	GameStart = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (GameStart == NULL) return 1;
	AcceptMorePlayer = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (AcceptMorePlayer == NULL) return 1;
	

	while (1) {

		WaitForSingleObject(AcceptMorePlayer, INFINITE); // �ο����� ������ �� �̻� accept() X
		if (IsGameOver)
			break;
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// ������ ����
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

	// �̺�Ʈ ����
	CloseHandle(GameStart);
	CloseHandle(AcceptMorePlayer);

	// �Ӱ迵�� ����
	DeleteCriticalSection(&cs);

	// closesocket()
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();

	return 0;
}

