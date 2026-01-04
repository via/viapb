import argparse
from dataclasses import dataclass
import os
import sys
from typing import List, Optional
from textwrap import indent

from google.protobuf import descriptor_pb2

def varint_size(value: int) -> int:
    '''Returns the maximum number of bytes a varint can take when encoded.'''
    if value < 0:
        value = 2**64 - value
    for i in range(1, 11):
        if (value >> (i * 7)) == 0:
            return i
    raise ValueError("Value too large for varint: " + str(value))

def field_type_name_to_cname(name: str) -> str:
    if not name.startswith("."):
        raise Exception("Non-absolute typenames not implemented yet: %s" % (name))
    return cname([name.removeprefix(".")])

def cname(names: List[str]) -> str:
    names = [n for n in names if n]
    result = "_".join(names).replace(".", "_")
    return result

class Options:
    def __init__(self, options_filename, max_size=None, max_count=None, default_submsg_store=None):
        self.default_max_size = max_size
        self.default_max_count = max_count
        self.default_submsg_store = default_submsg_store
        with open(options_filename, "r") as f:
            self.options = Options.parse_options_file(f.read())

    def max_size(self, path: str) -> int:
        if path in self.options:
            if "max_size" in self.options[path]:
                return self.options[path]["max_size"]

        if self.default_max_size is not None:
            return self.default_max_size

        raise Exception("Requested max_size for %s but not present in options" % (path))

    def max_count(self, path: str) -> int:
        if path in self.options:
            if "max_count" in self.options[path]:
                return self.options[path]["max_count"]

        if self.default_max_count is not None:
            return self.default_max_count

        raise Exception("Requested max_count for %s but not present in options" % (path))

    def submsg_store(self, path: str) -> Optional[int]:
        if path in self.options:
            if "submsg_store" in self.options[path]:
                return self.options[path]["submsg_store"]

        return self.default_submsg_store
        

    @staticmethod
    def parse_options_file(options_txt: str) -> dict[str, dict[str, int]]:
        all_options : dict[str, dict[str, int]] = {}
        for line in options_txt.split('\n'):
            try:
                parts = line.split()
                path = parts[0]
                if path not in all_options:
                    all_options[path] = {}
                for option in parts[1:]:
                    (k, v) = option.split(':')
                    all_options[path][k] = int(v)
            except:
                continue

        return all_options

class KnownTypeStore:
    def __init__(self):
        self.known_types : dict[str, PpbEnum | PpbMessage] = {}

    def add(self, path, t: PpbEnum | PpbMessage):
        self.known_types[path] = t

    def get(self, path) -> PpbEnum | PpbMessage:
        if path not in self.known_types:
            raise Exception("Referenced %s but not a known type" % (path))
        return self.known_types[path]


@dataclass
class PpbFile:
    filename: str
    package: str
    enums: List[PpbEnum]
    messages: List[PpbMessage]

    def __init__(self, d: descriptor_pb2.FileDescriptorProto,
                       known_types: KnownTypeStore,
                       options: Options):

        self.filename = d.name
        self.package = d.package
        self.enums = []
        self.messages = []
        path = "."
        if self.package:
            path += self.package + "."

        for e in d.enum_type:
            self.enums.append(PpbEnum(e, path, known_types))

        for m in d.message_type:
            self.messages.append(PpbMessage(m, path, known_types, options))

@dataclass
class PpbEnum:
    name: str
    path: str
    values: List[tuple[str, int]]
    max_size: int

    def __init__(self, d: descriptor_pb2.EnumDescriptorProto,
                       path: str,
                       known_types: KnownTypeStore):
        self.name = d.name
        self.values = []
        self.path = path + d.name
        for pv in d.value:
            self.values.append((pv.name, pv.number))

        self.max_size = self._compute_max_size()
        known_types.add(self.path, self)


    def _compute_max_size(self):
        minval = min([x[1] for x in self.values])
        maxval = min([x[1] for x in self.values])
        if minval < 0:
            return varint_size(minval)
        else:
            return varint_size(maxval)

@dataclass
class FieldType:
    pbtype: str
    wtype: str
    max_encoded_size: int
    is_packable: bool
    is_fixed_size: bool

