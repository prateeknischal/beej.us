default:
	g++ -std=c++14 -o identify.out identify.cc
	g++ -std=c++14 -o hostname.out hostname.cc

	./hostname.out
	./identify.out

	rm -rf *.out

	(cd identity_rs && cargo run)
