all: out/EveryType.pb.o

out:
	mkdir out

out/EveryType.descriptor: proto/EveryType.proto out
	python -m grpc_tools.protoc -I proto proto/EveryType.proto -o out/EveryType.descriptor

out/EveryType.pb.h out/EveryType.pb.c: out/EveryType.descriptor proto/EveryType.options
	python ../parse.py --file out/EveryType.descriptor -c out/EveryType.pb.c -H out/EveryType.pb.h --options proto/EveryType.options

out/EveryType.pb.o: out/EveryType.pb.h out/EveryType.pb.c
	cc -Wall -O -g -c -o out/EveryType.pb.o out/EveryType.pb.c -I out/ -I ../

clean:
	-rm out/*

.PHONE: clean
