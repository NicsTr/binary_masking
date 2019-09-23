import sys
import itertools

def apply_mask(v, mask):
    """
    Return the result of v & mask
    """
    res = []
    for i in range(len(v)):
        res.append(v[i] & mask[i])
    return res


def is_share_better(x1_s, x2_s):
    """
    Test the inclusion of x1_s into x2_s
    """
    return apply_mask(x1_s, x2_s) == x1_s


def check_redundant(M, Mb, mask_r, mask_s, d):
    """
    For given matrices `M` and `Mb` containing the probe description before
    and after filtering out probes for a single output share, for given masks
    `mask_r` and `mask_s` giving the position of the randoms and shares, and
    for a given order `d`:

    Return False if it contradicts the definition of an equipotent subset ;
    return True otherwise.
    """

    res = []
    for i in range(d):
        res.append(dict())
        size2 = Mb.nrows()
        for j in itertools.combinations(range(size2), i):
            v2 = [0]*size2
            for k in j:
                v2[k] = 1
            v2 = matrix(GF(2), v2)
            x2 = v2*Mb

            x2 = [int(i) for i in x2[0]]
            x2_r = apply_mask(x2, mask_r)
            x2_s = apply_mask(x2, mask_s)
            x2_r_str = ''.join([str(i) for i in x2_r])

            if x2_r_str not in res[i]:
                res[i][x2_r_str] = []
            res[i][x2_r_str].append(x2_s)

    for i in range(d):
        print(i)
        res1 = dict()
        size1 = M.nrows()
        for j in itertools.combinations(range(size1), i):
            v1 = [0]*size1
            for k in j:
                v1[k] = 1
            v1 = matrix(GF(2), v1)
            x1 = v1*M

            x1 = [int(i) for i in x1[0]]
            x1_r = apply_mask(x1, mask_r)
            x1_s = apply_mask(x1, mask_s)
            x1_r_str = ''.join([str(i) for i in x1_r])

            found = False
            for j in range(i, -1, -1):
                for x2_s in res[j][x1_r_str]:
                    if is_share_better(x1_s, x2_s):
                        found = True
                        break
                if found:
                    break

            if not found:
                print("Contradiction found!")
                return False
    return True


if __name__ == "__main__":
    if len(sys.argv) == 2:
        filename = sys.argv[1]
    else:
        filename = raw_input("Filename: ")

    masks_all = []
    matrices_all = []
    with open(filename, 'r') as f:
        line = f.readline()[:-1]
        d = -1
        while line:
            # Constructs needed structure for each output share
            line = line.split(" ")
            d += 1
            l = len(line)
            mask_r = []
            mask_s = []
            for i in line:
                if 'r' in i:
                    mask_r.append(1)
                    mask_s.append(0)
                else:
                    mask_r.append(0)
                    mask_s.append(1)
            masks_all.append((mask_r, mask_s))

            M = matrix.toeplitz([1]*l, [0]*(l-1), GF(2))
            Mb = matrix.toeplitz([1]*l, [0]*(l-1), GF(2))
            to_del = []
            # Filtering out probes
            for i in range(len(line)):
                s1 = line[:i+1]
                if not ''.join(s1).count('s') % 2:
                    to_del.append(i)
                    print(s1)
            Mb = Mb.delete_rows(to_del)
            line = f.readline()[:-1]
            matrices_all.append((M, Mb))

        is_ok = True
        for (M, Mb), (mask_r, mask_s) in zip(matrices_all, masks_all):
            is_ok &= check_redundant(M, Mb, mask_r, mask_s, d)
            if not is_ok:
                break

        if is_ok:
            print("You can delete those redundant probes!")
        else:
            print("/!\\ Something went wrong /!\\")
