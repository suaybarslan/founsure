# Founsure 1.0
An Erasure Code Library with Efficient Repair and Update Features: Version 1.0

# Overview and Objective
Founsure 1.0 is the very first version of a totally new erasure coding library using Intel's SIMD and multi-core architectures based on simple aligned XOR operations and fountain coding. The details of fountain coding is beyond the scope of this text. I encourage you to take a look at here. Founsure does not use specific SIMD instructions, it rather pays attention to memory alignment and leaves the assembly instructions to be optimed by the "magical" gcc/g++ compiler (with appropriate optimization flags). This library is expected to have comparable performance to other open source alternatives and yet shall provide distinguishably better repair and rebuilt performance which seem to be the main stream focus of erasure coding community nowadays. The version 1.0 has only two major functions "founsureEnc" and "founsureDec" for encoding and decoding, respectively. In other words, the function "founsureRep" for efficient repair is still missing. So the library is not COMPLETE and the documentation is probably not at a fulfilling stage at the moment. Given the time constraint i am doing my best to get them upto date as much as i can.

Here are the reasons why "Founsure" is developped and exists:
- [x] Founsure is based on fountain codes which are not space-optimal i.e., non-zero coding overhead. However, if you want your erasure code (even if you use space-optimal codes such as Reed-Solomon codes) to have efficient repair capabilities, it is unavoidable to have non-zero coding overhead. So some overhead is actually necessary for storage applications.
- [x] Founsure is exposed in non-sytematic form and thus natural encryption is embedded inside the code (supported by seeds).
- [x] Founsure is flexible i.e., if one wants to have a combination of erasure coding and replication, it is possible with founsure to have that by just setting the right parameters. Ofcourse this would require experience and familarity with the internals of the theory & library.

![Image of 3D coding structure of Founsure]
(https://github.com/suaybarslan/founsure/blob/master/Images/founsure_3d.png)

![Founsure](/images/founsure_3d.png)
Format: ![Founsure 1.0 3D code structure](http://http://www.suaybarslan.com/founsure.html)
