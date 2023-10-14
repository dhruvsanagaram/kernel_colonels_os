# Notes for ECE391 MP3

## Checkpoint 3.1: Processor Initialization
(6.1.1 is complete as the group repository has obviously been created)
### Loading the GDT
  - File to load from: x86_desc.S contains the empty GDB from line38
  - we need to write some connectivity stuff for having emulated Intel IA-32 processors use the GDT.\
    - General procedure: We will use the LGDT, LIDT commands
      (see [IA-32 manual reference](https://courses.engr.illinois.edu/ece391/fa2023/secure/references/IA32-ref-manual-vol-3.pdf))
    - `lgdt LBL` &larr; this just loads the GDT base address   (contained in the LBL) directly into the GDTR register
    - `lidt LBL` &larr; this will load the IDT base address directly into the IDTR register
## Checkpoint 3.2

## Checkpoint 3.3

## Checkpoint 3.4

## Checkpoint 3.5
