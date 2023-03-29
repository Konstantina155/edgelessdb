import os

os.chdir('edgelessdb/3rdparty/edgeless-mariadb/storage/rocksdb')
print('Current Working Directory is: ', os.getcwd())

for i in range(1,3):
   os.system("gcc test1.c -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient -o test")
   os.system("./test --default-storage-engine=rocksdb--skip-innodb --default-tmp-storage-engine=MyISAM --rocksdb")

#show engine rocksdb status\G
#above contains rocksdb_block_cache status variables: show status like 'rocksdb_block_cache%';
#Rocksdb_memtable_total     | 10640 increases in every delete, insert, etc