Type = descriptor_pb2.FieldDescriptorProto.Type
basetypes = {                     #PB Type     #  Wire type  Size     Pack   FixedSize
    Type.TYPE_BOOL:     FieldType('BOOL',     'PB_WT_VARINT',   1,    True,  True),
    Type.TYPE_INT32:    FieldType('INT32',    'PB_WT_VARINT',  10,    True,  False),
    Type.TYPE_SINT32:   FieldType('SINT32',   'PB_WT_VARINT',  10,    True,  False),
    Type.TYPE_UINT32:   FieldType('UINT32',   'PB_WT_VARINT',   5,    True,  False),
    Type.TYPE_INT64:    FieldType('INT64',    'PB_WT_VARINT',  10,    True,  False),
    Type.TYPE_SINT64:   FieldType('SINT64',   'PB_WT_VARINT',  10,    True,  False),
    Type.TYPE_UINT64:   FieldType('UINT64',   'PB_WT_VARINT',   5,    True,  False),
    Type.TYPE_FIXED32:  FieldType('FIXED32',  'PB_WT_32BIT',    4,    True,  True),
    Type.TYPE_SFIXED32: FieldType('SFIXED32', 'PB_WT_32BIT',    4,    True,  True),
    Type.TYPE_FIXED64:  FieldType('FIXED64',  'PB_WT_64BIT',    8,    True,  True),
    Type.TYPE_SFIXED64: FieldType('SFIXED64', 'PB_WT_64BIT',    8,    True,  True),
    Type.TYPE_FLOAT:    FieldType('FLOAT',    'PB_WT_32BIT',    4,    True,  True),
    Type.TYPE_DOUBLE:   FieldType('DOUBLE',   'PB_WT_64BIT',    8,    True,  True),
}


@dataclass
class PpbField:
    name: str
    path: str
    fieldtype: FieldType
    type_name: str
    is_repeated: bool
    explicit_optional: bool
    tag: int
    oneof: Optional[str]
    max_count: Optional[int]
    max_size: int

    @staticmethod
    def make_string_field_type(size: int) -> FieldType:
        return FieldType(pbtype='STRING',
                         wtype='PB_WT_STRING',
                         max_encoded_size=size,
                         is_packable=False,
                         is_fixed_size=False,
                         )

    @staticmethod
    def make_bytes_field_type(size: int) -> FieldType:
        return FieldType(pbtype='BYTES',
                         wtype='PB_WT_STRING',
                         max_encoded_size=size,
                         is_packable=False,
                         is_fixed_size=False,
                        )



    @staticmethod
    def make_enum_field_type(size: int) -> FieldType:
        return FieldType(pbtype='ENUM',
                         wtype='PB_WT_VARINT',
                         max_encoded_size=size,
                         is_packable=True,
                         is_fixed_size=False,
                         )

    @staticmethod
    def make_message_field_type(size: int) -> FieldType:
        return FieldType(pbtype='MESSAGE',
                         wtype='PB_WT_STRING',
                         max_encoded_size=size,
                         is_packable=False,
                         is_fixed_size=False,
                         )

    @staticmethod
    def _fieldtype(desc: descriptor_pb2.FieldDescriptorProto, path: str, known_types: KnownTypeStore, options: Options) -> FieldType:
        if desc.type == Type.TYPE_STRING:
            size = options.max_size(path)
            return PpbField.make_string_field_type(size)
        if desc.type == Type.TYPE_MESSAGE:
            msgtype = known_types.get(desc.type_name)
            return PpbField.make_message_field_type(msgtype.max_size)
        if desc.type == Type.TYPE_BYTES:
            size = options.max_size(path)
            return PpbField.make_bytes_field_type(size)
        if desc.type == Type.TYPE_ENUM:
            msgtype = known_types.get(desc.type_name)
            return PpbField.make_enum_field_type(msgtype.max_size)
        if desc.type in basetypes:
            return basetypes[desc.type]
        raise Exception("Unrecognized PB type: %s" % (desc.type))

    def __init__(self, d: descriptor_pb2.FieldDescriptorProto,
                       path: str,
                       oneofs: List[str],
                       known_types: KnownTypeStore,
                       options: Options):

        self.name = d.name
        self.path = path + d.name
        self.fieldtype = PpbField._fieldtype(d, self.path, known_types, options)
        self.type_name = d.type_name
        self.tag = d.number
        self.explicit_optional = d.proto3_optional
        self.oneof = oneofs[d.oneof_index] if d.HasField('oneof_index') and not d.proto3_optional else None
        self.is_repeated = (d.label == descriptor_pb2.FieldDescriptorProto.LABEL_REPEATED)
        self.max_count = options.max_count(self.path) if self.is_repeated else None
        self.max_size = self._compute_max_size()


    def _compute_max_size(self) -> int:
        element_size = self.fieldtype.max_encoded_size
        tag_size = varint_size(self.tag << 3)

        if self.is_repeated:
            if self.fieldtype.pbtype in ["STRING", "BYTES", "MESSAGE"]:
                # Not packed, just N * (TAG+LEN+PAYLOAD)
                return ((self.max_count or 1) *
                        (tag_size + varint_size(element_size) + element_size))
            else:
                # packed, just TAG+PACKEDLEN+(N*PAYLOAD)
                packed_size = element_size * (self.max_count or 1)
                return tag_size + varint_size(packed_size) + packed_size
        else:
            size = element_size
            size += tag_size
            if self.fieldtype.pbtype in ["STRING", "BYTES", "MESSAGE"]:
                # Add a length
                size += varint_size(element_size)
            return size

