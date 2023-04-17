import os
import sys
import subprocess
from tabulate import tabulate
import time as tme
import threading
import re

#run sgx mode for 1 cache, 2 different threads in each write_buffer_size and for all tests
#in each test, we initialize connection in server and a thread is used for the client (establish connection, run tests)

os.system("gcc create_database_test.c -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient -o create_database_test")
os.system("gcc drop_table_get_size_usertable.c -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient -o drop_table_get_size_usertable")
os.system("gcc get_write_buffer_size.c -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient -o get_write_buffer_size")
os.system("gcc change_global_size.c -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient -o change_global_size")
os.system("gcc get_cache_size.c -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient -o get_cache_size")

table_workloads=[["Default workload — Insert only", "Insert : 100%", "Uniform", "-", "10 of 100 Bytes"],
                 ["A — Update heavy","Read: 50%, Update: 50%","Zipfian", "User session","10 of 100 Bytes"],
                 ["B — Read heavy", "Read: 95%, Update: 5%", "Zipfian", "Photo tagging", "10 of 100 Bytes"],
                 ["C — Read only", "Read: 100%", "Zipfian", "User profile cache", "10 of 100 Bytes"],
                 ["D — Read latest", "Read: 95%, Insert: 5%", "Latest", "User status updates", "10 of 100 Bytes"],
                 ["E — Short range", "Scan: 95%, Insert: 5%", "Zipfian / Uniform", "Threaded conversations", "10 of 100 Bytes"],
                 ["F — Balanced", "Read: 50%, Insert: 50%", "Uniform", "-", "10 of 100 Bytes"],
                 ["100 — Insert only", "Insert: 100%", "Uniform", "-", "1 of 100 Bytes"],
                 ["1000 — Insert only", "Insert: 100%", "Uniform", "-", "1 of 1000 Bytes"],
                 ["10000 — Insert only", "Insert: 100%", "Uniform", "-", "1 of 10000 Bytes"],
                 ["Template — Read heavy", "Read: 95%, Update: 5%", "Zipfian", "-", "10 of 100 Bytes"]]

###write_buffer_sizes=[4194304,8388608,16777216]
cache_sizes=[16777216,33554432,67108864,134217728]
workload=["A","B","C","D","E","F","Template"]
general_table=[]
time_of_workload=[]
count_inserts=[]
size_cache=[]
ops=[]


def init_info():
    my_file = open("result_ycsb_sgx_500000.txt",'a')
    #print(tabulate(table_workloads, headers=['Workload','Operations','Record Selection', 'Application example','Fields per record'],tablefmt='github') + '\n', file=my_file) 
    print(tabulate([], headers=['Workload','Time (sec) on SGX with 1 thread','Cache','OPS','Count'],tablefmt='github'),file=my_file)
    my_file.close()

def run_benchmark():
    for w in workload:
        table=[[]]
        table[0].append(w)
        if (w == "E" or w == "F"): continue
        manage_connection_run_tests("-P go-ycsb/workloads/workload_" + w.lower() + " -p recordcount=500000",33554432,table) #+ w.lower()
    table=[[]]
    table[0].append("C")
    manage_connection_run_tests("-P go-ycsb/workloads/workload_c -p recordcount=500000",16777216,table) #+ w.lower()
    table=[[]]
    table[0].append("D")
    manage_connection_run_tests("-P go-ycsb/workloads/workload_c -p recordcount=500000",134217728,table) #+ w.lower()
    # 1 THREAD
    # for w in workload:
    #     table=[[]]
    #     table[0].append(w)
    #     if (w == "C" or w == "F" or w == "B"): continue
    #     manage_connection_run_tests("-P go-ycsb/workloads/workload_" + w.lower() + " -p recordcount=500000",33554432,table) #+ w.lower()
    # table=[[]]
    # table[0].append("F")
    # manage_connection_run_tests("-P go-ycsb/workloads/workload_f -p recordcount=500000",16777216,table) #+ w.lower()
    table=[[]]
    table[0].append("F")
    manage_connection_run_tests("-P go-ycsb/workloads/workload_f -p recordcount=500000",67108864,table) #+ w.lower()
    table=[[]]
    table[0].append("F")
    manage_connection_run_tests("-P go-ycsb/workloads/workload_f -p recordcount=500000",134217728,table) #+ w.lower()
    for i in cache_sizes:
        table=[[]]
        table[0].append("Template")
        manage_connection_run_tests("-P go-ycsb/workloads/workload_template -p recordcount=500000",i,table) #+ w.lower()

