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

def compress(data):
    # Build backref list
    backrefs = encodeBackrefs(data)
    #print backrefs
        
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
            
def encodeBackrefs(data):
    BACKREF_LIMIT = 1 << 15

    triples = {}
    i = 0
    out = []
    while i < len(data):
        # Try hash
        try:
            print("Try-hash i = {0}".format(i))
            key = (data[i] << 16) | (data[i+1] << 8) | data[i+2]
            if key in triples and i - triples[key] < BACKREF_LIMIT:
                if i == 2132:
                    import pdb; pdb.set_trace()
                # Valid backref, find its length
                begin = triples[key]
                print("Found triple match index {0} -> {1}".format(begin, i))
                l = 0
                try:
                    while l < 255 + 9 and data[begin + l] == data[i + l]:
                        l += 1
                except IndexError:
                    pass    # Hit end of input, backref terminates
                distance = i - begin
                assert distance > 0 and l < BACKREF_LIMIT
                print("Backref: {0} bytes -{1}".format(l, distance))
                out.append((l, distance))
            else:
                # Backref too far back, just emit a literal
                l = 1
                out.append(data[i])
            # Update triples, then advance input pointer for consumed backref
            triples[key] = i
            i += l - 1
        except IndexError:
            # Fewer than 3 bytes left, backref useless
            out.append(data[i])
        i += 1
    return out