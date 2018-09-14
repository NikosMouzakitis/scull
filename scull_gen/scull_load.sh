#/bin/bash

#	scull_load.sh 
#
#	Script to load module into Kernel,
#	create the desired minor numbers 
#	in /dev and related privillages.
#
# Uncomment to trace bugs.
#set -x

echo "Scull Loader."

module="scull"
mode="664"
device="scull"
space=" "
count=0
major=""

echo $module
/sbin/insmod ./$module.ko $* || exit 1

rm -f /dev/${device}[0-3]

major=$(awk "\$2==\"$module\" {print $1}" /proc/devices)

echo "Major info: "
sleep 1
echo $major
sleep 1
if [ "$major" == "" ]; then
		echo "Error getting major number"
		echo "Loader termination"
		exit 1
fi

#extract the number, and assign its value to major.

mj_len=${#major}

for i in $(seq 0 1 $mj_len)
do
	let count=$count+1

	if [[ ${major:$i:1} -eq $space ]]; then 
		break
	fi
done

major=${major:0:$count}

#to-do: Fix this with a for loop.
mknod /dev/"${device}"0 c $major 0
mknod /dev/"${device}"1 c $major 1
mknod /dev/"${device}"2 c $major 2
mknod /dev/"${device}"3 c $major 3

group="staff"
grep -q '^staff' /etc/group || group="wheel"

chgrp $group /dev/${device}[0-3]
chmod $mode /dev/${device}[0-3]

echo "Module loading completed."

exit 0
