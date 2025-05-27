#pragma once
// 为什么extern ：解决重定义
extern int clientEndPoint();
extern int serverEndPoint();
extern int createTcpSocket();
extern int createAcceptorSocket();
extern int bindAcceptorSocket();
extern int connectToEnd();
extern int dnsConnectToEnd();
extern int acceptNewConnection();
extern void useConstBuffer();
extern void useBufferStr();
extern void useBufferArray();
extern void writeToSocket();