@dataclass
class PpbMessage:
    name: str
    path: str
    enums: List[PpbEnum]
    messages: List[PpbMessage]
    fields: List[PpbField]
    oneofs: set[str]
    max_size: int
    submsg_store: Optional[int]

    def __init__(self, d: descriptor_pb2.DescriptorProto,
                       path: str,
                       known_types: KnownTypeStore,
                       options: Options):
        self.name = d.name
        self.enums = []
        self.messages = []
        self.fields = []
        self.path = path + self.name
        self.submsg_store = options.submsg_store(self.path)

        oneofs = []
        for o in d.oneof_decl:
            oneofs.append(o.name)
        for e in d.enum_type:
            self.enums.append(PpbEnum(e, self.path + ".", known_types))
        for m in d.nested_type:
            self.messages.append(PpbMessage(m, self.path + ".", known_types, options))
        for f in d.field:
            self.fields.append(PpbField(f, self.path + ".", oneofs, known_types, options ))

        # filter the oneof list actually used by non-proto3_optional fields, as
        # these are dummy ones we don't actually want to represent
        self.oneofs = set([f.oneof for f in self.fields if f.oneof is not None])
        self.max_size = self._compute_max_size()

        known_types.add(self.path, self)


    def _compute_max_size(self) -> int:

        size = 0
        for f in self.fields:
            if f.oneof:
                continue
            size += f.max_size

        for o in self.oneofs:
            sizes = [f.max_size for f in self.fields if f.oneof == o]
            size += max(sizes)

        return size


@dataclass
class CType:
    ctype: str
    encode_fn: Optional[str] = None
    decode_fn: Optional[str] = None
    
basectypes = {         #C Type     Encode Fn            Decode Fn
    'BOOL':     CType('bool',     'pb_encode_varint',   'pb_decode_bool'),
    'INT32':    CType('int32_t',  'pb_encode_varint',   'pb_decode_varint_int32'),
    'SINT32':   CType('int32_t',  'pb_encode_svarint',  'pb_decode_svarint_int32'),
    'UINT32':   CType('uint32_t', 'pb_encode_varint',   'pb_decode_varint_uint32'),
    'INT64':    CType('int64_t',  'pb_encode_varint',   'pb_decode_varint_int64'),
    'SINT64':   CType('int64_t',  'pb_encode_svarint',  'pb_decode_svarint_int64'),
    'UINT64':   CType('uint64_t', 'pb_encode_varint',   'pb_decode_varint_uint64'),
    'FIXED32':  CType('uint32_t', 'pb_encode_fixed32',  'pb_decode_fixed32'),
    'SFIXED32': CType('int32_t',  'pb_encode_fixed32',  'pb_decode_sfixed32'),
    'FIXED64':  CType('uint64_t', 'pb_encode_fixed64',  'pb_decode_fixed64'),
    'SFIXED64': CType('int64_t',  'pb_encode_fixed64',  'pb_decode_sfixed64'),
    'FLOAT':    CType('float',    'pb_encode_float',    'pb_decode_float'),
    'DOUBLE':   CType('double',   'pb_encode_double',   'pb_decode_double'),
}


def get_ctype(field: PpbField) -> CType:
    if field.fieldtype.pbtype == "STRING":
        inline_struct = "struct { char str[%d]; unsigned len; }" % (field.fieldtype.max_encoded_size)
        return CType(ctype=inline_struct)
    if field.fieldtype.pbtype == "BYTES":
        inline_struct = "struct { uint8_t bytes[%d]; unsigned len; }" % (field.fieldtype.max_encoded_size)
        return CType(ctype=inline_struct)
    if field.fieldtype.pbtype in ["ENUM", "UENUM"]:
        name = field_type_name_to_cname(field.type_name)
        return CType(ctype=name, encode_fn="pb_encode_varint",
                     decode_fn="pb_decode_enum_%s" % (name))
    if field.fieldtype.pbtype == "MESSAGE":
        name = field_type_name_to_cname(field.type_name)
        return CType(ctype="struct %s" % (name))
    if field.fieldtype.pbtype in basectypes:
        return basectypes[field.fieldtype.pbtype]

    raise Exception("Unrecognized C conversion for field %s", field.name)


def render_c_enum(e: PpbEnum) -> str:
    result = ""
    result += "typedef enum {\n"
    name = field_type_name_to_cname(e.path)
    for k, v in e.values:
      result += "  %s_%s = %d,\n" % (name, k, v)
    result += "} %s;\n\n" % name
    result += "\n"
    return result

