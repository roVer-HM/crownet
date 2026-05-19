# Setup for macOS

## Docker

For the containers to work properly it is needed to give docker permissions to share the user directory with the containers. This can be done by adding the `/Users/<your_user>` in the list of shared directories. This list can be found under Docker -> Preferences -> Resources -> FILE SHARING.

## X11 with xquartz
Xquartz is the open source macOS version of the X Window System and is required to display windows that are running inside a container (for example, `omnetpp-ide`).

It can be installed with Homebrew via `brew install --cask xquartz`. Once installed, the Xquartz application can be started via Spotlight and a new console can be opened via `Command + N`.

Unfortunately, these consoles do not read `.bash_profile` on startup.
For easier usage, load the CrowNet environment variables manually by running `source <path_to_project>/setup -i`.

Furthermore, under `Xquartz -> Preferences -> Security`, enable the checkbox "Allow connections from network clients".

![Image of the network connection checkbox](../img/xquartz_security_settings.png)

Inside the Xquartz console, you can run a GUI container and the window should appear.

## Container setup

Currently, it is not possible to execute `sudo` commands inside a container on macOS, because the user is unknown inside the container.
This can lead to unexpected behavior, since the container cannot execute commands on its mounted directories.
A possible workaround is to create a new user inside the container based on your current user details.


Inside the Dockerfile add following code:
```
ARG USER_ID
ARG USER
RUN adduser --disabled-password --uid $USER_ID $USER
RUN usermod -aG sudo $USER
RUN passwd -d $USER
```
Then build the container with the `USER_ID`-Argument set to $(id -u) and `USER` set to $USER.
```
# example for omnetpp-ide
docker build -t sam-dev.cs.hm.edu:5023/rover/crownet/omnetpp-ide --build-arg USER_ID=$(id -u) --build-arg USER=$USER .
```

