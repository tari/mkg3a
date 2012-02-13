#!/usr/bin/env python
# coding=utf-8

# Three kinds of streams:
#  000LLLLL                     Literal string of L+1 bytes
#  LLLaaaaa bbbbbbbb            Backref of L+3 bytes
#  111aaaaa LLLLLLLL bbbbbbbb   Backref of L+9 bytes
# The distance to the beginning of a backref is aaaaabbbbbbbb + 1 bytes.
# eg, 0x03 0x00 0x01 0x02 0x03 0x00 0x03
#  decompresses to
# 0 1 2 3 0 1 2, and
# 0x00 0x00 0xE0 0x3C 0x00
#  decompresses to
# [0] * 70

def compress_lzf(data):
    backrefs = []
    in_idx = 0
    
    # Build backref list
    while in_idx < len(data):
        print("Analyzing at index {0} of {1}".format(in_idx, len(data)))
        br = findBackref(data, in_idx)
        if br is not None:
            length, distance = br
            print("Backref of {0} bytes beginning at {1} - {2}".format(length, in_idx, distance))
            assert distance > 0        # Sanity check
            assert length <= 1 << 13
            in_idx += length
            backrefs.append(br)
        else:
            backrefs.append(data[in_idx])
            in_idx += 1
    print backrefs
        
    # Encode the output
    literals = 0    # Length of current literal string
    out = bytearray()
    for x in backrefs:
        if isinstance(x, tuple):    # Backref
            literals = 0
            l, d = x
            d -= 1          # Distance cannot be 0 (aaabbbb + 1)
            if l > (7+3):   # L+9 form
                out.append(0xE0 | (d >> 8))
                out.append(l - 9)
            else:           # L+3 form
                out.append(((l - 3) << 5) | (d >> 8))
            out.append(d & 0xff)
        else:                       # Literal
            if literals == 0:   # New string
                out.append(0)
            else:               # Increment length of previous
                out[-literals - 1] += 1
            out.append(x)
            # Increment length
            literals += 1
            if literals >= (1 << 5): # Reset run if hit limit (5 bits + 1)
                literals = 0
    return out
            
def findBackref(data, src_idx):
    """
    Searches ``data`` beginning at ``src_idx`` for backreferences.
    Returns a tuple (length, distance) of the best backreference to use, or
    a literal value if there are no worthwhile backreferences.
    """
    BACKREF_LIMIT = 1 << 13
    search_idx = max(0, src_idx - BACKREF_LIMIT)
    
    candidates = []
    #print("\tfindBackref range {0} - {1}".format(search_idx, src_idx))
    for i in range(search_idx, src_idx):
        distance = src_idx - i
        l = 0
        try:
            #print("\tdata[{0}] ({1}) == data[{2}] ({3})".format(i+l, data[i+l], src_idx+l, data[src_idx]))
            while data[i + l] == data[src_idx + l] and l < BACKREF_LIMIT:
                #print("\t\tHIT")
                l += 1
        except IndexError:
            pass    # Hit end of input
        if l >= 3:
            candidates.append((l, distance))
            
    best = (0, 1)
    for l, d in candidates:
        if l > best[0] or \
          (l == best[0] and d < best[1]):   # Prefer shorter offsets
            best = (l,d)
    return best if best[0] > 0 else None