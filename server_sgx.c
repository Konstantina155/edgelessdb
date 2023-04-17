import os
import sys
 
os.system("(time -p sudo docker run -t --name my_edb -p3306:3306 -p8080:8080 --device /dev/isgx -v /var/run/aesmd:/var/run/aesmd ghcr.io/edgelesssys/edgelessdb-debug-1gb) 2>> time_sudo_docker.txt")
