default:
	g++ -std=c++14 -o identify identify.cc
	g++ -std=c++14 -o hostname hostname.cc

	./hostname
	./identify

	rm -rf hostname identify

	(cd identity_rs && cargo run)