def generate_c_enum_decode_fn(e: PpbEnum) -> str:
    name = field_type_name_to_cname(e.path)
    result = "static bool pb_decode_enum_%s(%s *value, pb_read_fn r, void *user) {\n" % (name, name)
    if min(x[1] for x in e.values) < 0:
        result += "  int32_t temp;\n"
        result += "  if (!pb_decode_varint_int32(&temp, r, user)) { return false; }\n"
    else:
        result += "  uint32_t temp;\n"
        result += "  if (!pb_decode_varint_uint32(&temp, r, user)) { return false; }\n"
    result += "  *value = temp;\n"
    result += "  return true;\n"
    result += "}\n"
    result += "\n"
    return result

def render_c_message_def(message: PpbMessage) -> str:
    result = ""

    struct_name = field_type_name_to_cname(message.path)

    if len(message.enums) > 0:
        result += "// Nested enums for %s\n\n" % (message.name)
        for e in message.enums:
            result += render_c_enum(e)

    if len(message.messages) > 0:
        result += "// Nested messages for %s\n\n" % (message.name)
        for m in message.messages:
            result += render_c_message_def(m)

    result += "struct %s {\n" % (struct_name)
    for f in message.fields:
        if f.oneof:
            continue # We'll handle these after

        fieldname = cname([f.name])
        ctype = get_ctype(f).ctype

        arraydecl = "[%d]" % (f.max_count or 1) if f.is_repeated else ""
        if f.is_repeated:
            result += "  unsigned %s_count;\n" % (fieldname)
        elif f.explicit_optional or f.fieldtype.pbtype == "MESSAGE":
            result += "  bool has_%s;\n" % (fieldname)
        result += "  %s %s%s;\n" % (ctype, fieldname, arraydecl)
        result += "\n"

    for o in message.oneofs:
        union_cname = cname([o])
        result += "  unsigned which_%s;\n" % (union_cname)
        result += "  union {\n"
        for f in message.fields:
            name = cname([f.name])
            ctype = get_ctype(f).ctype
            if f.oneof != o:
                continue
            result += "    %s %s;\n" % (ctype, name)

        result += "  } %s;\n\n" % (union_cname)

    result += "};\n\n"

    result += "bool pb_encode_%s(const struct %s *msg, pb_write_fn w, void *user);\n" % (struct_name, struct_name)
    result += "unsigned pb_encode_%s_to_buffer(uint8_t buffer[%s], const struct %s *msg);\n" % (struct_name, message.max_size, struct_name)
    result += "unsigned pb_sizeof_%s(const struct %s *msg);\n\n" % (struct_name, struct_name)
    result += "bool pb_decode_%s(struct %s *msg, pb_read_fn r, void *user);\n" % (struct_name, struct_name)

    for f in message.fields:
        result += "#define PB_TAG_%s     %dul\n" % (field_type_name_to_cname(f.path), f.tag)
    result += "#define PB_MAX_SIZE_%s    %dul\n" % (field_type_name_to_cname(message.path), message.max_size)
    result += "\n"
    result += "\n"

    return result

def generate_c_header(files: List[PpbFile], filename):
    result = ""

    # Include guard

    guard = "VIAPB_" + filename.replace('/', '_',).replace('\\', '_').replace('-', '_').replace('.', '_').upper()
    result += "#ifndef %s\n" % (guard)
    result += "#define %s\n" % (guard)
    result += "\n"
    result += "#include <stdint.h>\n"
    result += "#include <stdbool.h>\n"
    result += "\n"
    result += "#include \"viapb.h\"\n"
    result += "\n"

    for f in files:
        result += "// Types from %s\n\n" % f.filename

        for e in f.enums:
            result += render_c_enum(e)

        for m in f.messages:
            result += render_c_message_def(m)

    result += "#endif // %s\n\n" % (guard)
    with open(filename, "w") as f:
        return print(result, file=f)

