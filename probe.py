#!/usr/bin/env python


def sseq(start, asize):
    seq = [str(start)]
    k = start
    i = k % asize
    last = start
    step = 0
    for _ in range(1000000):
        step = step + 1
        i = (i + step) % asize
        if i == last:
            break
        seq.append(str(i))
    return seq


def doit(start, asize):
    k = start
    step = 0
    for i in range(1000):
        step = step + 1
        r = (k + step) % asize
        print(f"{i:2d}: ({k} + {step:2d}) % {asize} = {r}")
        k = r
        if k == start:
            break


if __name__ == '__main__':
    doit(0, 8)
    # for i in range(2, 14):
    #     asize = 1 << i
    #     seq = sseq(0, asize)
    #     f = 2**(i+1) - 1
    #     print(f"{2**i}: {len(seq)} =?= {f}")

    # print(', '.join(sseq(0, 8)))
