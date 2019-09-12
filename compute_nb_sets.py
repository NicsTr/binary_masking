from math import log
import operator as op
from functools import reduce

def ncr(n, r):
    r = min(r, n-r)
    numer = reduce(op.mul, range(n, n-r, -1), 1)
    denom = reduce(op.mul, range(1, r+1), 1)
    return int(numer / denom)

if __name__ == "__main__":
    nb_pr = int(input("Number of probes: "))
    d = int(input("Order: "))

    n = 0
    for i in range(1, d + 1):
        n += ncr(nb_pr, i)

    print("Number of sets to check: {} ({})".format(n, log(n,2)))

