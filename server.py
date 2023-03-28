import os

os.chdir('edgelessdb/build')
print('Current Working Directory is: ', os.getcwd())
os.system(". /opt/edgelessrt/share/openenclave/openenclaverc")
os.system("OE_SIMULATION=1 ./edb")

#Next, run test1 (10M inserts for 100 bytes char) 3 times, in every loop keep statistics in a var
#Then print the average of 3 statistics

#Next, run test2 (10M inserts for 1000 bytes char) 3 times, in every loop keep statistics in a var
#Then print the average of 3 statistics

#Next, run test3 (10M inserts for 10000 bytes char) 3 times, in every loop keep statistics in a var
#Then print the average of 3 statistics


#See results for all of the above
