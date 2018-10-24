#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>

#include<event2/event.h>
#include<event2/bufferevent.h>
#include<arpa/inet.h>

void read_cb(struct bufferevent *bev,void *arg)
{
	char buf[1024] = {0};
	bufferevent_read(bev,buf,sizeof(buf));
	printf("server say : %s \n", buf);
	 
	//bufferevent_write(bev,buf,strlen(buf) + 1);

	//sleep(1);
}
 
// 写缓冲区回调
void write_cb(struct bufferevent *bev,void *arg)
{
	printf("I，m服务器，成功写数据给客户端 \n ");	
}


void event_cb(struct bufferevent *bev,short events,void*arg)
{
	if(events & BEV_EVENT_EOF)
	{
		printf("connection closed \n");

	}else if(events & BEV_EVENT_ERROR)
	{
		printf("some other error \n");
	}else if(events & BEV_EVENT_CONNECTED)
	{
		printf("connected server succeed //// \n ");
		return;	
	}

	bufferevent_free(bev); 
}


// 客户端和用户交互，从终端进行读取数据给服务器
void read_terminal(evutil_socket_t fd,short what,void *arg)
{
	char buf[1024] = {0};
	int len = read(fd,buf,sizeof(buf));
	
	struct bufferevent *bev = (struct bufferevent*)arg;
	
	bufferevent_write(bev,buf,len+1);
}

//监听回调(客户端建立连接进来)
void cb_listener(
	struct evconnlistener *listener,
	evutil_socket_t fd,
	struct sockaddr *addr,
	int len,
	void *ptr)
{
	printf("connect new client \n");
	
	struct event_base *base = (struct event_base*)ptr;

	// 增加新事件
	struct bufferevent *bev = bufferevent_socket_new(base,fd,BEV_OPT_CLOSE_ON_FREE);
	
	//
	bufferevent_setcb(bev,read_cb,write_cb,event_cb,NULL);
	
	// 启动bufferevent读缓冲，默认是disable的
	bufferevent_enable(bev,EV_READ);
} 

int main(int argc, const char *argv[])
{
	struct event_base *base = event_base_new();

	int fd = socket(AF_INET,SOCK_STREAM,0);
	
	struct bufferevent *bev = bufferevent_socket_new(base,fd,BEV_OPT_CLOSE_ON_FREE);		

	struct sockaddr_in serv;
	memset(&serv, 0 , sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_port   = htons(9991);
	inet_pton(AF_INET,"127.0.0.1", &serv.sin_addr.s_addr);

	//连接服务器
	bufferevent_socket_connect(bev,(struct sockaddr*)&serv, sizeof(serv));
	 
	// 设置回调
	bufferevent_setcb(bev,read_cb,write_cb,event_cb,NULL);

	//监听标准输入终端EV_PERSIST
	struct event *ev = event_new(base,STDIN_FILENO,EV_READ|EV_PERSIST,
				read_terminal,bev); 
	
	event_add(ev,NULL);
	
	event_base_dispatch(base);
 
	event_base_free(base);

	return 0;
}




