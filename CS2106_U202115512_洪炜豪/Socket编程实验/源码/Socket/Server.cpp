#include "pch.h"
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <iostream>
#include <winsock2.h>
#include <thread>
#include <regex>
#include <WS2tcpip.h>

#pragma comment(lib,"Ws2_32.lib")


void serverclient(SOCKET talksocket);
std::string a = "/404.html";
std::string _ipaddress;
int _port;
std::string fileposition;

int main(void){
	
	std::cout << "请输入监听地址\n";
	std::cin >> _ipaddress;
	std::cout << "请输入监听端口\n";
	std::cin >> _port;
	std::cout << "请输入主目录\n";
	std::cin >> fileposition;
	//初始化winsock(网络库)
	WSADATA wasdata;
	int re = WSAStartup(0x0202, &wasdata);//返回值
	if (re){
		std::cout << "WSAstartup error\n";
		return 0;
	}
	std::cout << "WSAstartup success\n";
	if (wasdata.wVersion != 0x0202){
		std::cout << "Version error\n";
		WSACleanup();
		return 0;
	}
	std::cout << "Version pass\n";
	//创建一个监听socket
	SOCKET tcpsocket;//服务器监听socket
	tcpsocket = socket(AF_INET, SOCK_STREAM,0);
	if (tcpsocket == INVALID_SOCKET){
		std::cout << "Get socket error\n";
		return 0;
	}
	std::cout << "get correct socket\n";
	//bind 监听socket

	sockaddr_in addr;//定义一个名为addr的结构体变量，该结构体用于存储套接字的地址信息
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	int result = inet_pton(AF_INET, _ipaddress.c_str(), &(addr.sin_addr));
	if (result <= 0) {
		perror("inet_pton failed");
		exit(EXIT_FAILURE);
	}
	//char ip_str[INET_ADDRSTRLEN];
	//if (inet_ntop(AF_INET, &(addr.sin_addr), ip_str, sizeof(ip_str))) {
	//	printf("IP Address: %s\n", ip_str);
	//}
	//addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //监听本机上的所有可用网络接口（网卡）的IP地址

	if (bind(tcpsocket, (LPSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR) {
		std::cout << "Bind error: " << WSAGetLastError() << "\n";
		closesocket(tcpsocket);
		WSACleanup();
		return 0;
	}

	//监听socket，等待客户机的连接请求
	re = listen(tcpsocket, 100);//最多接收100个连接
	if (re == SOCKET_ERROR){
		std::cout << "listen error\n";
		return 0;
	}
	std::cout << "listen correct\n";
	//accept接收客户机请求 连接并生成会话socket

	while (1)//服务器将一直处于监听状态，接受客户端的连接请求
	{
		int sockerror;//声明一个整数变量sockerror，用于存储套接字错误码
		int length=sizeof(sockaddr);
		sockaddr_in clientaddr;
		SOCKET talksocket = accept(tcpsocket, (sockaddr *)&clientaddr, &length);
		if (talksocket == INVALID_SOCKET){
			std::cout << "accept error\n";
			sockerror = WSAGetLastError();//使用WSAGetLastError函数获取套接字错误码
			return 0;
		}
		else{
			std::cout << "accept success\n";
			std::thread talkthread(serverclient, talksocket);//将每个新连接放入一个单独的线程或进程中进行处理
			std::cout << "request IP:" << (int)clientaddr.sin_addr.S_un.S_un_b.s_b1 << "." << (int)clientaddr.sin_addr.S_un.S_un_b.s_b2 << "." << (int)clientaddr.sin_addr.S_un.S_un_b.s_b3 << "." << (int)clientaddr.sin_addr.S_un.S_un_b.s_b4 << "\r\n";
			std::cout << "request Port:" << clientaddr.sin_port << "\r\n";
			//talkthread.join();//等待新线程结束再执行主线程
			talkthread.detach();//将新线程talkthread分离,使其在完成后自动释放资源,确保在客户端断开连接后线程可以正常退出
		}
	}
	return 0;
}

void serverclient(SOCKET talksocket){//进行会话的函数。
	int sendre;
	char* buf = (char*)malloc(1024 * sizeof(char));
	int len = 1024;
	int recharnum;
	const char* NOTFOUND = "HTTP/1.1 404 Not Found\r\n";
	const char* REQERROR = "HTTP/1.1 400 Bad Request\r\n";
	const char* FORMERROR = "HTTP/1.1 400 Bad Request\r\n";
	const char* LOADERROR = "HTTP/1.1 400 Bad Request\r\n";
	recharnum = recv(talksocket, buf, len, 0);
	if (recharnum == SOCKET_ERROR){
		recharnum = WSAGetLastError();
		std::cout << "thread falied!";
		std::cout << std::endl;
		return;
	}
	else{
		//对接受到的消息进行解析
		std::smatch strmatch;//正则表达式结果文本
		std::regex regulation("([A-Za-z]+) +(.*) +(HTTP/[0-9][.][0-9])");//正则表达式规则，匹配请求报文的请求行
		std::string str(buf);//需要用正则表达式的原始文本

		int matchnum = std::regex_search(str, strmatch, regulation);
		//在字符串 str 中查找与正则表达式 regulation 匹配的子串。如果找到匹配项，将捕获的子串存储在 strmatch 中，并返回匹配到的次数
		if (matchnum == 0){
			std::cout << "request message exception\n";
			sendre = send(talksocket, REQERROR, strlen(REQERROR),0);
			closesocket(talksocket);
			return;
		}
		else{//分离 GET  url http_version
			std::cout << strmatch[1] << " " << strmatch[2] << " " << strmatch[3] << "\r\n";
			std::string msg_get = strmatch[1];
			std::string msg_url = strmatch[2];
			std::smatch filetype;
			std::regex regulation2("\\..*");
			matchnum = regex_search(msg_url, filetype, regulation2);
			if (matchnum == 0){
				std::cout << msg_get + msg_url+"FORMAT ERROR\n";
				sendre=send(talksocket, FORMERROR, strlen(FORMERROR),0);
				closesocket(talksocket);
				return;
			}
			else
			{	

				std::ifstream f(fileposition + msg_url,std::ios::binary);
				if (!f){//没有找到文件
					f.close();
					std::cout << msg_url + "NOT FOUND";
					//sendre=send(talksocket, NOTFOUND, strlen(NOTFOUND),0);
					f.open(fileposition + a, std::ios::binary);
					if (!f.is_open()) {
						std::cerr << "Failed to open 404 file: " << fileposition + a << std::endl;
							closesocket(talksocket);
						return;
					}
						goto sendfile;
					closesocket(talksocket);
					return;
				}
				else{//如果找到了对应的文件
					sendfile:
					std::filebuf* tmp = f.rdbuf();
					int size = tmp->pubseekoff(0, f.end,f.in);
					tmp->pubseekpos(0, f.in);
					if (size <= 0){
						std::cout << "load file into memory failed!\n";
						sendre=send(talksocket, LOADERROR, strlen(LOADERROR), 0);
						closesocket(talksocket);
						return;
					}
					else{
						std::string Content_Type="image/jpg";
						char* buffer = new char[size];
						char* tail = buffer + size;
						tmp->sgetn(buffer,size);//从文件流中读取文件内容，并将其存储到缓冲区buffer中
						f.close();
						std::cout << "success return file " + msg_url << "\r\n\r\n";
						std::cout << std::endl;
						std::stringstream remsg;
						remsg << "HTTP/1.1 200 OK\r\n" << "Connection:close\r\n" << "Server:Shiratama\r\n"<<"Content Length:"<<size
							<<"\r\n"<<"Content Type:"+Content_Type<<"\r\n\r\n";
						std::string remsgstr = remsg.str();
						const char* remsgchar = remsgstr.c_str();
						int tmpsize = strlen(remsgchar);
						sendre=send(talksocket, remsgchar, tmpsize, 0);
						while (buffer < tail) {
							sendre = send(talksocket, buffer, size, 0);
							buffer = buffer + sendre;
							size = size - sendre;
						}//确保发送完整的文件内容
						closesocket(talksocket);
						return;
					}
				}
			}
		}
	}
	return;
}