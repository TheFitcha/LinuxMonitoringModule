#!/bin/bash

ip_address=$1
command=$2

output_file_ids="/proc/statux_info"
output_file_log="/proc/statux_log"

case $command in
	"machineRegister")
		linuxVersion=$(cat /proc/version | cut -d ' ' -f3)

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

				coreId=$(curl -s -X POST -H "Content-type: application/json" \
					-d "{\"processorId\":\"$newCpuId\", \"speed\":\"$cpuMhz\", \"coreNo\":\"$coreNo\", \"cacheSizeKB\":\"$cacheKb\"}" \
					--insecure https://$ip_address/api/main/coreRegister)

				printf "Core ID: %s\n\n" $coreId
			done
			printf "Cores registered\n" >> "$output_file_ids"
		fi



		if [ -z "$(cat ${output_file_ids} | grep 'Memory registered')" ]
		then
			echo "[machineRegister] Creating memory!" >> "$output_file_log"

			totalPhysicalMemoryKb=$(cat /proc/meminfo | grep MemTotal | cut -d ':' -f2 | xargs | cut -d ' ' -f1 | xargs)
			totalSwapKb=$(cat /proc/meminfo | grep SwapTotal | cut -d ':' -f2 | xargs | cut -d ' ' -f1 | xargs)
			freePhysicalMemoryKb=$(cat /proc/meminfo | grep MemFree | cut -d ':' -f2 | xargs | cut -d ' ' -f1 | xargs)
			freeSwapMemoryKb=$(cat /proc/meminfo | grep SwapFree | cut -d ':' -f2 | xargs | cut -d ' ' -f1 | xargs)

			registeredMemoryMachineId=$(curl -s -X POST -H "Content-type: application/json" \
				-d "{\"machineId\":\"$newMachineId\", \"totalPhysicalMemoryKb\":\"$totalPhysicalMemoryKb\", \"totalSwapMemoryKb\":\"$totalSwapKb\", \"freePhysicalMemoryKb\":\"$freePhysicalMemoryKb\", \"freeSwapMemoryKb\":\"$freeSwapMemoryKb\"}" \
				--insecure https://$ip_address/api/main/memoryRegister)

			echo 'New memory registered with machine: '$registeredMemoryMachineId$'\n'

			printf "Memory registered\n" >> "$output_file_ids"
		fi

		echo "$ip_address/api/main/$command"
		;;

	"memoryUpdate")
		machineId=$(cat ${output_file_ids} | grep MachineId | cut -d ' ' -f2)
		if [ -z "$machineId" ]
		then
			echo "MachineID not found!" >> "$output_file_log"
			echo "MachineID not found!"
			exit
		fi

		echo '[memoryUpdate] Updating memory for machineId: '$machineId >> "$output_file_log"

		totalPhysicalMemoryKb=$(cat /proc/meminfo | grep MemTotal | cut -d ':' -f2 | xargs | cut -d ' ' -f1 | xargs)
		totalSwapKb=$(cat /proc/meminfo | grep SwapTotal | cut -d ':' -f2 | xargs | cut -d ' ' -f1 | xargs)
		freePhysicalMemoryKb=$(cat /proc/meminfo | grep MemFree | cut -d ':' -f2 | xargs | cut -d ' ' -f1 | xargs)
		freeSwapMemoryKb=$(cat /proc/meminfo | grep SwapFree | cut -d ':' -f2 | xargs | cut -d ' ' -f1 | xargs)

		updatedMemoryMachineId=$(curl -s -X PUT -H "Content-type: application/json" \
			-d "{\"machineId\":\"$machineId\", \"totalPhysicalMemoryKb\":\"$totalPhysicalMemoryKb\", \"totalSwapMemoryKb\":\"$totalSwapKb\", \"freePhysicalMemoryKb\":\"$freePhysicalMemoryKb\", \"freeSwapMemoryKb\":\"$freeSwapMemoryKb\"}" \
			--insecure https://$ip_address/api/main/memoryUpdate)

		echo 'New memory updated for machine: '$updatedMemoryMachineId$'\n'
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
			echo "clear_proc_contents" >> "$output_file_ids"
			echo "[machineDelete] MachineId not found!" >> "$output_file_log"
			echo "MachineId not found!"
			exit
		fi

		echo '[machineDelete] Deleting machine with id:'$machineId >> "$output_file_log"

		returnId=$(curl -X DELETE --insecure https://$ip_address/api/main/machineDelete/$machineId -H "Accept: application/json")

		printf 'clear_proc_contents' >> "$output_file_ids"

		echo '[machineDelete] Deleted machine with id: '$machineId >> "$output_file_log"
		echo "$ip_address/api/main/machineDelete/$machineId"
		;;

	*)
		echo "Unknown command!"
		;;
esac
