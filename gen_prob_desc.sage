import sys

def pi_sh_to_c(pi_sh):
    """
    Convert the share description of a probe into the corresponding C code.
    (non-vectorized version)
    """
    nb_sh = len(pi_sh)
    code = "{ "
    for shi in pi_sh:
        shi_bin = ''.join([str(s) for s in shi])
        code += "{ "
        for i in range(nb_sh//64 + 1):
            shi_int = int(shi_bin[i*64:(i+1)*64], 2)
            code += "0x{:016X}".format(shi_int)
            code += ", "
        code += "},\n"
    code += " }"
    return code


def pi_sh_to_c_vect(pi_sh):
    """
    Convert the share description of a probe into the corresponding C code.
    (vectorized version)
    """
    code = "{ "
    for shi in pi_sh:
        shi_int = int(''.join([str(s) for s in shi]), 2)
        code += hex(shi_int)
        code += ", "
    code = code[:-2]
    code += " }"
    return code


def probes_sh_to_c_vect(probes_sh):
    """
    Convert the share description of all the probes into the corresponding C
    code. (vectorized version)
    """
    code = []

    # by rows
    nb_probes = len(probes_sh)
    nb_sh = len(probes_sh[0].rows()[0])

    assert nb_sh <= 16

    code.append("uint16_t probes_sh_a[{}][{}]".format(nb_probes, 16))
    arr = ''
    arr += " { "
    for pi_sh in probes_sh:
        arr += pi_sh_to_c_vect(pi_sh.rows())
        arr += ", \n"
    arr = arr[:-3]
    arr += "\n};"
    code.append(arr)

    # by columns
    code.append("uint16_t probes_sh_b[{}][{}]".format(nb_probes, 16))
    arr = ''
    arr += " { "
    for pi_sh in probes_sh:
        arr += pi_sh_to_c_vect(pi_sh.columns())
        arr += ", \n"
    arr = arr[:-3]
    arr += "\n};"
    code.append(arr)

    return code


def probes_sh_to_c(probes_sh):
    """
    Convert the share description of all the probes into the corresponding C
    code. (non-vectorized version)
    """
    code = []

    # by rows
    nb_probes = len(probes_sh)
    nb_sh = len(probes_sh[0].rows()[0])
    size_sh = nb_sh//64 + 1

    code.append("uint64_t probes_sh_a[{}][{}][{}]".format(nb_probes, nb_sh, size_sh))
    arr = ''
    arr += " { "
    for pi_sh in probes_sh:
        arr += pi_sh_to_c(pi_sh.rows())
        arr += ", \n"
    arr = arr[:-3]
    arr += "\n};"
    code.append(arr)

    # by columns
    code.append("uint64_t probes_sh_b[{}][{}][{}]".format(nb_probes, nb_sh, size_sh))
    arr = ''
    arr += " { "
    for pi_sh in probes_sh:
        arr += pi_sh_to_c(pi_sh.columns())
        arr += ", \n"
    arr = arr[:-3]
    arr += "\n};"
    code.append(arr)

    return code


def probes_r_to_c_vect(probes_r):
    """
    Convert the randoms description of all the probes into the corresponding C
    code. (vectorized version)
    """
    assert len(probes_r.columns()[0]) <= 64

    code = []
    code.append("uint64_t probes_r[{}]".format(len(probes_r.columns())))
    arr = ''
    arr += " { "
    for ri in probes_r.columns():
        ri_int = int(''.join([str(r) for r in ri]), 2)
        arr += "{}, ".format(hex(ri_int))
    arr = arr[:-2 ] + " };"
    code.append(arr)
    return code


def probes_r_to_c(probes_r):
    """
    Convert the randoms description of all the probes into the corresponding C
    code. (non-vectorized version)
    """
    nb_r = len(probes_r.columns()[0])
    code = []
    code.append("uint64_t probes_r[{}][{}]".format(len(probes_r.columns()), nb_r//64 + 1))
    arr = ''
    arr += " { \n"
    for ri in probes_r.columns():
        ri_bin = ''.join([str(r) for r in ri])
        arr += "{ "
        for i in range(nb_r//64 + 1):
            ri_int = int(ri_bin[i*64:(i+1)*64], 2)
            arr += "0x{:016X}, ".format(ri_int)

        arr += "},\n"
    arr += " };"
    code.append(arr)
    return code


if __name__ == "__main__":
    # Load the tools
    base_dir = "./private_multiplication/"
    load_attach_path(base_dir)
    load(base_dir + "tools.sage")
    
    if len(sys.argv) == 2:
        filename = sys.argv[1]
    else:
        filename = raw_input("Filename: ")
    
    
    with open(filename, "r") as f:
        txt_desc = f.read()
    
    # Check correctness + generate internal representations
    desc = get_desc(txt_desc)
    print("Correct" if test_correctness(desc) else "ERROR")
    probes_desc = (d, probes_r, probes_sh, probes_expl) = get_probes(desc)
    
    
    # Exclude redundant probes
    pos_to_keep= []
    for i in range(len(probes_expl)):
        s1 = probes_expl[i].split(" - ")[0]
        if s1.count('s') % 2:
            pos_to_keep.append(i)
    
    probes_r = probes_r.matrix_from_columns(pos_to_keep)
    probes_sh = [probes_sh[i] for i in pos_to_keep]
    probes_expl = [probes_expl[i] for i in pos_to_keep]
    
    
    # Exclude probes with only random values
    pos_to_keep = []
    for i in range(len(probes_sh)):
        if probes_sh[i] != 0:
            pos_to_keep.append(i)
    
    probes_r = probes_r.matrix_from_columns(pos_to_keep)
    probes_sh = [probes_sh[i] for i in pos_to_keep]
    probes_expl = [probes_expl[i] for i in pos_to_keep]
    
    
    # Regroup external probes at the end
    to_move = []
    for i in range(d + 1):
        ci = "c{}".format(i)
        m = -1
        for j in range(len(probes_expl)):
            p = probes_expl[j]
            if ci in p and len(p) > m:
                m = len(p)
                index = j
        to_move.append(index)
    
    probes_r = block_matrix([[probes_r.matrix_from_columns([i for i in range(probes_r.ncols()) if (i
        not in to_move)]), probes_r.matrix_from_columns(to_move)]])
    probes_sh = [probes_sh[i] for i in range(len(probes_sh)) if i not in to_move] + [probes_sh[i] for i in to_move]
    
    
    nb_sh = len(probes_sh[0].rows()[0])
    nb_r = len(probes_r.columns()[0])
    vect = False
    if nb_sh <= 16 and nb_r <= 64:
        vect = True
    
    
    # Write output probes description content
    with open("prob_desc.c", "w") as f:
        f.write("#include <stdint.h>\n")
        f.write("\n")
        f.write("/* Probe description for {} */".format(filename))
        f.write("\n\n")
        f.write("char *filename = \"{}\";".format(filename))
        f.write("\n\n")
        if vect:
            f.write(" = ".join(probes_r_to_c_vect(probes_r)))
            f.write("\n\n")
            f.write(" = ".join(probes_sh_to_c_vect(probes_sh)[:2]))
            f.write("\n\n")
            f.write(" = ".join(probes_sh_to_c_vect(probes_sh)[2:]))
        else:
            f.write(" = ".join(probes_r_to_c(probes_r)))
            f.write("\n\n")
            f.write(" = ".join(probes_sh_to_c(probes_sh)[:2]))
            f.write("\n\n")
            f.write(" = ".join(probes_sh_to_c(probes_sh)[2:]))
    
    
    # Write output probes description header
    with open("prob_desc.h", "w") as f:
        f.write("#ifndef PROBES_DESC_H\n")
        f.write("#define PROBES_DESC_H\n\n")
        f.write("#define NB_SH {}\n".format(nb_sh))
        f.write("#define NB_PR {}\n".format(len(probes_sh)))
        f.write("#define NB_R {}\n".format(nb_r))
        f.write("#define D {}\n".format(d))
        if not vect:
            f.write("#define SIZE_SH {}\n".format(nb_sh//64 + 1))
            f.write("#define SIZE_R {}\n".format(nb_r//64 + 1))
        else:
            f.write("#define VECT")
        f.write("\n")
        f.write("/* Probe description for {} */".format(filename))
        f.write("\n\n")
        f.write("char *filename;")
        f.write("\n")
        if vect:
            f.write(probes_r_to_c_vect(probes_r)[0] + ';')
            f.write("\n")
            f.write(probes_sh_to_c_vect(probes_sh)[0] + ';')
            f.write("\n")
            f.write(probes_sh_to_c_vect(probes_sh)[2] + ';')
            f.write("\n\n")
        else:
            f.write(probes_r_to_c(probes_r)[0] + ';')
            f.write("\n")
            f.write(probes_sh_to_c(probes_sh)[0] + ';')
            f.write("\n")
            f.write(probes_sh_to_c(probes_sh)[2] + ';')
            f.write("\n\n")
    
        f.write("#endif /* PROBES_DESC_H */")
