default:
	g++ -std=c++14 getaddrinfo.cc -o getaddrinfo.out
	./getaddrinfo.out www.example.com google.com amazonaws.com
	@rm *.out
	(cd showip \
		&& cargo build --release \
		&& target/release/showip www.example.com google.com amazonaws.com)
