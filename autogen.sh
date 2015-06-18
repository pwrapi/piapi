#!/bin/sh

rm -f *.in */*.in 
autoreconf --install --symlink --force
