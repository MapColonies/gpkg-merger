#!/bin/bash

gcc main.c gpkg.h gpkg.c -lsqlite3
./a.out /home/shimon/Downloads/bluemar.gpkg /home/shimon/Downloads/test4.GPKG
