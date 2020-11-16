![Founsure](https://github.com/suaybarslan/founsure/blob/master/Images/Founsure10.png)
# Founsure 1.0
<ins> An Erasure Code Library with Efficient Repair and Update Features: Version 1.0 </ins>

# Overview and Objective
**Founsure 1.0** is the first version of a totally new erasure coding library using Intel's SIMD and multi-core architectures based on simple aligned XOR operations and fountain coding. The details of fountain coding is beyond the scope of this text. I encourage you to take a look at my [introductory paper](https://arxiv.org/pdf/1402.6016.pdf). Founsure does not use specific SIMD instructions, it rather pays attention to memory alignment and leaves the assembly instructions to be optimized by the "magical" gcc/g++ compiler (with appropriate optimization flags). This library is expected to have comparable performance to other open source alternatives and yet shall provide distinguishably better repair and rebuilt performance which seem to be the main stream focus of erasure coding community nowadays in a distributed setting. The version 1.0 has three major functions "founsureEnc", "founsureDec" and "founsureRep" for encoding, decoding and repair/update respectively. 

Here are the reasons why "Founsure" is developped and exists:
- [x] Founsure is based on fountain codes which are not space-optimal i.e., non-zero coding overhead. However, if you want your erasure code (even if you use space-optimal codes such as Reed-Solomon codes) to have efficient repair capabilities, it is unavoidable to have non-zero coding overhead. So some overhead is actually necessary for storage applications.
- [x] Founsure is exposed in non-sytematic form and thus natural encryption is embedded inside the code (supported by seeds).
- [x] Founsure is flexible i.e., if one wants to have a combination of erasure coding and replication, it is possible with founsure to have that by just setting the right parameters. Ofcourse this would require experience and familarity with the internals of the theory & library.

![Founsure](https://github.com/suaybarslan/founsure/blob/master/Images/founsure_3d.png)

Founsure 1.0 implements an LT code and a concatenated fountain code (a precode + LT code). More specifically, the precode is based on a systematic binary array codes whose parity check matrix exhibits sparseness as the block length gets large. The precode is concatenated by an LT code based on different degree and selection distributions. More precode support shall be provided in future releases. Also custom degree/selection distributions will be part of the library for flexibility reasons.

# Summary
There are four major functions of Founsure: encoding, decoding, repair and update.
There are also utility functions provided with the library to better use the library for different use cases. For instance, simDisk utility function has nothing to do with a user file and can be used to help system admins provide required reliability with their own system.

- [x] **FounsureEnc**: This function takes a file and produces multiple segments of data. The file itself does not appear in any of these segments.
- [x] **FounsureDec**: This function assumes  a ./Coding directory and a subset of encoded file segments to be available and decodes and re-generates the original file.
- [x] **FounsureRep**: Repair engine that also requires a Coding directory with enough number of files and
** fixes/repairs one or more data chunks should they have been erased, corrupted or flagged as unavailable.
** generates extra coding chunks should a code update has been requested. System update is triggered if data reliability is decreased/degraded over time or increased due to equipment replacements.
- [x] **DiskSim**: This utility function can be used to find fault tolerance of Founsure in terms of the number of failed disks that can be tolerated. This function is of great value to map founsure parameters such as k, n to number of disks and numer of tolerable disk failures and provide a way to understand fault tolerance numbers as in MDS codes (such as Jerasure or Intel's ISA libraries which are based on RS code implementions).
- [x] **genChecks**: This utility function is crucial for two different important functionalities: (1) fast/efficient repair/rebuild of data and (2) seemless on-the-fly update. 

# Installation
Installation from the source: There are four directories of source code:

* The src directory contains the founsure source code.
* The examples directory contains the example programs.
* The man page for the entire founsure software.
* The scripts page for running tests.

If you do not have autotools installed, you can simply install it using:
sudo yum install autotools autoconf (for centos), sudo apt-get install autotools-dev autoconf (for ubuntu)

Make sure you run autreconf before you run the usual installation procedure outline below.
- [x] **autoreconf --force --install**
- [x] **./configure**
- [x] **make**
- [x] **sudo make install**

This will install the library into your machine's lib directory, most probably /usr/local/lib.

The configuration process assumes shared objects are searched for in /usr/local/lib. If this is not the case on your system, you can specify a search path at configuration time. Alternatively if you receive the following error when you try to execute one of the main functions of the library to do encoding or decoding, setting the LD_LIBRARY_PATH to installed directory (For instance /usr/local/lib) and exporting it might help. Otherwise, let me know the specifics of the problem, I will give my own shot.

*error while loading shared libraries: libfounsure.so.2: cannot open shared object file: No such file or directory*

# Tests
### Basic Functionality Tests
#### Encode Example
The following command will encode a test file testfile.txt with k=500 data chunks with each chunk occupying t=512 bytes. The encoder generates n=1000 coding chunks using d="FiniteDist" degree distribution and p="ArrayLDPC" precoding. Finally, generated chunks are striped/written to s=10 distinct files for default disk/drive allocation under Coding directory.
```bash
founsureEnc -f  testfile.txt  -k  500  -n  1000  -t  512  -d’FiniteDist’ -p ’ArrayLDPC’ -s 10 -v
```
#### Decode Example
Let us erase one of the coding chunks (0007) and run Founsure decoder. The decoder shall generate a decoded file testfile_decoded.txt under Coding directory. You can use "diff" command to compare this file with the original.
```bash
rm -rf Coding/testfile_disk0007.txt
founsureDec -f testfile.txt -v
diff testfile.txt Coding/testfile_decoded.txt
```
#### Repair Example
The erased chunk (0007) can efficiently be repaired using three different types of check relations. First, we need to generate these checks then call repair function to repair the erased chunk.
```bash
genChecks -f testfile.txt -m 1 -v
founsureRep -f testfile.txt -v
```
#### Update Example
Let us assume we want to generate two more chunks (in addition to 10 so a total of 12) without re-encoding the entire data. In that case, we need to update the checks (add extra checks on top of the existing ones) and generate two more chunks accordingly. 
```bash
genChecks -f testfile -m 1 -v -e 2
founsureRep -f testfile.txt -v
```
### Unittests
For automated (and more detailed functionality) testing, we provide a self-contained unittest which attempts the following set of jobs, check if the system successfully completes these jobs and report either failure or success. Code parameter selections are made (hard coded) inside the unittest file. Different unittests can be generated by changing the code parameters inside this file.
```python
python founsure_unittest.py
```

- A binary file of size `fsize` is generated.
- A standard encoding function `founsureEnc` with default code parameters is executed. The process as well as the outputs (including metadata) is checked for accuracy.
- A file chunk is renamed to simulate loss. 
- A Decoder using `founsureDec` is executed and the output (decoded file) is compared with the original file for consistency check. 
- The lost file is renamed back to its original name.
- Data for efficient repair is generated (`genCheck`) and output ischecked for accuracy. Furthermore, metadata is checked if necessary modifications are made by the function `genCheck`.
- Data loss is resimulated through renaming. Repair function `founsureRep` is invoked to complete repair. Repaired data chunk is compared with the original to check exact repair accuracy.
- Data for efficient update is generated (`genCheck`) and output is checked for accuracy. Furthermore, metadata is checked if necessary modifications are made by the function `genCheck`.
- Repair function `founsureRep` is invoked to complete update. Updated data chunks are checked if they would be originally generated by the encoder with the modified metadata.

![Unittest](https://github.com/suaybarslan/founsure/blob/master/Images/unittest.png)

**For more details on the comprehensive testing of the library, please see** [the user manual](https://github.com/suaybarslan/founsure/blob/master/tests/Founsure_1_0_User_Manual.pdf).


# Reference

If you find this code useful in your work, we kindly request that you cite the following [paper](https://arxiv.org/abs/1702.07409):
* Suayb S. Arslan (2017) Founsure 1.0: An Erasure Code Library with Efficient Repair and Update Features. arXiv preprint arXiv:1702.07409.
```
@article{arslan2017founsure,
  title={Founsure 1.0: An erasure code library with efficient repair and update features},
  author={Arslan, {\c{S}}uayb {\c{S}}},
  journal={arXiv preprint arXiv:1702.07409},
  year={2017}
}
```
You can find the theory behind the code construction, motivation for fountain codes, mathematical machinary for analysis as well as different application areas in the following comprehensive [paper](https://arxiv.org/abs/1402.6016):  
```
@article{arslan2014incremental,
  title={Incremental redundancy, fountain codes and advanced topics},
  author={Arslan, Suayb S},
  journal={arXiv preprint arXiv:1402.6016},
  year={2014}
}
```

