from itertools import chain, combinations
hexnums = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"

def powerset(iterable):
    s = list(iterable)
    return chain.from_iterable(combinations(s, r) for r in range(len(s)+1))


def add_new_p(probes_r, r_names, probes_sh, d, new_p):
    r_nb = len(r_names)
    probe_r = vector(GF(2), r_nb)
    probe_sh = matrix(GF(2), d + 1)

    for elem in new_p.split():
        if elem in r_names:
            probe_r[r_names.index(elem)] += 1
        elif elem[0] == 's':
            i,j = hexnums.index(elem[1]), hexnums.index(elem[2])
            if i > d or j > d:
                raise ValueError("Shares identifiers must be consecutive")
            probe_sh[i, j] += 1
        else:
            raise NotImplementedError

    probes_r.append(probe_r)
    probes_sh.append(probe_sh)


def get_r_names(desc):
    r_names = []

    for share in desc:
        share = share.split()
        for elem in share:
            if 'r' == elem[0] and elem not in r_names:
                r_names.append(elem)
            else:
                continue

    return r_names


def get_probes(txtr_desc):
    desc = txt_desc.split("\n")
    if not desc[-1]:
        desc = desc[:-1]


    d = len(desc) - 1
    r_names = get_r_names(desc)
    r_nb = len(r_names)

    probes_r = []
    probes_sh = []
    probes_expl = []

    all_shares = []

    for (nshare, share) in enumerate(desc):
        share = share.split("|")
        for (npart, part) in enumerate(share):
            part = part.split()
            before_part = ' '.join(share[:npart])
            for p in powerset([before_part] + part):
                if not p:
                    continue
                # Remove unwanted spaces
                new_p = ' '.join((' '.join(p).split())) 
                if len(new_p.split(' ')) < 2:
                    continue
                suffix = " - in c{}".format(hexnums[nshare])

                if new_p + suffix in probes_expl:
                    continue

                add_new_p(probes_r, r_names, probes_sh, d, new_p)
                new_p += suffix
                probes_expl.append(new_p)
    
    print(probes_expl)
    for i, p_exp in enumerate(probes_expl):
        print(p_exp)
        print(probes_r[i])
        print
        print(probes_sh[i])

    probes_r = matrix(probes_r).transpose()

    return (d, probes_r, probes_sh, probes_expl)


#if __name__ == "__main__":
#    filename = input("scheme: ")
#    with open(filename, "r") as f:
#        txt_desc = f.read()
#
#    get_probes(txt_desc)