def generate_presence_expression(msgptr: str, f: PpbField) -> str:
    fieldname = cname([f.name])
    if f.is_repeated:
        return "%s->%s_count > 0" % (msgptr, fieldname)
    elif f.oneof:
        tagdef = "PB_TAG_%s" % (field_type_name_to_cname(f.path))
        return "%s->which_%s == %s" % (msgptr, f.oneof, tagdef)
    elif f.explicit_optional or f.fieldtype.pbtype == "MESSAGE":
        return "%s->has_%s" % (msgptr, fieldname)
    elif f.fieldtype.pbtype in ["STRING", "BYTES"]:
        return "%s->%s.len > 0" % (msgptr, fieldname)
    elif f.fieldtype.pbtype == "BOOL":
        return "%s->%s" % (msgptr, fieldname)
    elif f.fieldtype.pbtype in ["INT32", "SINT32", "UINT32", "INT64", "SINT64", "UINT64",
                                "FIXED32", "SFIXED32", "FIXED64", "SFIXED64", "ENUM", "UENUM"]:
        return "%s->%s != 0" % (msgptr, fieldname)
    elif f.fieldtype.pbtype in ["FLOAT"]:
        return "%s->%s != 0.0f" % (msgptr, fieldname)
    elif f.fieldtype.pbtype in ["DOUBLE"]:
        return "%s->%s != 0.0" % (msgptr, fieldname)
    raise Exception("Support for field presence not implemented")

def generate_element_size_expression(f: PpbField, element: str) -> str:
    ctype = get_ctype(f)

    if f.fieldtype.is_fixed_size:
        return "%d" % (f.fieldtype.max_encoded_size)
    if f.fieldtype.pbtype in ["BYTES", "STRING"]:
        return "%s.len" % element
    if f.fieldtype.pbtype in ["MESSAGE"]:
        type_cname = field_type_name_to_cname(f.type_name)
        return "pb_sizeof_%s(&%s)" % (type_cname, element)
    
    # Only thing left are varints
    if f.fieldtype.pbtype in ["SINT32", "SINT64"]:
        return "pb_sizeof_svarint(%s)" % (element)
    else:
        return "pb_sizeof_varint(%s)" % (element)

def generate_field_size_statements(msgptr: str, accumulator: str, f: PpbField) -> str:
    result = ""
    fieldname = cname([f.name])
    if f.is_repeated and f.fieldtype.pbtype not in ["MESSAGE", "BYTES", "STRING"]:
        # All repeated scalar types are packed
        if f.fieldtype.is_fixed_size:
            result += "    unsigned packed_size = %s->%s_count * %s;\n" % (msgptr, fieldname, f.fieldtype.max_encoded_size)
        else:
            # We have to accumulate the size of all the individual elements
            result += "    unsigned packed_size = 0;\n"
            result += "    for (unsigned i = 0; i < %s->%s_count; i++) {\n" % (msgptr, fieldname)
            element = "%s->%s[i]" % (msgptr, fieldname)
            result += "      packed_size += %s;\n" % generate_element_size_expression(f, element)
            result += "    }\n"
        result += "    %s += %d;  // Size of tag\n" % (accumulator, varint_size(f.tag << 3))
        result += "    %s += pb_sizeof_varint(packed_size);\n" % (accumulator)
        result += "    %s += packed_size;\n" % (accumulator)

    elif f.is_repeated:
        # non-packed non-scalar messages
        element = "%s->%s[i]" % (msgptr, fieldname)
        result += "    for (unsigned i = 0; i < %s->%s_count; i++) {\n" % (msgptr, fieldname)
        result += "      %s += %d;  // Size of tag\n" % (accumulator, varint_size(f.tag << 3))
        result += "      unsigned element_size = %s;\n" % (generate_element_size_expression(f, element))
        result += "      %s += pb_sizeof_varint(element_size);\n" % (accumulator)
        result += "      %s += element_size;\n" % (accumulator)
        result += "    }\n"
    else:
        oneof_part = "%s." % (f.oneof) if f.oneof else ""
        element = "%s->%s%s" % (msgptr, oneof_part, fieldname)
        result += "    %s += %d;  // Size of tag\n" % (accumulator, varint_size(f.tag << 3))
        result += "    unsigned element_size = %s;\n" % (generate_element_size_expression(f, element))
        if f.fieldtype.pbtype in ["BYTES", "STRING", "MESSAGE"]:
            result += "    %s += pb_sizeof_varint(element_size);\n" % (accumulator)
        result += "    %s += element_size;\n" % (accumulator)



    return result
    

def generate_sizeof_msg_fn(m: PpbMessage) -> str:
    result = ""

    for nested_msg in m.messages:
        result += generate_sizeof_msg_fn(nested_msg)

    struct_name = field_type_name_to_cname(m.path)
    result += "unsigned pb_sizeof_%s(const struct %s *msg) {\n" % (struct_name, struct_name)
    result += "  unsigned size = 0;\n"
    for f in m.fields: 
        result += "  if (%s) {\n" % (generate_presence_expression("msg", f))
        result += generate_field_size_statements("msg", "size", f)
        result += "  }\n"
        result += "\n"

    result += "\n"
    result += "  return size;\n"
    result += "}\n"

    return result

