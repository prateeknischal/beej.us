default:
	g++ -std=c++14 -o simple_server.out simple_server.cc
	(cd tokio_server && cargo build --release)

	@echo "Binaries have been built at ./simple_server.out"
	@echo "and tokio_server/target/release/tokio_server"
