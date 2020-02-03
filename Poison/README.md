# Poison
[![Poison](https://www.hackthebox.eu/storage/avatars/453800925395b3a5b14099e005fb5a77.png)](https://www.hackthebox.eu/home/machines/profile/132)

## Initial Foothold
From nmap we get ports 80 and 22.
```sh
$ nmap -A -v 10.10.10.84
```
From the web browser we open the ***listfiles.php*** and we get ***pwdbackup.txt*** wich contains a string in base64.
```sh
Username: charix
Password: Charix!2#4%6&8(0
```
We connect via ssh and after a basic enumeration we can see that ***Xvnc server*** is running as root.
```sh
$ ssh charix@10.10.10.84
pass: Charix!2#4%6&8(0
```
I order to use ***vcnviewer*** we must do ssh tunneling and connect locally.
```sh
$ ssh -L 5902:localhost:5901 charix@10.10.10.84
```
Inside home directory there is a file called ***secret.zip***. We download it locally and we unzip it using ***Charix!2#4%6&8(0*** as password. Finally we run ***vcnviewer*** and we put the file ***secret*** we just unziped, as parameter.
```sh
$ vncviewer localhost:5902 -passwd /home/name/secret
```
We got root!

[![bertolis](https://www.hackthebox.eu/badge/image/27897)](https://www.hackthebox.eu/home/users/profile/27897)  [![Poison](https://www.hackthebox.eu/images/logofull-tr-web.png)](https://www.hackthebox.eu/)
