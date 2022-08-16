#!/bin/bash

ip_address=$1
command=$2

case $command in
	"machineRegister")
		linuxVersion=$(cat /proc/version | cut -d ' ' -f3)

		#registracija mašine
		name=$(hostname)
		if [ -z "$(cat machineId | grep MachineId)" ]
		then
			newId=$(curl -s -X POST -H "Content-type: application/json" \
				-d "{\"name\":\"$name\", \"linuxVersion\":\"$linuxVersion\"}" \
				--insecure https://$ip_address/api/main/machineRegister)
			printf "MachineId: %s\n" $newId > machineId
		else
			newId=$(cat machineId | grep MachineId | cut -d ':' -f2 | xargs)
		fi

		echo 'Machine id: '$newId$'\n'

		#registracija procesora
		cpuName=$(cat /proc/cpuinfo | grep 'model name' -m1 | cut -d ':' -f2 | xargs)
		if [ -z "$(cat machineId | grep CpuId)" ]
		then
			newCpuId=$(curl -s -X POST -H "Content-type: application/json" \
					-d "{\"name\":\"$cpuName\", \"machineId\":\"$newId\"}" \
			   		--insecure https://$ip_address/api/main/cpuRegister)
			printf "CpuId: %s\n" $newCpuId >> machineId
		else
			newCpuId=$(cat machineId | grep CpuId | cut -d ':' -f2 | xargs)
		fi

		echo 'CPU id: '$newCpuId$'\n'

		#registracija jezgri
		if [ -z "$(cat machineId | grep 'Cores registered')" ]
		then
			cpuCores=$(cat /proc/cpuinfo | grep -e 'processor' -e 'cpu MHz' -e 'cache size')
			numOfIterations=$(($(echo "$cpuCores" | wc -l) / 3))
			for ((i = 0 ; i < $numOfIterations ; i++)); do
				add=$((${i}*3))
				echo '-------------------------------------'
				coreNo=$(echo "$cpuCores" | sed -n $((1+$add))p | cut -d ':' -f2 | xargs)
				printf "CoreNo: %s\n" $coreNo
				cpuMhz=$(echo "$cpuCores" | sed -n $((2+$add))p | cut -d ':' -f2 | xargs)
				printf "Cpu MHz: %s\n" $cpuMhz
				cacheKb=$(echo "$cpuCores" | sed -n $((3+$add))p | cut -d ':' -f2 | xargs | cut -d ' ' -f1)
				printf "Cache KB: %s\n" $cacheKb

				coreId=$(curl -X POST -H "Content-type: application/json" \
					-d "{\"processorId\":\"$newCpuId\", \"speed\":\"$cpuMhz\", \"coreNo\":\"$coreNo\", \"cacheSizeKB\":\"$cacheKb\"}" \
					--insecure https://$ip_address/api/main/coreRegister)

				printf "Core ID: %s\n\n" $coreId
			done
			printf "Cores registered" >> machineId
		fi

		echo "$ip_address/api/main/$command"
		;;

	"processRegister")
		pid=$3
		if [ -z "$pid" ]
		then
			echo "Third argument should be PID!"
			exit
		fi

		machineId=$(cat machineId | grep MachineId | cut -d ' ' -f2) #mozda preko nekog filea pohraniti id koji vrati machineRegister?
		if [ -z "$machineId" ]
		then
			echo "MachineId not found!"
			exit
		fi

		processName=$(cat /proc/$pid/status | grep Name | cut -d$'\t' -f2)
		processPath=$(ps -p $pid -o cmd | cut -d$'\n' -f2)

		newProcessId=$(curl -s -X POST -H "Content-type: application/json" \
			-d "{\"processIdSystem\":\"$pid\", \"name\":\"$processName\", \"machineId\":\"$machineId\", \"processPath\":\"$processPath\"}" \
			--insecure https://$ip_address/api/main/processRegister)

		printf "%s: %s" $pid $newProcessId >> machineId

		echo $newProcessId$'\n'
		echo "$ip_address/api/main/$command"
		;;

	"processUpdate")
		pid=$3
		if [ -z "$pid" ]
		then
			echo "Third argument should be PID!"
			exit
		fi

		processId=$(cat machineId | grep $pid: | cut -d ' ' -f2)
		if [ -z "$processId" ]
		then
			echo "ProcessId not found!"
			exit
		fi

		state=$(cat /proc/$pid/status | grep State | cut -d$'\t' -f2)
		cpuUtil=$(ps -p $pid -o %cpu | cut -d$'\n' -f2 | xargs)
		memUtil=$(ps -p $pid -o %mem | cut -d$'\n' -f2 | xargs)
		threads=$(cat /proc/$pid/status | grep Threads | cut -d$'\t' -f2)

		echo $processId
		echo $state$'\n'$cpuUtil$'\n'$memUtil$'\n'$threads$'\n'

		updatedProcessId=$(curl -s -X POST -H "Content-type: application/json" \
			-d "{\"processId\":\"$processId\", \"state\":\"$state\", \"cpuUtil\":$cpuUtil, \"memUtil\":$memUtil, \"threads\":$threads}" \
			--insecure https://$ip_address/api/main/processUpdate)

		echo $updatedProcessId$'\n'
		echo "$ip_address/api/main/$command"
		;;
	*)
		echo "Unknown command!"
		;;
esac
