#! /bin/bash

UNAME=$(uname)
Platform=$UNAME
if [ "$UNAME" == "Linux" ] ; then
  Platform="Linux"
elif [[ "$UNAME" == CYGWIN* || "$UNAME" == MINGW* || "$UNAME" == MSYS* ]] ; then
  Platform="Windows"
fi
