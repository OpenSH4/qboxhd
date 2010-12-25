#!/bin/sh

cp src/.libs/libdvbsi++.so.0.0.0 /mnt/rootfs/usr/local/lib/libdvbsi++.so.0.0.0.new
cp src/libdvbsi++.la /mnt/rootfs/usr/local/lib/libdvbsi++.la.new

echo "Done!"
