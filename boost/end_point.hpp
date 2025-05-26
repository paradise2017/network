#pragma once

extern int clientEndPoint();
extern int serverEndPoint();
extern int createTcpSocket();
extern int createAcceptorSocket();
extern int bindAcceptorSocket();
extern int connectToEnd();
extern int dnsConnectToEnd();
extern int acceptNewConnection();