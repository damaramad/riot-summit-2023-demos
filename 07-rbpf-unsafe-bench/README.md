# `rbpf-unsafe-bench`

## Description

This demonstration illustrates a 30 percent improvement in the
performance of the post-issuance deployed rBPF virtual machine. This
enhancement is achieved by removing the dynamic memory check on bytecode
instructions, while also providing a higher level of security due to the
MPU configuration by Pip-MPU on a microcontroller running `xipfs` as a
module of RIOT over Pip-MPU.
