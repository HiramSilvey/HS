from google.protobuf import text_format
from pathlib import Path
import profiles_pb2

out_path = Path('profiles/serialized')
in_files = Path('profiles/text').rglob('*.textpb')
for f_in_name in in_files:
    f_in = open(f_in_name, 'rb')
    buf = f_in.read()
    f_in.close()

    profile = profiles_pb2.Profile()
    text_format.Parse(buf, profile)

    f_out_name = out_path.joinpath(f_in_name.stem)
    f_out = open(f_out_name, 'wb')
    f_out.write(profile.SerializeToString())
    f_out.close()
