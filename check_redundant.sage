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


def number_missing_share(x1_s, x2_s):
    res = 0
    for i1, i2 in zip(x1_s, x2_s):
        if i1 and not i2:
            res += 1
    return res


def xor(x1_r, x2_r):
    return [i1^^i2 for i1,i2 in zip(x1_r, x2_r)]


def hamming_weight(x):
    res = 0
    for i in x:
        if i:
            res += 1
    return res

def share_completion(res, x1_s, x_r_str, i, j, n_r=0):
    for x2_s in res[j][x_r_str]:
        n_s = number_missing_share(x1_s, x2_s)
        # using n elementary probes
        if n_r + n_s + j <= i:
            return True

    return False


def check(M, Mb, mask_r, mask_s, d):
    """
    For given matrices `M` and `Mb` containing the probe description before
    and after filtering out probes for a single output share, for given masks
    `mask_r` and `mask_s` giving the position of the randoms and shares, and
    for a given order `d`:

    Return False if it contradicts the definition of an equipotent subset ;
    return True otherwise.
    """

    res = []
    for i in range(d+1):
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

    for i in range(d+1):
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
                if x1_r_str in res[j]:
                    # Check for already correct randomness
                    found = share_completion(res, x1_s, x1_r_str, i, j)
                    if found:
                        break

                # Tries to fix randomness with elementary probes
                for x2_r_str in res[j]:
                    x2_r = [int(v) for v in x2_r_str]
                    xf_r = xor(x2_r, x1_r)
                    n_r = hamming_weight(xf_r)
                    # number of elementary random probes already too high
                    if n_r + j > i:
                        continue
                    # x2_r corrected with xf_r
                    found = share_completion(res, x1_s, x2_r_str, i, j, n_r)
                    if found:
                        break

                if found:
                    break

            if not found:
                print("Cannot remove the probes")
                return False
    return True


def gen_matrices_and_masks(filename):
    masks_all = []
    matrices_all = []
    probes_todel = []
    with open(filename, 'r') as f:
        txt_desc = f.read().split('\n')
        for line in txt_desc:
            if not line or "ORDER" in line or "MASKS" in line:
                continue
            # Constructs needed structure for each output share
            line = line.replace('|', '').split()
            l = len(line)
            mask_r = []
            mask_s = []
            for i in line:
                if 'r' in i:
                    mask_r.append(1)
                    mask_s.append(0)
                elif 's' in i:
                    mask_r.append(0)
                    mask_s.append(1)

            masks_all.append((mask_r, mask_s))

            M = matrix.toeplitz([1]*l, [0]*(l-1), GF(2))
            Mb = matrix.toeplitz([1]*l, [0]*(l-1), GF(2))
            to_del = []
            # Filtering out probes
            for i in range(len(line)):
                s1 = ' '.join(line[:i+1])
                # HERE IS the criterion to filter out probes (see Section 5)
                if not s1.count('s') % 2 and i+1 < len(line) and 's' in line[i + 1]:
                    to_del.append(i)
                    probes_todel.append(s1)

            Mb = Mb.delete_rows(to_del)
            line = f.readline()[:-1]
            matrices_all.append((M, Mb))

    return matrices_all, masks_all, probes_todel 


def check_file(filename):

    matrices_all, masks_all, probes_todel = gen_matrices_and_masks(filename)
    print("Checking that filtering out the following probes won't affect the"
            " correctness of the verification:")

    for p in probes_todel:
        print(p)

    is_ok = True
    for (M, Mb), (mask_r, mask_s) in zip(matrices_all, masks_all):
        print(".")
        d = len(matrices_all) - 1
        is_ok &= check(M, Mb, mask_r, mask_s, d)
        if not is_ok:
            break

    if is_ok:
        print("You can delete those redundant probes!")
    else:
        print("/!\\ Something went wrong /!\\")

    return is_ok

