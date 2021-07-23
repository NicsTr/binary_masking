# Verification tool for binary masking schemes

This repository contains software and additional data related to the paper
_Fast verification of masking schemes in
characteristic two_ by Nicolas Bordes and Pierre Karpman,
available as the [Eprint 2019/1165](https://eprint.iacr.org/2019/1165) tech report.

The tool in the main directory can be used to verify the
schemes found in the [`schemes`](/schemes/) folder or other schemes of the same
format.

## Schemes

Instantiations of the new schemes introduced in Section 5 of our paper can be
found in the [`schemes`](/schemes/) directory for all orders up to 11 for both
NI and SNI schemes. Due to historical reasons, those
schemes are named according to the _number of shares_, which is one more than
their security order.

Glitch-resistant schemes introduced by Gross et al. in their paper (available
at [Eprint 2016/286](https://eprint.iacr.org/2016/486.pdf)) can be found in
[`DOM-indep`](/schemes/DOM-indep).

A _d_-NI scheme can be generated for any _d_ using `gen_sch.py`.

## Probes description generation

Before compiling the verification tool, it is required to generate the
C description of the scheme that will be verified. To do so you can use
`gen_prob_desc.sage`, a [Sage](https://www.sagemath.org/) script.
The output is a pair of files called `prob_desc.c` and `prob_desc.h`
that contains the information needed before compilation.

The script has one dependency: [`ply`](https://pypi.org/project/ply/). It can be
installed using `pip` (`pip install ply`) or your favorite package manager.
It must be accessible by your sage environment (be careful when you sage
install is not system-wide).

**WARNING: The optimization based on equipotent sets given in
`gen_prob_desc.sage` doesn't apply to every schemes. Whether or not it can be
applied to a given scheme can be determined using `check_redundant.sage`**

**WARNING 2: Python2 is
officially no longer supported since 1st January 2020. [Sage](https://www.sagemath.org/)
being based on Python, it needs to run on Python3. Since version 9.0,
[Sage](https://www.sagemath.org/) is running on Python3 by default.**

## Compilation and architectural dependencies

Our implementation benefit from `AVX2` and `AVX512` instructions, but the main
feature (scheme verification in the probing model) can be used without
vectorised instructions. Only the verification of schemes in presence of
hardware glitches is not yet supported without at least `AVX2`.

One can enforce the use of only non-vectorised instructions when generating the
probes description.

We provide a simple `Makefile` that should satisfy most needs. It can be
tweaked in order to reflect the extensions you want to use. By default, the
verification tool is compiled for both `AVX2` and `AVX512`.

## Verification of a scheme

The generated binary `matverif` can be used for both sequential and parallel
verification. By default, it sequentially verifies whether the scheme given as
input in `prob_desc.c` is NI or not. To enable SNI checks, you must use the
option `--sni`. To use parallel verification, you must use the option `-p`
followed by the number of threads.

## Number of sets

The total number of sets to check for a given number of probes and for a given
order `d` can be computed using `compute_nb_sets.py`.
