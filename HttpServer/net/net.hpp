/*
* gliu 2017.8.5
* write_bugs@163.com
* 
*/
#pragma once
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

template<class XSession>

class Net{
public:
	Net()
	{
		listenfd = -1;
		epfd = -1;
	}
	
	/*
	* 建立一个socket并将port绑定到上面，初始化epoll并准备进行事件循环
	*/
	void start(int port)
	{
		struct sockaddr_in serverAddr;
		struct epoll_event ev;
		//socket
		listenfd = socket(AF_INET, SOCK_STREAM, 0);
		if (-1 == listenfd)
		    exit_e("socket() return -1");

		bzero(&serverAddr, sizeof(serverAddr));
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		serverAddr.sin_port = htons(port);

		int ret = setnonblocking(listenfd);
		if (-1 == ret)
		    exit_e("setnonblocking() return -1");
		ret = bind(listenfd, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr));
		if (-1 == ret)
		    exit_e("bind() return -1");
		ret = listen(listenfd, 1024);
		if (-1 == ret)
		    exit_e("listen() return -1");
		//epoll
		epfd = epoll_create(1024);
		if (-1 == epfd)
		    exit_e("epoll_create() return -1");
		ev.data.fd = listenfd;
		//ev.events = EPOLLIN | EPOLLET;
		ev.events = EPOLLIN | EPOLLERR;
		ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
		if (-1 == ret)
		    exit_e("epoll_ctl() return -1");
		
	}
	
	/*
	* epoll事件循环，处理各种发生的epoll事件
	*/
	void run()
	{
		struct epoll_event events[MAX_EVENT_NUM];
		struct epoll_event ev;

		for (;;)
		{
		    int nfds = epoll_wait(epfd, events, MAX_EVENT_NUM, -1);
		    if (-1 == nfds)
			continue;
		    for (int i = 0; i < nfds; i++)
		    {
				if (events[i].data.fd == listenfd)
				    handle_accept();
				else if (events[i].events & EPOLLIN)
				    handle_read((XSession *)events[i].data.ptr);
				else if (events[i].events & EPOLLOUT)
				    handle_write((XSession *)events[i].data.ptr);
				else if (events[i].events & EPOLLERR)
				{
				    XSession *session = (XSession *)events[i].data.ptr;
				    delete session;
				}
				else if (events[i].events & EPOLLHUP)
				{
				    XSession *session = (XSession *)events[i].data.ptr;
				    delete session;
				}
		    }
		}
	}
	
	/*
	* 当handleRead收到一个完整的关于协议的包时会停止接受数据并调用
	* 此函数，用户需重写此函数来实现对于收到的数据包的处理逻辑
	*/
	virtual void recvAClientReq(XSession* session)=0;
	
	/*
	* 用于发送一个响应数据包到网络中，该函数会以函数指针的形式嵌入
	* 上层会话逻辑中并在会话发起响应时被调用
	*/
	static void sendAClientResp(void* session,int _fd,int _epfd)
	{
		struct epoll_event ev;
		ev.data.ptr = session;
		//ev.events = EPOLLOUT | EPOLLET | EPOLLERR | EPOLLHUP;
		ev.events = EPOLLOUT | EPOLLERR;
		printf("--------->start send a session\n");
		if(epoll_ctl(_epfd,EPOLL_CTL_MOD,_fd,&ev) == -1){
			printf("----------->send a session failed\n");
			delete (XSession*)session;
		}
	}
	