def client_side(i,workload_option,cache_size,table):
    initial_stdout = sys.stdout
    file = open('result_ycsb_sgx_500000.txt', 'a')
    sys.stdout = file
    
    file2 = open('result_ycsb_sgx_500000_extensively.txt', 'a')

    whole_time=""

    tme.sleep(50)
    output = subprocess.Popen(["curl -k --data-binary @src/edgelessdb/manifest.json https://localhost:8080/manifest"], stdout=subprocess.PIPE, shell=True)
    
    tme.sleep(50)
    #output = subprocess.Popen(["./create_database_test --default-storage-engine=rocksdb--skip-innodb --default-tmp-storage-engine=MyISAM --rocksdb"], stdout=subprocess.PIPE, shell=True)

    if len(sys.argv) > 1: #day
        threads = 8
    else: #night
        threads = find_next_num_threads(i)

    output = subprocess.Popen(["./change_global_size "+ str(cache_size)+ " --default-storage-engine=rocksdb--skip-innodb --default-tmp-storage-engine=MyISAM --rocksdb"], stdout=subprocess.PIPE, shell=True)

    output = subprocess.Popen(["./get_cache_size --default-storage-engine=rocksdb--skip-innodb --default-tmp-storage-engine=MyISAM --rocksdb"], stdout=subprocess.PIPE, shell=True)
    (size_of_cache, err) = output.communicate()

    ycsb_command = "./go-ycsb/bin/go-ycsb load mysql -p mysql.host=127.0.0.1 -p mysql.port=3306 -p mysql.user=konstantina -p mysql.db=test -p threadcount=" + str(threads) + " " + workload_option
    output = subprocess.Popen([ycsb_command], stdout=subprocess.PIPE, shell=True)
    (time, err) = output.communicate()

    print("Running go-ycsb with w: " + str(workload_option) + " ,c: " + str(cache_size) + " for " + str(i) + "th time", file=file2)
    print(time,file=file2)
    print("\n\n", file=file2)

    output = subprocess.Popen(["./drop_table_get_size_usertable --default-storage-engine=rocksdb--skip-innodb --default-tmp-storage-engine=MyISAM --rocksdb"], stdout=subprocess.PIPE, shell=True)
    
    # output = subprocess.Popen(["./get_write_buffer_size --default-storage-engine=rocksdb--skip-innodb --default-tmp-storage-engine=MyISAM --rocksdb"], stdout=subprocess.PIPE, shell=True)
    # (write_buffer_size, err) = output.communicate()

    
    
    #whole_time += str(re.findall(r'\b\d+\b', str(write_buffer_size))[0])
    #if (i==0): whole_time+=","
    if (len(size_of_cache) == 0): size_cache.append(0)
    else: size_cache.append(re.findall(r'\d+', str(cache_size)))

    if ("Finished" in str(time) and "Run finished " in str(time)):
        result = str(time).split("Run finished ")[1]
        time_of_workload.append(str(result.partition("Finished")[0]))
        
        result_after_finished = str(time).split("Finished counting")[1]
        result_count = str(result_after_finished).split("Count: ")[1]
        count_inserts.append(str(result_count.partition(", OPS")[0]))
        
        result_ops = str(result_after_finished).split("OPS: ")[1]
        ops.append(str(result_ops.partition(", Avg")[0]))

    if (i==1 or i==3):
        table[0].append(','.join(str(v) for v in time_of_workload))
        time_of_workload.clear()
    
    if (i==1): #it is 3 for 4,8 threads
        #table[0].append(whole_time)
        size_cache.append(str(cache_size))
        table[0].append(','.join(str(v) for v in size_cache))
        size_cache.clear()
        table[0].append(','.join(str(v) for v in ops))
        ops.clear()
        table[0].append(','.join(str(v) for v in count_inserts))
        count_inserts.clear()
        print(tabulate(table, "",tablefmt='github'))
        general_table.append(table)
    
    print(general_table)

    # subprocess.call(["sudo systemctl stop mariadb"], shell=True)
    

    subprocess.call(["sudo docker stop my_edb && sudo docker rm my_edb"], shell=True)

    sys.stdout = initial_stdout
    file.close()

def manage_connection_run_tests(workload_option,cache_size,table):
    for i in range(2):
        client = threading.Thread(args=(i,workload_option,cache_size,table),target=client_side)
        client.start()

        file_ = open("time_sudo_docker.txt","a")
        print("Running go-ycsb with w: " + str(workload_option) + " ,c: " + str(cache_size) + " for " + str(i) + "th time", file=file_)
        file_.close()

        #os.system("(time -p sudo systemctl start mariadb) 2>> time_docker_mariadb.txt")
        subprocess.call(["python3 server.py"], shell=True)
        client.join()

def find_next_num_threads(i):
    if (i < 2):
        return 1
    elif (i < 4):
        return 4

init_info()
run_benchmark()
