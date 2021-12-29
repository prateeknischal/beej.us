default:
	g++ -std=c++14 -o socketclient.out client.cc
	./socketclient.out google.com 80
	@rm *.out

	(cd client \
		&& cargo build --release \
		&& target/release/client google.com 80)
