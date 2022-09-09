#!/bin/sh
set -e
clang-format -style=file --assume-filename=C -i main.c
