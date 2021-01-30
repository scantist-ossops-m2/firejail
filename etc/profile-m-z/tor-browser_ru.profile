# Firejail profile alias for torbrowser-launcher
# This file is overwritten after every install/update
# Persistent local customizations
include tor-browser_ru.local
# Persistent global definitions
# added by included profile
#include globals.local

noblacklist ${HOME}/.tor-browser_ru

mkdir ${HOME}/.tor-browser_ru
whitelist ${HOME}/.tor-browser_ru

# Redirect
include torbrowser-launcher.profile
