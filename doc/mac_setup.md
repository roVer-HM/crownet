# Setup for macOS

## Docker

For the containers to work properly it is needed to give docker permissions to share the user directory with the containers. This con be done by adding the `/Users/<your_user>` in list of shared directories. This list can be found under Docker -> Preferences -> Resources -> FILE SHARING.

## X11 with xquartz
Xquartz is the open source macOS version of the X Window System and it is required to display windows that are running inside a container (f.e. omnetpp-ide). 

It can be installed with homebrew via `brew install --cask xquartz`. Once installed, the xquartz application can be started via s dockerpotlight and a new console can be opened via `command + n`.

Unfortunatally the these consoles does not read the `.bash_profile` on startup. 
For easier usability it's recommended to add the crownet environment variables to the console by typing `source <path_to_project>/scripts/crownetenv`.

Furthermore under xquartz -> Preferences -> Security, the "Allow connections form network clients" checkbox must be enabled.

![Image of the network connection checkbox](img/xquartz_security_settings.png)

Inside the xquartz console you can run a gui-container and the window should appear.