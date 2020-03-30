#!/usr/bin/env python
# Suayb S. Arslan @ MEF University, 2020.
#                   Maslak, Sariyer, Istanbul.
# Calculation of frequencies of recovery.
# Let n disks and f of n are failed.
# This script computes m out of comb(n,f) 
# that can be tolerated by the founsure. 

from itertools import combinations
import os
import fnmatch
import subprocess
import re
import numpy



# Parameter definitions:
fname 		= 'test1'
fext  		= ''
codingdir 	= 'Coding/'
disks 		= 10;
findDigits 	= 4; #default is 4 also set by parameter.h
#findDigits = len(str(disks));
NumOfFail 	= 6;
items 		= list(range(1,disks+1));
cwd 		= os.getcwd();
k			= 1045
n			= 2900
s			= disks
d			= 'RSD'
#d='RSD'
t			= 512
MAXI		= 100


# Script starts here:
if d is 'FiniteDist':
    p ='ArrayLDPC'; 
else:
    p = 'None';

# reset Coding directory:
cmd = "rm -rf Coding/"
out = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True).communicate()[0];

# run encoder to fill up Coding directory:
cmd = "founsureEnc -f "+fname+" -k "+str(k)+" -n "+str(n)+" -s "+str(disks)+" -d "+d+" -v -t "+str(t)+" -p "+p;

out = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True).communicate()[0];
pos = out.split("\n"); 
print out
for i in range(0,11):
    if pos[i].find('Precode Type') == 0:
        break;

print pos[i][20:]
if pos[i][20:] == 'None':
    k = float(re.findall(r'\d+', pos[i+1])[0]);
    n = float(re.findall(r'\d+', pos[i+2])[0]);
if pos[i][20:] == 'ArrayLDPC':
    k = float(re.findall(r'\d+', pos[i+1])[0]);
    n = float(re.findall(r'\d+', pos[i+3])[0]);

print k, n
print "Rate of the code: %.6f\n" % (k/n)
#print out
print "Computing repair data.."
cmd = "genChecks -f "+fname+" -m "+str(1);
out = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True).communicate()[0];

BWsum 		= [];
totsum 		= 0;
enRep		= False
if os.path.isfile(cwd+'/'+codingdir+fname+'_check.data'):
	enRep = True

for p in combinations(items, NumOfFail):
    totsum = totsum + 1;
    for i in range(0,NumOfFail):
        for file in os.listdir(codingdir):  
            filefullname = fname+'_disk'+str(p[i]).zfill(findDigits)+fext;
            if fnmatch.fnmatch(file, filefullname):     
                filefullname_modified = fname+'_disk'+str(p[i]).zfill(findDigits)+'_modified'+fext;
                os.rename(cwd+'/'+codingdir+filefullname, cwd+'/'+codingdir+filefullname_modified);
    
    # run repair:
    cmd = "founsureRep -f "+fname+fext
    outp = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True).communicate()[0];
    print outp
    if outp.find("Error")<0:
		if enRep:
			BWsum.append(int(outp.split()[11+3*(NumOfFail-1)]));
		else:
			BWsum.append(int(outp.split()[26+3*(NumOfFail-1)]));
        
    for i in range(0,NumOfFail):  
        filefullname = fname+'_disk'+str(p[i]).zfill(findDigits)+fext;
        filefullname_modified = fname+'_disk'+str(p[i]).zfill(findDigits)+'_modified'+fext;
        os.rename(cwd+'/'+codingdir+filefullname_modified, cwd+'/'+codingdir+filefullname);
    print totsum
    if totsum > MAXI:
    	break;
   	
print "Avg. BW : %d " % (numpy.mean(BWsum))
print "STD. BW : %d " % (numpy.std(BWsum))
print "Min BW : %d " % (numpy.min(BWsum))
print "Max BW : %d " % (numpy.max(BWsum))
print "Rate of the code: %.6f\n" % (k/n)

