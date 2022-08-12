#!/bin/bash

ip_address=$1
command=$2

case $command in
	"machineRegister")
		linuxVersion=$(cat /proc/version | cut -d ' ' -f3)
		name=$(hostname)

		newId=$(curl -s -X POST -H "Content-type: application/json" \
			-d "{\"name\":\"$name\", \"linuxVersion\":\"$linuxVersion\"}" \
		 	--insecure https://$ip_address/api/main/machineRegister)

		printf "MachineId: %s\n" $newId > machineId

		echo $newId$'\n'
		echo "$ip_address/api/main/$command" ;;
	"processRegister")
		pid=$3

		machineId=$() #mozda preko nekog filea pohraniti id koji vrati machineRegister?

		processName=$(cat /proc/$pid/status | grep Name | cut -d$'\t' -f2)
		processPath=$(ps -p $pid -o cmd | cut -d$'\n' -f2)

		curl -s -X POST -H "Content-type: application/json" \
			-d "{\"processIdSystem\":\"$pid\", \"name\":\"$processName\", \"machineId\":\"$machineId\"}" \
			--insecure https://$ip_address/api/main/processRegister

		echo "$ip_address/api/main/$command" ;;
	"processUpdate")
		pid=$3

		state=$(cat /proc/$pid/status | grep Status | cut -d$'\t' -f2)
		cpuUtil=$(ps -p $pid -o %cpu | cut -d$'\n' -f2)
		memUtil=$(ps -p $pid -o %mem | cut -d$'\n' -f2)
		threads=$(cat /proc/$pid/status | grep Threads | cut -d$'\t' -f2)

		curl -s -X POST -H "Content-type: application/json" \
			-d "{\"processId\":\"$pid\", \"state\":\"$state\", \"cpuUtil\":\"$cpuUtil\", \"threads\":\"$threads\"}"
			--insecure https://$ip_address/api/main/processUpdate

		echo "$ip_address/api/main/$command" ;;
	*)
		echo "Unknown command!" ;;
esac
