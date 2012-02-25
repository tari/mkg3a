from __future__ import print_function
from PIL import Image
import lzf

def loadPlanes(file):
    im = Image.open(file)
    pixels = im.getdata()
    if im.mode == "RGBA":
        unpack = lambda r,g,b,a: (r,g,b)
    elif im.mode == "RGB":
        unpack = lambda r,g,b: (r,g,b)
    else:
        print("Unsupported image mode: " + im.mode)
        return
        
    width, height = im.size
    r, g, b = ([], [], [])
    for px in pixels:
        rpx, gpx, bpx = unpack(*px)
        r.append(rpx)
        g.append(gpx)
        b.append(bpx)

    print("Loaded 3 channels RGB: {0} bytes".format(len(r) + len(g) + len(b)))
    r, g, b = map(lzf.compress, (r,g,b))
    return (r,g,b)
    
if __name__ == "__main__":
    import sys, operator, time, os
    for f in sys.argv[1:]:
        print("{0}: {1} bytes in, ".format(f, os.path.getsize(f)), end='')
        start = time.time()
        r, g, b = loadPlanes(f)
        print("{0} bytes out in {1} seconds".format(
            reduce(operator.add, map(len, (r, g, b))), time.time() - start))