#!/bin/bash

module="scull"
device="scull"

echo "scull unloader."
sleep 1
/sbin/rmmod $module $* || exit 1

rm -f /dev/${device}[0-3] /dev/${device}

echo "scull unloaded."

exit 0
