#!/bin/bash

dirname=$(dirname "$0")
tmp="${dirname#?}"

if [ "${dirname%$tmp}" != "/" ]; then
  dirname="$PWD/$dirname"
fi

if [ -z "$LD_LIBRARY_PATH" ]; then
  LD_LIBRARY_PATH="$dirname/plugins"
else
  LD_LIBRARY_PATH="$dirname/plugins:$LD_LIBRARY_PATH"
fi

export LD_LIBRARY_PATH

exec $dirname/ayam "$@"

