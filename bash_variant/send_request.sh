#!/bin/bash

command=$1

case $command in
	"machineRegister")
		linuxVersion=$(cat /proc/version | cut -d ' ' -f3)
		echo $linuxVersion
		echo "192.168.1.52/api/main/$command" ;;
	"processRegister")
		pid=$2
		echo "192.168.1.52/api/main/$command" ;;
	"processUpdate")
		pid=$2
		echo "192.168.1.52/api/main/$command" ;;
	*)
		echo "Unknown command!" ;;
esac
