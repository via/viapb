all: test

out:
	mkdir out

out/example.descriptor: proto/example.proto out
	python -m grpc_tools.protoc -I proto proto/example.proto -o out/example.descriptor

out/example.pb.h out/example.pb.c: out/example.descriptor proto/example.options
	python ../parse.py --file out/example.descriptor -c out/example.pb.c -H out/example.pb.h --options proto/example.options

test: test.c out/example.pb.c
	cc -O2 -g -o test test.c out/example.pb.c -I .. -I out -fsanitize=undefined

clean:
	-rm out/* test test.out

.PHONE: clean
