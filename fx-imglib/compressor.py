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