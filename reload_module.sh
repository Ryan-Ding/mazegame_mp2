#!/bin/bash

pwd;
cd ./module/;
make ;
sudo /sbin/rmmod tuxctl;
sudo /sbin/insmod ./tuxctl.ko;
cd ../;
make input;