# Recommendations


## Start SSH agent automatically
When logging in to your profile, you may be asked repeatedly for your SSH key passphrase. Start the SSH agent and add your key:
 ```
eval "$(ssh-agent -s)"
SSH_ASKPASS='ssh-askpass'
ssh-add ~/.ssh/id_rsa
```
Replace `id_rsa` with your private key file if you use a different key name.