private:

	/*
	* 当run事件循环接收到一个新的连接事件时调用此函数处理新连接
	*/
	void handle_accept()
	{
		struct sockaddr_in clientAddr;
		struct epoll_event ev;
		socklen_t socklen = sizeof(struct sockaddr_in);
		int clientfd = accept(listenfd, (struct sockaddr *)&clientAddr, &socklen);
		if (-1 == clientfd)
		    return;
		/*-------------------for debug--------------------*/
		char gustIP[20]={0};
		socklen_t gust_len=sizeof(gustIP);
		inet_ntop(AF_INET,&clientAddr.sin_addr,gustIP,gust_len);
		printf("--->accpet a connection from %s:%d\n",gustIP,ntohs(clientAddr.sin_port));
		/*------------------------------------------------*/
		setnonblocking(clientfd);
		XSession *session = new XSession(sendAClientResp, clientfd,epfd);
		ev.data.ptr = session;
		//ev.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP;
		ev.events = EPOLLIN | EPOLLERR;
		int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
		if (-1 == ret)
		{
		    close(clientfd);
		}
	}

	/*
	* 当某一个连接的数据到来时run函数会调用此函数进行数据接受
	* 本函数通过上层会话协议判断数据是否接受完成，如果数据接收
	* 完成则将此连接移除监听队列，否则继续监听
	*/
	void handle_read(XSession* session)
	{
		int nread = 0;
		/*-------------------for debug--------------------*/
		struct sockaddr_in clientAddr;
		socklen_t socklen = sizeof(struct sockaddr_in);
		getpeername(session->fd,(struct sockaddr*)&clientAddr,&socklen);
		char gustIP[20]={0};
		socklen_t gust_len=sizeof(gustIP);
		inet_ntop(AF_INET,&clientAddr.sin_addr,gustIP,gust_len);
		int port= ntohs(clientAddr.sin_port);
		printf("--->try to get data from %s:%d\n",gustIP,port);
		/*------------------------------------------------*/
		while (true)
		{
		    if(session->getRestReqBufSize() <= 0)
		    	session->expandReqBuf();
		    //read data from net buf
		    nread = read(session->fd, 
		    			 session->getReqBufWritePtr(),
		    			 session->getRestReqBufSize());
		    if (nread < 0)
			{
				//printf("---read data size < 0\n");
				if (EINTR == errno || EWOULDBLOCK == errno || EAGAIN == errno)
				    break;
				else{
				    delete session;
				    return;
				}
		    }
		    else if (nread > 0)
			{
				printf("--read data size is %d\n",nread);
				session->commitReqData(nread);
				if (nread < session->getRestReqBufSize()){
				    break;
				}
				else
				    continue;
		    }
		    else if (nread == 0)
			{
				close(session->fd);
				printf("--->%s:%d close the connection\n",gustIP,port);
				delete session;
				return;
		    }
		}

		if (session->isDataOver())
		{
			printf("data read over\n");
			recvAClientReq(session);
		}else{
			printf("--->data is not over and wait the next event\n");
		}
	}

	/*
	* 在会话请求发送响应数据后run函数会收到缓冲区写的事件
	* 并调用本函数对数据进行发送
	*/
	void handle_write(XSession* session)
	{
		/*-------------------for debug--------------------*/
		struct sockaddr_in clientAddr;
		socklen_t socklen = sizeof(struct sockaddr_in);
		getpeername(session->fd,(struct sockaddr*)&clientAddr,&socklen);
		char gustIP[20]={0};
		socklen_t gust_len=sizeof(gustIP);
		inet_ntop(AF_INET,&clientAddr.sin_addr,gustIP,gust_len);
		int port= ntohs(clientAddr.sin_port);
		/*------------------------------------------------*/
		printf("------>try to send response data\n");
		while(true)
		{
			if(session->getRestRespDataSize() <= 0)
				break;

			//write data to network
			int nwrite = write(session->fd,
							   session->getCouldSentDataPtr(),
							   session->getCouldSentDataSize());
			
			if(nwrite > 0)
			{
				//printf("wirte %d bytes:%s<\n",nwrite,session->getCouldSentDataPtr());
				session->commitSentDataSize(nwrite);
			}
			else if(nwrite < 0)
			{
				if (EINTR == errno || EWOULDBLOCK == errno || EAGAIN == errno){
				    	break;
				}else{
					//printf("--->nwrite < 0\n");
				    delete session;
				    return;
				}
			}else if(nwrite == 0)
			{
				close(session->fd);
				//printf("--->nwrite = 0\n");
				delete session;
				break;
			}
		}

		if(session->getRestRespDataSize() <= 0)
		{
			session->reset();
		    struct epoll_event ev;
		    ev.data.ptr = (void *)session;
		    ev.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP;
		    if (epoll_ctl(epfd, EPOLL_CTL_MOD, session->fd, &ev) == -1){
		    	printf("--->try to listen again error\n");
				delete session;
		   	}
			printf("--->try to listen again\n");
		}
	}
	

	bool setnonblocking(int sockfd)
	{
		if(fcntl(sockfd,F_SETFL,fcntl(sockfd,F_GETFD,0)|O_NONBLOCK) == -1)
		{
			return -1;
		}

		return 0;
	}

	inline void exit_e(const char* error_info)
	{
		perror(error_info);
		exit(1);
	}

private:
	int listenfd;
	int epfd;

private:
	const int MAX_EVENT_NUM = 1024;
};