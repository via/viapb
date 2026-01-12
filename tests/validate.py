import py.EveryType_pb2
import unittest
import subprocess

INT32_MIN = -2147483647 - 1
INT64_MIN = -9223372036854775807 - 1
INT32_MAX = 2147483647
INT64_MAX = 9223372036854775807
UINT32_MAX = 4294967295
UINT64_MAX = 18446744073709551615


class EveryType_tests(unittest.TestCase):

    def setUp(self):
        self.testcase1 = py.EveryType_pb2.AllTypes()
        self.testcase1.one_int32 = 1
        self.testcase1.one_uint32 = 2
        self.testcase1.one_sint32 = 3
        self.testcase1.one_int64 = 4
        self.testcase1.one_uint64 = 5
        self.testcase1.one_sint64 = 6
        self.testcase1.one_fixed32 = 7
        self.testcase1.one_fixed64 = 8
        self.testcase1.one_bool = True
        self.testcase1.one_str = "First String"
        self.testcase1.one_bytes = b"First Bytes"
        self.testcase1.one_float = 9.0
        self.testcase1.one_double = 10.0
        self.testcase1.one_sfixed32 = 11
        self.testcase1.one_sfixed64 = 12

        self.testcase1.many_int32.extend([100, 101, 102, 103])
        self.testcase1.many_uint32.extend([104, 105, 106, 107])
        self.testcase1.many_sint32.extend([108, 109, 110, 111])
        self.testcase1.many_int64.extend([112, 113, 114, 115])
        self.testcase1.many_uint64.extend([116, 117, 118, 119])
        self.testcase1.many_sint64.extend([120, 121, 122, 123])
        self.testcase1.many_fixed32.extend([124, 125, 126, 127])
        self.testcase1.many_fixed64.extend([128, 129, 130, 131])
        self.testcase1.many_bool.extend([True, False, True, False])
        self.testcase1.many_str.extend(["String 0", "String 1", "String 2", "String 3"])
        self.testcase1.many_bytes.extend([b"Bytes 0", b"Bytes 1", b"Bytes 2", b"Bytes 3"])
        self.testcase1.many_float.extend([132.0, 133.0, 134.0, 135.0])
        self.testcase1.many_double.extend([136.0, 137.0, 138.0, 139.0])
        self.testcase1.many_sfixed32.extend([140, 141, 142, 143])
        self.testcase1.many_sfixed64.extend([144, 145, 146, 147])
        self.testcase1.selected_str = "OneOf"

        self.testcase1.one_msg.some_int = 1
        self.testcase1.one_msg.many_ints.nested_int.extend([10, 11, INT32_MIN, 13])
        self.testcase1.one_msg.e_val = py.EveryType_pb2.ParentMessage.NestedEnum.More
        self.testcase1.many_msg.add().some_int = 5
        self.testcase1.many_msg.add().some_int = 6
        self.testcase1.many_msg.add().some_int = 7
        self.testcase1.many_msg.add().some_int = 8


        self.testcase1.reg_enum = py.EveryType_pb2.RegularEnum.One
        self.testcase1.larger_enum = py.EveryType_pb2.LargerEnum.TwoFiveSix
        self.testcase1.neg_enum = py.EveryType_pb2.NegativeEnum.Minus14


        self.testcase2 = py.EveryType_pb2.AllTypes()
        self.testcase2.one_int32 = INT32_MIN
        self.testcase2.one_uint32 = UINT32_MAX
        self.testcase2.one_sint32 = INT32_MAX
        self.testcase2.one_int64 = INT64_MIN
        self.testcase2.one_uint64 = UINT64_MAX
        self.testcase2.one_sint64 = INT64_MAX
        self.testcase2.one_fixed32 = UINT32_MAX
        self.testcase2.one_fixed64 = UINT64_MAX
        self.testcase2.one_sfixed32 = INT32_MAX
        self.testcase2.one_sfixed64 = INT64_MAX
        self.testcase2.one_msg.some_int = INT32_MIN
        self.testcase2.larger_enum = py.EveryType_pb2.LargerEnum.SixteenThousand

    def test_all_types_encode_stream_1(self):
        path = "test_%s.out" % (self.id())
        ret = subprocess.run(["./test_EveryType", "-t", "1", "-f", path])
        assert ret.returncode == 0

        x = py.EveryType_pb2.AllTypes()
        with open(path, "rb") as f:
            x.ParseFromString(f.read())

        assert x == self.testcase1

    def test_all_types_encode_buffer_1(self):
        path = "test_%s.out" % (self.id())
        ret = subprocess.run(["./test_EveryType", "-m", "-t", "1", "-f", path])
        assert ret.returncode == 0

        x = py.EveryType_pb2.AllTypes()
        with open(path, "rb") as f:
            x.ParseFromString(f.read())

        assert x == self.testcase1

    def test_all_types_encode_stream_2(self):
        path = "test_%s.out" % (self.id())
        ret = subprocess.run(["./test_EveryType", "-t", "2", "-f", path])
        assert ret.returncode == 0

        x = py.EveryType_pb2.AllTypes()
        with open(path, "rb") as f:
            x.ParseFromString(f.read())

        assert x == self.testcase2

    def test_all_types_encode_buffer_2(self):
        path = "test_%s.out" % (self.id())
        ret = subprocess.run(["./test_EveryType", "-m", "-t", "2", "-f", path])
        assert ret.returncode == 0

        x = py.EveryType_pb2.AllTypes()
        with open(path, "rb") as f:
            x.ParseFromString(f.read())

        assert x == self.testcase2

    def test_all_types_decode_stream_1(self):
        path = "test_%s.in" % (self.id())
        with open(path, "wb") as f:
            f.write(self.testcase1.SerializeToString())

        ret = subprocess.run(["./test_EveryType", "-r", "-t", "1", "-f", path])
        assert ret.returncode == 0

    def test_all_types_decode_stream_2(self):
        path = "test_%s.in" % (self.id())
        with open(path, "wb") as f:
            f.write(self.testcase2.SerializeToString())

        ret = subprocess.run(["./test_EveryType", "-r", "-t", "2", "-f", path])
        assert ret.returncode == 0

    def test_decode_too_many_repeated_int32(self):
        path = "test_%s.in" % (self.id())
        testcase = py.EveryType_pb2.AllTypes()
        testcase.many_int32.extend([5, 6, 7, 8, 9])
        with open(path, "wb") as f:
            f.write(testcase.SerializeToString())

        ret = subprocess.run(["./test_EveryType", "-r", "-f", path])
        # Parsing should fail
        assert ret.returncode != 0

    def test_decode_too_many_repeated_msgs(self):
        path = "test_%s.in" % (self.id())
        testcase = py.EveryType_pb2.AllTypes()
        testcase.many_msg.add().some_int = 5
        testcase.many_msg.add().some_int = 6
        testcase.many_msg.add().some_int = 7
        testcase.many_msg.add().some_int = 8
        testcase.many_msg.add().some_int = 9
        with open(path, "wb") as f:
            f.write(testcase.SerializeToString())

        ret = subprocess.run(["./test_EveryType", "-r", "-f", path])
        # Parsing should fail
        assert ret.returncode != 0

    def test_decode_too_many_repeated_in_submsg(self):
        path = "test_%s.in" % (self.id())
        testcase = py.EveryType_pb2.AllTypes()
        testcase.one_msg.many_ints.nested_int.extend([5, 6, 7, 8, 9])
        with open(path, "wb") as f:
            f.write(testcase.SerializeToString())

        ret = subprocess.run(["./test_EveryType", "-r", "-f", path])
        # Parsing should fail
        assert ret.returncode != 0

if __name__ == "__main__":
    unittest.main()
