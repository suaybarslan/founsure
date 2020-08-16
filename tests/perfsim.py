#!/usr/bin/env python
# Suayb S. Arslan @ MEF University, 2020.
#                   Maslak, Sariyer, Istanbul.
# Calculation of frequencies of recovery.
# Let n disks and f of n are failed.
# This script computes m out of comb(n,f) that can be tolerated by the founsure. 

from itertools import combinations
import os
import fnmatch
import subprocess
import re

fname = 'test1'
fext  = ''
codingdir = 'Coding/'

disks = 10;
findDigits = 4; #default is 4 also set by parameter.h
#findDigits = len(str(disks));
NumOfFail = 2;
items = list(range(1,disks+1));
cwd = os.getcwd();

k=1030
n=2180
s=disks
d='FiniteDist'
#d='RSD'
t=512
if d is 'FiniteDist':
    p='ArrayLDPC';
else:
    p = 'None';

# reset Coding directory:
cmd = "rm -rf Coding/"
out = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True).communicate()[0];

# run encoder to fill up Coding directory:
cmd = "founsureEnc -f "+fname+" -k "+str(k)+" -n "+str(n)+" -s "+str(10)+" -d "+d+" -v -t "+str(t)+" -p "+p;
#print cmd

out = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True).communicate()[0];
pos = out.split("\n"); 

for i in range(0,11):
    if pos[i].find('Precode Type') == 0:
        break;

if pos[i][20:] == 'None':
    k = float(re.findall(r'\d+', pos[i+1])[0]);
    n = float(re.findall(r'\d+', pos[i+2])[0]);
if pos[i][20:] == 'ArrayLDPC':
    k = float(re.findall(r'\d+', pos[i+1])[0]);
    n = float(re.findall(r'\d+', pos[i+3])[0]);

print "Rate of the code: %.3f\n" % (k/n)
#print out

NumOfFailsum = 0;
totsum = 0;

for p in combinations(items, NumOfFail):
    totsum = totsum + 1;
    for i in range(0,NumOfFail):
        for file in os.listdir(codingdir):  
            filefullname = fname+'_disk'+str(p[i]).zfill(findDigits)+fext;
            if fnmatch.fnmatch(file, filefullname):     
                filefullname_modified = fname+'_disk'+str(p[i]).zfill(findDigits)+'_modified'+fext;
                os.rename(cwd+'/'+codingdir+filefullname, cwd+'/'+codingdir+filefullname_modified);
    
    # run decoder:
    cmd = "founsureDec -f "+fname+fext
    outp = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True).communicate()[0];
    if "Decoding is Successful" in outp: 
        print str(p)+'--> success'
    else:
        print str(p)+'--> fail'
        NumOfFailsum = NumOfFailsum + 1;
        
    for i in range(0,NumOfFail):  
        filefullname = fname+'_disk'+str(p[i]).zfill(findDigits)+fext;
        filefullname_modified = fname+'_disk'+str(p[i]).zfill(findDigits)+'_modified'+fext;
        os.rename(cwd+'/'+codingdir+filefullname_modified, cwd+'/'+codingdir+filefullname);
    #break;
    
print "Number of Failures:%d out of %d" % (NumOfFailsum, totsum)
print "Rate of the code: %.4f\n" % (k/n)

