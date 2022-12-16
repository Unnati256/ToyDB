## Problem Statement ##
Use PF layer to simulate the performance of disk-to-disk backup where the
disks are within the same controller having a cache of M pages. Assume
the database to be on N1 disk spindles in RAID 01, and the backup to be
on another set of disks but in RAID 0. Do as much sequential
reading/writing as possible. Use as much parallelism in IO across the
spindles as possible. </br>

To run testpf: </br>
rm file1 file2 </br>
gcc testpf.c buf.c hash.c pf.c </br>
./a.out
