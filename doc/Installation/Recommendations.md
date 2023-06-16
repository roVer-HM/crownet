# Recommendations


## Start SSH agent automatically
When logging in into your profile, you are asked for your passphrase for your key:
 ```
eval `ssh-agent`
SSH_ASKPASS='ssh-askpass'
ssh-add ~/.ssh/id_rsa
```
with id_rsa as your private key.