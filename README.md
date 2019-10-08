# Verification tool for binary masking schemes

This repository contains software and additional data related to the paper
_High-order private multiplication in characteristic two revisited_ by Nicolas Bordes and Pierre Karpman,
available as the [Eprint 2019/1165][https://eprint.iacr.org/2019/1165] tech report.

The tool in the main directory can be used to verify the
schemes found in the [`schemes`](/schemes/) folder or other schemes of the same
format.

## Schemes

Instantiations of the new schemes introduced in Section 5 of our paper can be
found in the [`schemes`](/schemes/) directory for all orders up to 11 for NI
schemes and up to 9 for SNI schemes. Due to historical reasons, those
schemes are named according to the _number of shares_, which is one more than
their security order.

A _d_-NI scheme can be generated for any _d_ using `gen_sch.py`.

## Probe description generation

Before compiling the verification tool, it is required to generate the
C description of the scheme that will be verified. To do so you can use
`gen_prob_desc.sage`, a [Sage](https://www.sagemath.org/) script. It depends on
the parser of (Belaïd et al., EUROCRYPT 2016) that can be found in the `private_multiplication`
submodule. The output is a pair of files called `prob_desc.c` and `prob_desc.h`
that contains the information needed before compilation.

**WARNING: The optimization based on equipotent sets given in
`gen_prob_desc.sage` doesn't apply to every schemes. Whether or not it can be
applied to a given scheme can be determined using `check_redundant.sage`**

## Compilation and architectural dependencies

We provide a simple `Makefile` that should satisfy most needs.
Our implementation currently requires at least `AVX2` instructions and
can benefit from `AVX512` extensions. The `Makefile` can be
tweaked in order to reflect your particular architecture. By default, the
verification tool is compiled for both `AVX2` and `AVX512`.

## Verification of a scheme

The generated binary `binverif` can be used for both sequential and parallel
verification. By default, it sequentially verifies whether the scheme given as
input in `prob_desc.c` is NI or not. To enable SNI checks, you must use the
option `--sni`. To use parallel verification, you must use the option `-p`
followed by the number of threads.

## Number of sets

The total number of sets to check for a given number of probes and for a given
order `d` can be computed using `compute_nb_sets.py`.
