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
    r, g, b = (bytearray(),) * 3
    for px in pixels:
        rpx, gpx, bpx = unpack(*px)
        r.append(rpx)
        g.append(gpx)
        b.append(bpx)

    print("Loaded 3 channels RGB: {0} bytes".format(len(r) + len(g) + len(b)))
    return (r,g,b)
    
if __name__ == "__main__":
    import sys, operator, time
    f = sys.argv[1]
    r, g, b = loadPlanes(f)
    start = time.time()
    rc, gc, bc = map(lzf.compress, (r,g,b))
    size = reduce(operator.add, map(len, (rc,gc,bc)))
    print("{0} bytes out in {1} seconds".format(size, time.time() - start))
