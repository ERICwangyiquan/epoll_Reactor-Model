epoll_server:epoll_server.cc
	g++ -o $@ $^ -std=c++11
.PHONY:clean
clean:
	rm -f epoll_server