#!/bin/bash

ip_address=$1
command=$2

output_file_ids="/proc/statux_info"
output_file_log="/proc/statux_log"

case $command in
	"machineRegister")
		linuxVersion=$(cat /proc/version | cut -d ' ' -f3)

		#registracija mašine
		name=$(hostname)
		if [ -z "$(cat ${output_file_ids} | grep MachineId)" ]
		then
			echo "[machineRegister] Creating new machine!" >> "$output_file_log"

			newMachineId=$(curl -s -X POST -H "Content-type: application/json" \
				-d "{\"name\":\"$name\", \"linuxVersion\":\"$linuxVersion\"}" \
				--insecure https://$ip_address/api/main/machineRegister)
			printf "MachineId: %s\n" $newMachineId >> "$output_file_ids"
		else
			echo "[machineRegister] Machine ID already present!" >> "$output_file_log"

			newMachineId=$(cat "$output_file_ids" | grep MachineId | cut -d ':' -f2 | xargs)
		fi

		echo 'Machine id: '$newMachineId$'\n'

		#registracija procesora
		cpuName=$(cat /proc/cpuinfo | grep 'model name' -m1 | cut -d ':' -f2 | xargs)
		if [ -z "$(cat ${output_file_ids} | grep CpuId)" ]
		then
			echo "[machineRegister] Creating new CPU!" >> "$output_file_log"

			newCpuId=$(curl -s -X POST -H "Content-type: application/json" \
					-d "{\"name\":\"$cpuName\", \"machineId\":\"$newMachineId\"}" \
			   		--insecure https://$ip_address/api/main/cpuRegister)
			printf "CpuId: %s\n" $newCpuId >> "$output_file_ids"
		else
			echo "[machineRegister] CPU already present!" >> "$output_file_log"

			newCpuId=$(cat "$output_file_ids" | grep CpuId | cut -d ':' -f2 | xargs)
		fi

		echo 'CPU id: '$newCpuId$'\n'

		#registracija jezgri
		if [ -z "$(cat ${output_file_ids} | grep 'Cores registered')" ]
		then
			echo "[machineRegister] Creating cores!" >> "$output_file_log"

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
			printf "Cores registered\n" >> "$output_file_ids"
		fi

		#registracija memorije
		if [ -z "$(cat ${output_file_ids} | grep 'Memory registered')" ]
		then
			echo "[machineRegister] Creating memory!" >> "$output_file_log"

			totalPhysicalMemoryKb=$(cat /proc/meminfo | grep MemTotal | cut -d ':' -f2 | xargs | cut -d ' ' -f1 | xargs)
			totalSwapKb=$(cat /proc/meminfo | grep SwapTotal | cut -d ':' -f2 | xargs | cut -d ' ' -f1 | xargs)
			registeredMemoryMachineId=$(curl -X POST -H "Content-type: application/json" \
				-d "{\"machineId\":\"$newMachineId\", \"totalPhysicalMemoryKb\":\"$totalPhysicalMemoryKb\", \"totalSwapMemoryKb\":\"$totalSwapKb\"}" \
				--insecure https://$ip_address/api/main/memoryRegister)

			echo 'New memory registered with machine: '$registeredMemoryMachineId$'\n'

			printf "Memory registered\n" >> "$output_file_ids"
		fi

		echo "$ip_address/api/main/$command"
		;;

	"processRegister")
		pid=$3
		if [ -z "$pid" ]
		then
			echo 'PID not present!' >> "$output_file_log"
			echo "Third argument should be PID!"
			exit
		fi
		echo '[processRegister] Got process ID: '$pid >> "$output_file_log"

		machineId=$(cat ${output_file_ids} | grep MachineId | cut -d ' ' -f2)
		if [ -z "$machineId" ]
		then
			echo "MachineID not found!" >> "$output_file_log"
			echo "MachineId not found!"
			exit
		fi
		echo '[processRegister] Got Machine ID: '$machineId >> "$output_file_log"

		processName=$(cat /proc/$pid/status | grep Name | cut -d$'\t' -f2)
		processPath=$(ps -p $pid -o cmd | cut -d$'\n' -f2)

		newProcessId=$(curl -s -X POST -H "Content-type: application/json" \
			-d "{\"processIdSystem\":\"$pid\", \"name\":\"$processName\", \"machineId\":\"$machineId\", \"processPath\":\"$processPath\"}" \
			--insecure https://$ip_address/api/main/processRegister)

		printf "%s: %s\n" $pid $newProcessId >> "$output_file_ids"

		echo $newProcessId$'\n'
		echo "$ip_address/api/main/$command"
		;;

	"processUpdate")
		pid=$3
		if [ -z "$pid" ]
		then
			echo "[processUpdate] PID not found!" >> "$output_file_log"
			echo "Third argument should be PID!"
			exit
		fi

		processId=$(cat ${output_file_ids} | grep $pid: | cut -d ' ' -f2)
		if [ -z "$processId" ]
		then
			echo "[processUpdate] ProcessId not found!" >> "$output_file_log"
			echo "ProcessId not found!"
			exit
		fi

		state=$(cat /proc/$pid/status | grep State | cut -d$'\t' -f2)
		cpuUtil=$(ps -p $pid -o %cpu | cut -d$'\n' -f2 | xargs)
		memUtil=$(ps -p $pid -o %mem | cut -d$'\n' -f2 | xargs)
		threads=$(cat /proc/$pid/status | grep Threads | cut -d$'\t' -f2)

		echo '[processUpdate] Process ID: '$processId >> "$output_file_log"
		echo $state$'\n'$cpuUtil$'\n'$memUtil$'\n'$threads$'\n' >> "$output_file_log"

		updatedProcessId=$(curl -s -X POST -H "Content-type: application/json" \
			-d "{\"processId\":\"$processId\", \"state\":\"$state\", \"cpuUtil\":$cpuUtil, \"memUtil\":$memUtil, \"threads\":$threads}" \
			--insecure https://$ip_address/api/main/processUpdate)

		echo $updatedProcessId$'\n'
		echo "$ip_address/api/main/$command"
		;;

	"machineDelete")
		machineId=$(cat ${output_file_ids} | grep MachineId | cut -d ' ' -f2)
		if [ -z "$machineId" ]
		then
			echo "[machineDelete] MachineId not found!" >> "$output_file_log"
			echo "MachineId not found!"
			exit
		fi

		echo '[machineDelete] Deleting machine with id:'$machineId >> "$output_file_log"

		curl -s -X DELETE -H $ip_address/api/main/machineDelete/$machineId "Accept: application/json"

		echo '[machineDelete] Deleted machine with id: '$machineId >> "$output_file_log"
		echo "$ip_address/api/main/machineDelete/$machineId"
		;;

	*)
		echo "Unknown command!"
		;;
esac
