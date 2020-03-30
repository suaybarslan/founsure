# Founsure 1.0
An Erasure Code Library with Efficient Repair and Update Features: Version 1.0

# Overview and Objective
Founsure 1.0 is the very first version of a totally new erasure coding library using Intel's SIMD and multi-core architectures based on simple aligned XOR operations and fountain coding. The details of fountain coding is beyond the scope of this text. I encourage you to take a look at here. Founsure does not use specific SIMD instructions, it rather pays attention to memory alignment and leaves the assembly instructions to be optimed by the "magical" gcc/g++ compiler (with appropriate optimization flags). This library is expected to have comparable performance to other open source alternatives and yet shall provide distinguishably better repair and rebuilt performance which seem to be the main stream focus of erasure coding community nowadays. The version 1.0 has only two major functions "founsureEnc" and "founsureDec" for encoding and decoding, respectively. In other words, the function "founsureRep" for efficient repair is still missing. So the library is not COMPLETE and the documentation is probably not at a fulfilling stage at the moment. Given the time constraint i am doing my best to get them upto date as much as i can.

Here are the reasons why "Founsure" is developped and exists:
- [x] Founsure is based on fountain codes which are not space-optimal i.e., non-zero coding overhead. However, if you want your erasure code (even if you use space-optimal codes such as Reed-Solomon codes) to have efficient repair capabilities, it is unavoidable to have non-zero coding overhead. So some overhead is actually necessary for storage applications.
- [x] Founsure is exposed in non-sytematic form and thus natural encryption is embedded inside the code (supported by seeds).
- [x] Founsure is flexible i.e., if one wants to have a combination of erasure coding and replication, it is possible with founsure to have that by just setting the right parameters. Ofcourse this would require experience and familarity with the internals of the theory & library.

![Founsure](https://github.com/suaybarslan/founsure/blob/master/Images/founsure_3d.png)
Format: ![Founsure 1.0 3D code structure](http://www.suaybarslan.com/founsure.html)

Founsure 1.0 implements an LT code and a concatenated fountain code (a precode + LT code). More specifically, the precode is based on a systematic binary array codes whose parity check matrix exhibits sparseness as the block length gets large. The precode is concatenated by an LT code based on different degree and selection distributions. More precode support shall be provided in future releases. Also custom degree/selection distributions will be part of the library for flexibility reasons.

# Summary
There are four major functions of Founsure: encoding, decoding, repair and update.
There are also utility functions provided with the library to better use the library for different use cases. For instance, simDisk utility function has nothing to do with a user file and can be used to help system admins provide required reliability with their own system.

- [x] FounsureEnc: This function takes a file and produces multiple segments of data. The file itself does not appear in any of these segments.
- [x] FounsureDec: This function assumes  a ./Coding directory and a subset of encoded file segments to be available and decodes and re-generates the original file.
- [x] DiskSim: This utility function can be used to find fault tolerance of Founsure in terms of the number of failed disks that can be tolerated. This function is of great value to map founsure parameters such as k, n to number of disks and numer of tolerable disk failures and provide a way to understand fault tolerance numbers as in MDS codes (such as Jerasure or Intel's ISA libraries which are based on RS code implementions).

# Installation
Installation from the source: There are four directories of source code:

* The src directory contains the founsure source code.
* The examples directory contains the example programs.
* The man page for the entire founsure software.
* The scripts page for running tests.

If you do not have autotools installed, you can simply install it using:
sudo yum install autotools autoconf (for centos), sudo apt-get install autotools-dev autoconf (for ubuntu)

Make sure you run autreconf before you run the usual installation procedure outline below.
- [x] autoreconf --force --install
- [x] ./configure
- [x] make
- [x] sudo make install

This will install the library into your machine's lib directory, most probably /usr/local/lib.

The configuration process assumes shared objects are searched for in /usr/local/lib. If this is not the case on your system, you can specify a search path at configuration time. Alternatively if you receive the following error when you try to execute one of the main functions of the library to do encoding or decoding, setting the LD_LIBRARY_PATH to installed directory (For instance /usr/local/lib) and exporting it might help. Otherwise, let me know the specifics of the problem, I will give my own shot.

-> error while loading shared libraries: libfounsure.so.2: cannot open shared object file: No such file or directory