def generate_field_encode_statement(f: PpbField, direct : bool = False, submsg_store_size: Optional[int] = None) -> str:
    """Generate a set of statements to encode the field, including handling
    of repeated elements and use of a scratch buffer. These fields are
    expected in the function context:
        - `msg` is a pointer to the message structure
    If direct is set, ptr is a uint8_t* to a buffer sufficient to hold the entire field. Otherwise:
        - `scratch` is a uint8_t[20]
        - `w` is the writer function
        - `user` is the write function user pointer

    If submsg_size is not None and large enough to hold a message field, the 
    uint8_t[] `submsg` is used to write out the submessage to avoid having to
    separately calculate the size.
    """

    ct = get_ctype(f)
    result = ""
    oneof = "%s." % (f.oneof) if f.oneof else ""
    fieldname = oneof + f.name

    use_submsg_store = not direct and \
                       f.fieldtype.pbtype == "MESSAGE" and \
                       submsg_store_size is not None and \
                       submsg_store_size >= f.fieldtype.max_encoded_size

    if f.fieldtype.pbtype not in ["MESSAGE", "BYTES", "STRING"]:
        if not direct:
            result += "uint8_t *ptr = scratch;\n"

        if f.is_repeated: # Primitive packed types
            result += "ptr += pb_encode_varint(ptr, (%s << 3) | PB_WT_STRING);\n" % (f.tag)

            if f.fieldtype.is_fixed_size:
                result += "unsigned sz = %s * msg->%s_count;\n" % (f.fieldtype.max_encoded_size, f.name)
            else:
                result += "unsigned sz = 0;\n"
                result += "for (unsigned idx = 0; idx < msg->%s_count; idx++) {\n" % (f.name)
                element = "msg->%s[idx]" % (f.name)
                result += "  sz += %s;\n" % (generate_element_size_expression(f, element))
                result += "}\n"

            result += "ptr += pb_encode_varint(ptr, sz);\n"
            if not direct:
                result += "if (!w(scratch, ptr - scratch, user)) { return false; }\n"

            result += "for (unsigned idx = 0; idx < msg->%s_count; idx++) {\n" % (f.name)
            if direct:
                result += "  ptr += %s(ptr, msg->%s[idx]);\n" % (ct.encode_fn, f.name)

            else:
                result += "  unsigned len = %s(scratch, msg->%s[idx]);\n" % (ct.encode_fn, f.name)
                result += "  if (!w(scratch, len, user)) { return false; }\n"
            result += "}\n"

        else: # Single primitive type
            result += "ptr += pb_encode_varint(ptr, (%s << 3) | %s);\n" % (f.tag, f.fieldtype.wtype)
            result += "ptr += %s(ptr, msg->%s);\n" % (ct.encode_fn, fieldname)
            if not direct:
                result += "if (!w(scratch, ptr - scratch, user)) { return false; }\n"

    else: # MESSAGE, STRING, or BYTES 

        # Each message is individually encoded even if its repeated, so determine the 
        # right statements and hold onto them

        elem = "msg->%s[idx]" % (f.name) if f.is_repeated else "msg->%s" % (fieldname)
        encode_stmts = "ptr += pb_encode_varint(ptr, (%s << 3) | PB_WT_STRING);\n" % (f.tag)
        if use_submsg_store:
            # encode the message to submsg_store
            encode_stmts += "unsigned elem_size = pb_encode_%s_to_buffer(submsg, &%s);\n" % (field_type_name_to_cname(f.type_name), elem)
        else: 
            encode_stmts += "unsigned elem_size = %s;\n" % (generate_element_size_expression(f, elem))
        encode_stmts += "ptr += pb_encode_varint(ptr, elem_size);\n"
        if direct:
            if f.fieldtype.pbtype == "MESSAGE":
                encode_stmts += "ptr += pb_encode_%s_to_buffer(ptr, &%s);\n" % (field_type_name_to_cname(f.type_name), elem)
            elif f.fieldtype.pbtype == "STRING":
                encode_stmts += "memcpy(ptr, %s.str, %s.len);\n" % (elem, elem)
                encode_stmts += "ptr += %s.len;\n" % (elem)
            elif f.fieldtype.pbtype == "BYTES":
                encode_stmts += "memcpy(ptr, %s.bytes, %s.len);\n" % (elem, elem)
                encode_stmts += "ptr += %s.len;\n" % (elem)

        else:
            encode_stmts += "if (!w(scratch, ptr - scratch, user)) { return false; }\n"
            if f.fieldtype.pbtype == "MESSAGE":
                if use_submsg_store:
                    encode_stmts += "if (!w(submsg, elem_size, user)) { return false; }\n"
                else:
                    encode_stmts += "if (!pb_encode_%s(&%s, w, user)) { return false; }\n" % (field_type_name_to_cname(f.type_name), elem)
            elif f.fieldtype.pbtype == "STRING":
                encode_stmts += "if (!w((const uint8_t *)%s.str, %s.len, user)) { return false; }\n" % (elem, elem)
            elif f.fieldtype.pbtype == "BYTES":
                encode_stmts += "if (!w(%s.bytes, %s.len, user)) { return false; }\n" % (elem, elem)

        if f.is_repeated:
            result += "for (unsigned idx = 0; idx < msg->%s_count; idx++) {\n" % (f.name)
            if not direct:
                result += "  uint8_t *ptr = scratch;\n"
            result += indent(encode_stmts, "  ")
            result += "}\n"
        else:
            if not direct:
                result += "uint8_t *ptr = scratch;\n"
            result += encode_stmts


    return result


