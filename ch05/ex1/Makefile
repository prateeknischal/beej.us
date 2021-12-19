default:
	g++ -std=c++14 getaddrinfo.cc -o getaddrinfo
	./getaddrinfo www.example.com google.com amazonaws.com
	@rm getaddrinfo
	(cd showip \
		&& cargo build --release \
		&& target/release/showip www.example.com google.com amazonaws.com)
