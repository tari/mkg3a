#!/usr/bin/env python
# coding=utf-8
from collections import defaultdict
import functools

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
            
def commonLeader(data, i, j):
    """Get the number of common leading elements beginning at data[i] and data[j]."""
    l = 0
    try:
        while l < 255+9 and data[i+l] == data[j+l]:
            l += 1
    except IndexError:
        pass    # terminates
    return l

def encodeBackrefs(data):
    """
    Annotate ``data`` with backrefs.  Backreferences are emitted as tuples of
    (length, distance), while literal bytes are passed through unmodified.
    
    This function has rather large memory requirements, around 12n (where n is
    the number of bytes input).
    """
    BACKREF_LIMIT = 1 << 15
    hash = lambda d,i: (d[i] << 16) | (d[i+1] << 8) | d[i+2]

    # Maps a triple of bytes to indices it was found at
    triples = defaultdict(list)
    i = 0
    out = []
    while i < len(data)-2:
        try:
            key = hash(data, i)
            #print("Try-hash  idx {0}, t = {1}".format(i, key))
            if key in triples:
                #print("\t{0} matches".format(len(triples[key])))
                # Drop any elements too far back to reference
                triples[key] = filter(lambda k: i - k < BACKREF_LIMIT, triples[key])
                #print("\treduced to {0} in-range".format(len(triples[key])))
                if len(triples[key]) == 0:
                    raise IndexError()  # No backrefs possible
                    
                # Find longest matches
                cl = functools.partial(commonLeader, data, i)
                matchlens = map(cl, triples[key])
                length, begin = (0, 0)
                for l, b in zip(matchlens, triples[key]):
                    if l > length or (l == length and b > begin):
                        length, begin = l, b
                #print("Best match len = {0} begins at {1}".format(length, begin))

                distance = i - begin
                assert distance > 0 and length < BACKREF_LIMIT
                #print("Backref: {0} bytes -{1}".format(length, distance))
                out.append((length, distance))
            else:
                # Backref too far back, just emit a literal
                raise IndexError()
        except IndexError:
            length = 1
            out.append(data[i])
        # Scan all consumed data for triples, advance input pointer
        for j in xrange(i, min(i+length, len(data)-2)):
            triples[hash(data, j)].append(j)
        i += length
        
    # Last few bytes useless to backref
    for j in xrange(i, len(data)):
        out.append(data[j])
    return out