def generate_encode_stream_fn(m: PpbMessage) -> str:
    """Create a C function to serialize the message to a given writer"""
    result = ""
    for nested_msg in m.messages:
        result += generate_encode_stream_fn(nested_msg)

    struct_name = field_type_name_to_cname(m.path)
    result += "bool pb_encode_%s(const struct %s *msg, pb_write_fn w, void *user) {\n" % (struct_name, struct_name)
    result += "  uint8_t scratch[20];\n"
    if m.submsg_store:
        result += "  uint8_t submsg[%d];\n" % (m.submsg_store)
    for f in m.fields:
        result += "  if (%s) {\n" % (generate_presence_expression("msg", f))
        result += indent(generate_field_encode_statement(f, direct=False, submsg_store_size=m.submsg_store), "    ")
        result += "  }\n"
        result += "\n"
    result += "  return true;\n"
    result += "}\n"

    return result

def generate_encode_buffer_fn(m: PpbMessage) -> str:
    """Create a C function to serialize the message to a given buffer"""
    result = ""
    for nested_msg in m.messages:
        result += generate_encode_buffer_fn(nested_msg)

    struct_name = field_type_name_to_cname(m.path)
    result += "unsigned pb_encode_%s_to_buffer(uint8_t buffer[%s], const struct %s *msg) {\n" % (struct_name, m.max_size, struct_name)
    result += "  uint8_t *ptr = buffer;\n"
    for f in m.fields:
        result += "  if (%s) {\n" % (generate_presence_expression("msg", f))
        result += indent(generate_field_encode_statement(f, direct=True), "    ")
        result += "  }\n"
        result += "\n"
    result += "  return (ptr - buffer);\n"
    result += "}\n"

    return result

def generate_field_decode_statements(f: PpbField) -> str:
    result = ""
    ct = get_ctype(f)
    fieldname = "msg->"
    if f.oneof:
        fieldname += f.oneof + "."
    fieldname += f.name
    if f.is_repeated or f.fieldtype.pbtype in ["STRING", "MESSAGE", "BYTES"]:
        # Length prefix
        result += "if (prefix == ((%sul << 3) | PB_WT_STRING)) {\n" % (f.tag)
        result += "  uint32_t length;\n"
        result += "  if (!pb_decode_varint_uint32(&length, r, user)) { return false; }\n"

        if f.is_repeated:
            fieldname += "[%s_count]" % (fieldname)

            if f.fieldtype.pbtype in ["STRING", "MESSAGE", "BYTES"]:
                result += "  if (length > %d) { return false; }\n" % (f.fieldtype.max_encoded_size)


        if f.fieldtype.pbtype == "STRING":
            result += "  if (!r((uint8_t *)%s.str, length, user)) { return false; }\n" % (fieldname)
            result += "  %s.len = length;\n" % (fieldname)
        elif f.fieldtype.pbtype == "BYTES":
            result += "  if (length > %d) { return false; }\n" % (f.fieldtype.max_encoded_size)
            result += "  if (!r(%s.bytes, length, user)) { return false; }\n" % (fieldname)
            result += "  %s.len = length;\n" % (fieldname)
        elif f.fieldtype.pbtype == "MESSAGE":
            result += "  struct pb_bounded_reader br = { .r = r, .user = user, .len = length };\n"
            result += "  if (!pb_decode_%s(&%s, pb_bounded_read, &br)) { return false; }\n" % (field_type_name_to_cname(f.type_name), fieldname)
            if not f.is_repeated and not f.oneof:
                result += "  msg->has_%s = true;\n" % (f.name)
        else:
            # Packed list of scalars
            result += "  struct pb_bounded_reader br = { .r = r, .user = user, .len = length };\n"
            result += "  while (br.len  > 0) {\n"
            result += "    if (msg->%s_count >= %d) { return false; }\n" % (f.name, f.max_count or 0)
            result += "    if (!%s(&%s, pb_bounded_read, &br)) { return false; }\n" % (ct.decode_fn, fieldname)
            result += "    msg->%s_count++;\n" % (f.name)
            result += "  }\n"    
            
        if f.is_repeated and f.fieldtype.pbtype in ["STRING", "MESSAGE", "BYTES"]:
            result += "  msg->%s_count++;\n" % (f.name)

        if f.oneof:
            result += "  msg->which_%s = %d;\n" % (f.oneof, f.tag)


        result += "}\n"

    if f.fieldtype.pbtype not in ["STRING", "MESSAGE", "BYTES"]:
        # Scalar or unpacked repeated primitive types
        result += "if (prefix == ((%sul << 3) | %s)) {\n" % (f.tag, f.fieldtype.wtype)
        if f.is_repeated:
            result += "  if (msg->%s_count >= %d) { return false; }\n" % (f.name, f.max_count or 0)
        result += "  if (!%s(&%s, r, user)) { return false; }\n" % (ct.decode_fn, fieldname)

        if f.oneof:
            result += "  msg->which_%s = %d;\n" % (f.oneof, f.tag)

        if f.is_repeated:
            result += "  msg->%s_count++;\n" % (f.name)

        result += "}\n"





    return result

