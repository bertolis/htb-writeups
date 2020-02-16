# Chain
&nbsp;

## Analyzing the image
In order to analyze the image we have, we are going to create python [script](https://github.com/grpapoutsis/Stegit/blob/master/stegit.py) and use some libraries like Pillow. This [script](https://github.com/grpapoutsis/Stegit/blob/master/stegit.py) will check for hidden messages in the pixel's bits. For this specific image, ordinary tools couldn't find differences in the colors, so we will start by checking the LSB.

Once we convert the pixels to binary and then their LSB binary to ASCII, we check if we can detect any human readable words by using regular expressions. After some effort, we figure out that if we include symbols for base64 encoding, we can extract some information.
```
$ python stegit.py
1. Analyze
2. Hide
3. Extract
4. Encrypt
5. Decrypt
3

Possible messages:
YWxpc2FzdWNrcw==
Y3VybCBodHRwOi8vMTc0LjEzOC4zNC4xNDY6MTIzNC90ZWtrZW43LnppcCAtLW91dHB1dCB0ZWtrZW43LnppcA==

Select method:
1. Base64
1

The hidden message is:
alisasucks

The hidden message is:
curl http://174.138.34.146:1234/tekken7.zip --output tekken7.zip

Finished!
```

We have extracted two strings in base64. Let’s see how we can use them.
- alisasucks
- 174.138.34.146 1234 tekken7.zip

Connecting to this service over HTTP, gives us a password protected zip file, which we can unlock with the first string we found erlier.
```
$ curl http://174.138.34.146:1234/tekken7.zip --output tekken7.zip
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100  9.9M  100  9.9M    0     0   937k      0  0:00:10  0:00:10 --:--:--  932k

$ unzip tekken7.zip
Archive:  tekken7.zip
[tekken7.zip] crack_images/hwoarang.png password:
  inflating: crack_images/hwoarang.png 
```

From the zip file we get hwoarang.png. Again, at first glance there are no changes in the colors. So we start analyzing the image on the same way.
```
$ python stegit.py
1. Analyze
2. Hide
3. Extract
4. Encrypt
5. Decrypt
3

Possible messages:
MTc0LjEzOC4zNC4xNDYgNTY1Ng==
M4V_QiS8dh7X6pyCLXZ8OKkHenObng8DikOtPPcJaKc=

Select method:
1. Base64
1

The hidden message is:
174.138.34.146 5656

Finished!
```
This time we had to add some more characters in our regex like "_]" to match right pattern because the image contains strings in different encoding method. Here is my regex for this matching:
```
[-A-Za-z0-9+=_]{14,}
```
From this images we got a two strings as well. The second one has an unknown format while the first one gives us an IP and a port. If we try to connect with netcat, we get a lot of output. By redirecting the output into a file, we get another image.
```
$ python stegit.py
1. Analyze
2. Hide
3. Extract
4. Encrypt
5. Decrypt
3

Possible messages:
gAAAAABeSRBfUnSZA0jYaClMeKirgJd4yB7me51Y5coIDBRv3rgNbAb5U6Qynju9RIHcN5-RN5-K4tjrCto-YSGt0pIEnJqY_LB56oLkxq54ei7EAyeQJWgcaKxWM1FcjmWzJD2gQ6jv

Finished!
```
The output this time is encrypted and not encoded. So after googling a little while having in mind the format of this string and the previous one, we come to the conclude that is encrypted with the Fernet symmetric algorithm.

Easaly using python we can import Cryptography library in our [script](https://github.com/grpapoutsis/Stegit/blob/master/stegit.py), and decrypt the string using the key we found.
```
$ python stegit.py
Select method:
1. Base64
2. Fernet
2

Give key to decrypt with: M4V_QiS8dh7X6pyCLXZ8OKkHenObng8DikOtPPcJaKc=

The hidden message is:
flag{thekingofironfisttournament}

Finished!
```
Eventually the decrypted message gives as the flag.
