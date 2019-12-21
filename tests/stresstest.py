#!/usr/bin/env python

import argparse
import random


class Op:
    INSERT = 0
    ERASE  = 1
    FIND   = 2
    MISS   = 3
    SIZE   = 4
    MAX    = 5


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("-n", "--nops", type=int, default=100,
                      help="Number of operations to generate")
    return parser.parse_args()


if __name__ == '__main__':
    args = parse_args()

    max_ = (1 << 31) - 1
    min_ = -(1 << 31)

    d = {}
    for i in range(args.nops):
        op = random.randint(0, Op.MAX - 1)
        if not d and op == Op.FIND:
            op = Op.MISS

        if op == Op.INSERT:
            k = random.randint(min_, max_)
            v = random.randint(min_, max_)
            if k not in d:
                d[k] = v
            print(f"INSERT {k} {v} {d[k]}")
        elif op == Op.ERASE:
            k = random.randint(min_, max_)
            try:
                del d[k]
                r = 1
            except KeyError:
                r = 0
            print(f"ERASE {k} {r}")
        elif op == Op.FIND:
            i = random.randint(0, len(d) - 1)
            for j, k in enumerate(d.keys()):
                if j == i:
                    break
            print(f"FIND {k} {d[k]}")
        elif op == Op.MISS:
            while 1:
                k = random.randint(min_, max_)
                if k not in d:
                    break
            print(f"MISS {k}")
        elif op == Op.SIZE:
            print(f"SIZE {len(d)}")
        else:
            raise ValueError(f"Invalid operation: {op}")
