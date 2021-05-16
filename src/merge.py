from PIL import Image
from io import BytesIO
import sys
import binascii
first_buffer = sys.argv[1]
second_buffer = sys.argv[2]
image1 = Image.open(BytesIO(binascii.unhexlify(first_buffer))).convert('RGBA')
image2 = Image.open(BytesIO(binascii.unhexlify(second_buffer))).convert('RGBA')
composite = Image.alpha_composite(image1, image2)
print(binascii.hexlify(composite.tobytes()))
