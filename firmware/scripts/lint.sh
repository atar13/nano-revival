#!/bin/sh

cpplint \
    --filter=-legal,-readability/casting,-readability/todo \
    --recursive \
    ./src \
    ./include
