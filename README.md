# Verification tool for binary masking schemes

This is the source code of our tool. This tool can be used to verify the
schemes found in the [`schemes`](/schemes/) folder or other schemes of the same
format.

## Schemes

Instantiations of the new schemes introduced in Section 5 of our paper can be
found in the [`schemes`](/schemes/) directory for all orders up to 10 for NI
schemes and up to 9 for SNI schemes. The d-NI schemes can be generated using
`gen_sch.py`.

## Probes description generation

Before compiling the verification tool, it is required to generate the
C description of the scheme that will be verified. To do so you can use
`gen_prob_desc.sage`, a [Sage](https://www.sagemath.org/) script. It depends on
the parser of _Belaïd et al._ that can be found in the `private_multiplication`
submodule. The output is a pair of files called `prob_desc.c` and `prob_desc.h`
that contains the information needed before compilation.

**WARNING: The optimization based on equipotent sets given in
`gen_prob_desc.sage` doesn't apply to every schemes. Whether or not it can be
applied to a given scheme can be determined using `check_redundant.sage`**

## Compilation and architectural dependencies

Our verification can be compile using the given `Makefile` simply be executing
`make`. To correctly work, our tool must be run using at least `AVX2` (minimal)
or both `AVX2` and `AVX512` (recommended) extensions. The `Makefile` can be
tweaked in order to reflect your particular architecture. By default, the
verification tool will be compiled for both `AVX2` and `AVX512`.

## Verify a scheme

The generated binary `binverif` can be used for both sequential and parallel
verification. By default, it sequentially verify whether the scheme given as
input in `prob_desc.c` is NI or not. To enable SNI checks, you must use the
option `--sni`. To use parallel verification, you must use the option `-p`
followed by the number of additional threads wanted.

## Number of sets

The total number of sets to check for a given number of probes and for a given
order `d` can be computed using `compute_nb_sets.py`.
