/* Copyright (c) 2000-2004 MySQL AB
   Use is subject to license terms

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA */

#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <string.h>


int main(int argc, char **argv){
  MYSQL *sock,mysql;
  char qbuf[1000000];

  mysql_init(&mysql);
  if (!(sock = mysql_real_connect(&mysql,"127.0.0.1","root","password","rocks",3306,NULL,0))){
    fprintf(stderr,"Couldn't connect to engine!\n%s\n",mysql_error(&mysql));
    perror("");
    exit(1);
  }
  sock->reconnect= 1;

  char query[] = "INSERT INTO test (id, foo) VALUES (1,'foo')";

  //for (int i=0; i<1000000; i++){
     if(mysql_query(sock,query)){
         fprintf(stderr,"Query failed (%s)\n",mysql_error(sock));
         exit(1);
     }
  //}

  if (mysql_query(sock, "SHOW engine rocksdb status")){
    fprintf(stderr,"Query failed (%s)\n",mysql_error(sock));
    mysql_close(sock);
    exit(1);
  }

  MYSQL_RES *result = mysql_store_result(sock);

  int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;

  while ((row = mysql_fetch_row(result))){
      for(int i = 0; i < num_fields; i++){
          printf("%s ", row[i] ? row[i] : "NULL");
      }

      printf("\n");
  }
  printf("\n\n\n");

  mysql_free_result(result);

  if (mysql_query(sock, "DELETE FROM test WHERE id=1")){
    fprintf(stderr,"Query failed (%s)\n",mysql_error(sock));
    mysql_close(sock);
    exit(1);
  }

  mysql_close(sock);
  exit(0);
  return 0;
}
