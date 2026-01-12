all: test_EveryType py/EveryType_pb2.py

validate: all
	python validate.py

out:
	mkdir out

py:
	mkdir py


out/EveryType.descriptor: proto/EveryType.proto out
	python -m grpc_tools.protoc -I proto proto/EveryType.proto -o out/EveryType.descriptor

out/EveryType.pb.h out/EveryType.pb.c: out/EveryType.descriptor proto/EveryType.options
	python ../generate.py --file out/EveryType.descriptor -c out/EveryType.pb.c -H out/EveryType.pb.h --options proto/EveryType.options

test_EveryType: test_EveryType.c out/EveryType.pb.h out/EveryType.pb.c
	cc -fsanitize=undefined -Wall -O -g -o test_EveryType test_EveryType.c out/EveryType.pb.c -I out/ -I ../

py/EveryType_pb2.py: proto/EveryType.proto py
	python -m grpc_tools.protoc -I proto proto/EveryType.proto --python_out py

clean:
	-rm -r py out *.out *.in test_EveryType

.PHONE: clean
