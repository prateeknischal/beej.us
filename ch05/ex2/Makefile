default:
	g++ -std=c++14 -o socketclient client.cc
	./socketclient google.com 80
	@rm socketclient

	(cd client \
		&& cargo build --release \
		&& target/release/client google.com 80)