def generate_decode_stream_fn(m: PpbMessage) -> str:
    """ Create a C function to deserialize a message from a reader"""

    result = ""

    for nested_enum in m.enums:
        result += generate_c_enum_decode_fn(nested_enum)
    for nested_msg in m.messages:
        result += generate_decode_stream_fn(nested_msg)

    struct_name = field_type_name_to_cname(m.path)
    result += "bool pb_decode_%s(struct %s *msg, pb_read_fn r, void *user) {\n" % (struct_name, struct_name)
    result += "  while (true) {\n"
    result += "    uint32_t prefix;\n"
    result += "    if (!pb_decode_varint_uint32(&prefix, r, user)) { break; }\n"
    for f in m.fields:
        result += indent(generate_field_decode_statements(f), "    ")
    result += "  }\n"
    result += "  return true;\n"
    result += "}\n"

    return result

def generate_c_source(files: List[PpbFile], source_filename: str, header_filename: str):
    includepath = os.path.basename(header_filename)
    result  = "#pragma GCC diagnostic ignored \"-Wunused-parameter\"\n"
    result += "#pragma GCC diagnostic ignored \"-Wunused-function\"\n"
    result += "#pragma GCC diagnostic ignored \"-Wunused-variable\"\n"
    result += '\n'
    result += "#include \"viapb-private.h\"\n"
    result += "#include \"%s\"\n" % (includepath)
    result += '\n'
    for f in files:
        result += "// Types from %s\n\n" % (f.filename)
        for enum in f.enums:
            result += generate_c_enum_decode_fn(enum)
        for msg in f.messages:
            result += generate_sizeof_msg_fn(msg)
            result += generate_encode_stream_fn(msg)
            result += generate_encode_buffer_fn(msg)
            result += generate_decode_stream_fn(msg)

    with open(source_filename, "w") as f:
        print(result, file=f)

def main():
    parser = argparse.ArgumentParser(prog="viapb",
                                     description="Generate C headers and source from compiled protobuf descriptors")
    parser.add_argument("--options", action="store", default="viapb.options", help="Path to options file for setting field sizes/lengths")
    parser.add_argument("-f", "--file", action="store", required=True, help="Path to a compiled FileDescriptorSet for the protocol files to generate from")
    parser.add_argument("-c", "--output-source", action="store", default="viapb-generated.c", help="path to output generated C source file")
    parser.add_argument("-H", "--output-header", action="store", default="viapb-generated.h", help="path to output generated C header file")
    parser.add_argument("--default-max-size", action="store", type=int, help="default max size for string/bytes fields when otherwise not specified")
    parser.add_argument("--default-max-count", action="store", type=int, help="default max count of repeated fields when otherwise unspecified")
    parser.add_argument("--default-submsg-store", action="store", type=int, help="default max count of repeated fields when otherwise unspecified")
    args = parser.parse_args()

    options = Options(args.options, args.default_max_size, args.default_max_count, args.default_submsg_store)
    files = descriptor_pb2.FileDescriptorSet.FromString(open(args.file, "rb").read()).file

    parsed_files : List[PpbFile] = []
    known_types = KnownTypeStore()
    for f in files:
        parsed_files.append(PpbFile(f, known_types, options))

    for f in parsed_files:
        for m in f.messages:
            print("%s: %s (%d bytes)" % (f.filename, m.path, m.max_size))

    print("Generating %s" % (args.output_header))
    generate_c_header(parsed_files, filename=args.output_header)

    print("Generating %s..." % (args.output_source))
    generate_c_source(parsed_files, source_filename=args.output_source, header_filename=args.output_header)

if __name__ == "__main__":
    main()
