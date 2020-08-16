import unittest
import filecmp
import os
import sys
import fnmatch
import subprocess
import re
import numpy
import random
import shutil

unittest.TestLoader.sortTestMethodsUsing = None

# Global Parameter definitions:
fname 		= 'test1'
fsize 		= 8388608 # 8MiB is assumed 
fext  		= ''
disks 		= 10;
findDigits 	= 4; #default is 4 also set by p
k		    = 2000
n		    = 3950
codingdir 	= 'Coding/'
s		    = disks
d		    = 'RSD'
t		    = 512
extradisks	= 2
cwd 		= os.getcwd();
DEVNULL     = open(os.devnull,'wb');
pp          = random.randint(1,disks);
# reset Coding directory:
cmddest 	= "rm -rf Coding/"
# choose precoder:
if d is 'FiniteDist':
    p ='ArrayLDPC'; 
else:
    p = 'None';

class TestFounsure(unittest.TestCase):

    #def test_sum(self):
    #    self.assertEqual(sum([1,4,3]),6,"Should be 6")
    def padots(self,x,n):
	    x += ' '
	    return x + '.'*(n-len(x))
    
    def generate_big_random_bin_file(self,filename,size):
        """
        Generation of a random binary file with the specified size for testing
	    ======================================================================
        :parameter filename: The name of the file
        :parameter size: The desired size of the file
        :output: void
        """
        with open('%s'%filename, 'wb') as fout:
            fout.write(os.urandom(size))
            
        MBsize = int(size)/(1024*1024)
        print '%s %s' % ( self.padots("A random binary file with size "+str(MBsize)+"MB generated",58), "OK");
        pass 
        
        
    def test_01_encoding(self):
        """
        Encoding of a specific file
	    ===========================
        :parameter fname: The name of the file to be encoded
        :parameter fsize: The size of the file to be encoded
        :output: encoded set of file partitions to be written to where codingDir points to. 
        """
	    # reset the destination folder. 
        out = subprocess.Popen(cmddest, stdout=DEVNULL, shell=True).communicate()[0];

        # alternative -- generate a test file:
        # cmd = "dd if=/dev/urandom of="+fname+" bs=1K count=1024" 
        # out = subprocess.Popen(cmd, stdout=DEVNULL, shell=True).communicate()[0];

        try:
            self.generate_big_random_bin_file(fname,fsize)
        except:
            unittest_exception = sys.exc_info()
            raise unittest_exception[0],unittest_exception[1],unittest_exception[2].tb_next
       
        # run encoder to fill up Coding directory:
        cmd = "founsureEnc -f "+fname+" -k "+str(k)+" -n "+str(n)+" -s "+str(disks)+" -d "+d+" -v -t "+str(t)+" -p "+p;
        out = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True).communicate()[0];
        
        try:
            self.assertTrue(os.path.exists(codingdir+'/'+fname+'_meta.txt'), "Meta Data cannot be created.")
            print '%s %s' % ( self.padots("Encoding DONE, Metadata check",58), "OK");
        except:
            unittest_exception = sys.exc_info()
            raise unittest_exception[0],unittest_exception[1],unittest_exception[2].tb_next
            
        #self.assertTrue(filecmp.cmp(fname,fname))
        
    def test_02_introduce_loss(self,testok=True):
        """
        Simulate data loss
	    ==================
        :parameter testok: When True test is run and the result is printed out.
        :fname: The name of the file whose partitions can be lost
        :findDigits: Number of digits used in file fragment naming 
        :codingDir: directory to which encoded file partitions are written.
	    :pp: the index of the encoded data parition to be renamed (simulated loss). 
        """
    
        filefullname = fname+'_disk'+str(pp).zfill(findDigits)+fext;
        for file in os.listdir(codingdir): 
            if fnmatch.fnmatch(file, filefullname):  
                filefullname_modified = fname+'_disk'+str(pp).zfill(findDigits)+'_modified'+fext;
                try:
                    os.rename(cwd+'/'+codingdir+filefullname, cwd+'/'+codingdir+filefullname_modified)
                    if testok:
                        print '%s %s' % ( self.padots("Lost file is renamed",58), "OK");
                    else:
			            return
                except:
                    unittest_exception = sys.exc_info()
                    raise unittest_exception[0],unittest_exception[1],unittest_exception[2].tb_next    
        
    def test_03_decoding(self):
        """
        Run Decoding and Compare decoded and original files. 
	    ====================================================
        :fname: The name of the file whose partitions can be lost
        :codingDir: directory to which encoded file partitions are written. 
        """
    
        # run decoder to recover the file:
        cmd = "founsureDec -f "+fname
        out = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True).communicate()[0];
        
        try: 
            self.assertTrue(filecmp.cmp(cwd+'/'+fname,cwd+'/'+codingdir+'/'+fname+'_decoded',shallow=False),'File do not check: Decoding Error!')
            print '%s %s' % ( self.padots("Decoding is succesful",58), "OK");
        except:
            unittest_exception = sys.exc_info()
            raise unittest_exception[0],unittest_exception[1],unittest_exception[2].tb_next 
            
        # erase decoded file:
        cmd = "rm -rf Coding/"+fname+"_decoded"
        out = subprocess.Popen(cmd, stdout=DEVNULL, shell=True).communicate()[0];

    def test_04_repair_conventional(self):
        """
        Run Conventional Repair.
	    ====================================================
        :fname: The name of the file whose partitions can be lost
        :codingDir: directory to which encoded file partitions are written. 
        """

        # run repair in a conventional way to recover the file:
        cmd = "founsureRep -f "+fname
        out = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True).communicate()[0];
        
        try:
            repaired_file_partition = cwd+'/'+codingdir+'/'+fname+'_disk'+str(pp).zfill(findDigits)+fext;
            files_partition_renamed = cwd+'/'+codingdir+'/'+fname+'_disk'+str(pp).zfill(findDigits)+'_modified'+fext;
            self.assertTrue(filecmp.cmp(repaired_file_partition,files_partition_renamed,shallow=False),'Repair do not check: Repair Error!')
            print '%s %s' % ( self.padots("Conventional Data Repair is succesful",58), "OK");
        except:
            unittest_exception = sys.exc_info()
            raise unittest_exception[0],unittest_exception[1],unittest_exception[2].tb_next 
	
        
    def test_05_take_back_loss(self,testok=True):
        """
        Simulate encoded data partition regeneration
	    Since we do not actually erase but rename data paritions, 
	    we name it back to return to the original output of encoding.
	    Decoding cannot recognize the new name so treats it as lost. 
	    =============================================================
        :parameter fname: The name of the file whose partitions can be lost
        :findDigits: Number of digits used in file fragment naming 
        :codingDir: directory to which encoded file partitions are written.
        """
     
        # rename the renamed file to bring it back
        filefullname = fname+'_disk'+str(pp).zfill(findDigits)+fext;
        filefullname_modified = fname+'_disk'+str(pp).zfill(findDigits)+'_modified'+fext;
        try:
            os.rename(cwd+'/'+codingdir+filefullname_modified, cwd+'/'+codingdir+filefullname)
            if testok:
                print '%s %s' % ( self.padots("Lost file is renamed back",58), "OK");
            else:
                return
        except:
            unittest_exception = sys.exc_info()
            raise unittest_exception[0],unittest_exception[1],unittest_exception[2].tb_next  

    def	test_06_generate_checks(self):
        """
        Check Generation for efficient repair: Generate check #2 and check #3 at the 
        same time using the algorithm defined in the paper.
	    ===================================================================
        :parameter fname: The name of the file whose partitions can be lost
        :codingDir: directory to which encoded file partitions are written.
        """
        
        # before modifying the metadata, let us make a copy for file-compare/update functions later. 
        try:
            shutil.copyfile(codingdir+'/'+fname+'_meta.txt',codingdir+'/'+fname+'_meta_copy.txt');
            self.assertTrue(filecmp.cmp(codingdir+'/'+fname+'_meta.txt',codingdir+'/'+fname+'_meta_copy.txt',shallow=False),'Metadata cannot be copied!')
            print '%s %s' % ( self.padots("Metadata copied",58), "OK");
        except:
            unittest_exception = sys.exc_info()
            raise unittest_exception[0],unittest_exception[1],unittest_exception[2].tb_next 
            
        # now, run check generation function for efficient repair. 
        cmd = "genChecks -f "+fname+" -m 1"
        out = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True).communicate()[0];
        
        try:
            self.assertTrue(os.path.exists(codingdir+'/'+fname+'_check.data'), "Repair data cannot be created.")
            print '%s %s' % ( self.padots("Repair data (checks) generated",58), "OK");
        except:
            unittest_exception = sys.exc_info()
            raise unittest_exception[0],unittest_exception[1],unittest_exception[2].tb_next
            
        # check if metadata is succesfully modified.
        try:
            self.assertFalse(filecmp.cmp(codingdir+'/'+fname+'_meta.txt',codingdir+'/'+fname+'_meta_copy.txt',shallow=False),'Metadata could NOT be modified!')
            print '%s %s' % ( self.padots("Metadata modification for data repair",58), "OK");
        except:
            unittest_exception = sys.exc_info()
            raise unittest_exception[0],unittest_exception[1],unittest_exception[2].tb_next 

    def test_07_repair_efficient(self):
        """
        Run Efficient Repair. 
	    ===================================================================
        :parameter fname: The name of the file whose partitions can be lost
        :codingDir: directory to which encoded file partitions are written.
        :findDigits: total number of digits used to name file partitions. 
        """
    
        #reintroduce loss without testing/comments
        self.test_02_introduce_loss(False)
        
        # run repair in an efficient way to recover the file: (Less bandwidth)
        cmd = "founsureRep -f " + fname
        
        try:
            out = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True).communicate()[0];
            repaired_file_partition = cwd+'/'+codingdir+'/'+fname+'_disk'+str(pp).zfill(findDigits)+fext;
            files_partition_renamed = cwd+'/'+codingdir+'/'+fname+'_disk'+str(pp).zfill(findDigits)+'_modified'+fext;
            self.assertTrue(filecmp.cmp(repaired_file_partition,files_partition_renamed,shallow=False),'Repair do not check: Repair Error!')
            print '%s %s' % ( self.padots("Efficient Data Repair is succesful",58), "OK");
        except:
            unittest_exception = sys.exc_info()
            raise unittest_exception[0],unittest_exception[1],unittest_exception[2].tb_next 
            
        #test is finished, rename the file
        self.test_05_take_back_loss(False)

    def test_08_generate_check_update(self):
        """
        Check Generation for efficient update: Generate check #2 and check #3 at the 
        same time using the algorithm defined in the paper.
	    ===================================================================
        :parameter fname: The name of the file whose partitions can be lost
        :codingDir: directory to which encoded file partitions are written.
        """
    
        # revert the metadata to the original from efficient repair. 
        try:
            os.rename(codingdir+'/'+fname+'_meta_copy.txt', codingdir+'/'+fname+'_meta.txt');
        except:
            unittest_exception = sys.exc_info()
            raise unittest_exception[0],unittest_exception[1],unittest_exception[2].tb_next  
        
        # before modifying the metadata, let us make a copy for file-compare/update functions later. 
        try:
            shutil.copyfile(codingdir+'/'+fname+'_meta.txt',codingdir+'/'+fname+'_meta_copy.txt');
            self.assertTrue(filecmp.cmp(codingdir+'/'+fname+'_meta.txt',codingdir+'/'+fname+'_meta_copy.txt',shallow=False),'Metadata cannot be copied!')
        except:
            unittest_exception = sys.exc_info()
            raise unittest_exception[0],unittest_exception[1],unittest_exception[2].tb_next 
        
        # now, run check generation function for efficient update. 
        cmd = "genChecks -f "+fname+" -m 1 -e "+str(extradisks)
        out = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True).communicate()[0];
        
        try:
            self.assertTrue(os.path.exists(codingdir+'/'+fname+'_check.data'), "Update data cannot be created.")
            print '%s %s' % ( self.padots("Update data (checks) generated",58), "OK");
        except:
            unittest_exception = sys.exc_info()
            raise unittest_exception[0],unittest_exception[1],unittest_exception[2].tb_next
       
        # check if metadata is succesfully modified.
        try:
            self.assertFalse(filecmp.cmp(codingdir+'/'+fname+'_meta.txt',codingdir+'/'+fname+'_meta_copy.txt',shallow=False),'Metadata could NOT be modified!')
            print '%s %s' % ( self.padots("Metadata modification for data update",58), "OK");
        except:
            unittest_exception = sys.exc_info()
            raise unittest_exception[0],unittest_exception[1],unittest_exception[2].tb_next 
            
    def test_09_update_efficient(self):
        """
        Run Efficient Update. 
	    ===================================================================
        :parameter fname: The name of the file whose partitions can be lost
        :codingDir: directory to which encoded file partitions are written.
        :findDigits: total number of digits used to name file partitions. 
        """
    
        # learn new code parameters from the check.data for update 
        meta_data_file_open = open(codingdir+'/'+fname+'_meta.txt')
        for pos, line in enumerate(meta_data_file_open):
            if pos is 5:
                newcodeparams = [int(i) for i in line.split() if i.isdigit()];
                
        # run update in a efficient way to recover the file:
        cmd = "founsureRep -f "+fname
        
        try:
            out = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True).communicate()[0];
            print '%s %s' % ( self.padots("Efficient Data Update is successful",58), "OK");
        except:
            unittest_exception = sys.exc_info()
            raise unittest_exception[0],unittest_exception[1],unittest_exception[2].tb_next 
            
        # erase the metadata/check.data for conventional repair. 
        try:
            os.remove(codingdir+'/'+fname+'_meta_copy.txt');
            os.remove(codingdir+'/'+fname+'_check.data');
        except:
            unittest_exception = sys.exc_info()
            raise unittest_exception[0],unittest_exception[1],unittest_exception[2].tb_next  
        
if __name__ == '__main__':
    print "Execution of tests:"
    print "----------------------------------------------------------------------"
    unittest.main()
