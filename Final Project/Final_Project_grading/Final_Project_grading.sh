#!/bin/bash
homeworkName=Final_Project
binaryName=cell_move_router
testcasePool="case1 case2 case3 case4 case5 case6 case3B case4B case5B case6B"
timeLimit=3600

root=$( pwd )
groupDir=$root/group
outputDir=$root/output
testcasesDir=$root/testcases
verifierBin=$root/verifier/evaluator
chmod 500 $verifierBin
csvFile=$root/${homeworkName}_grade.csv

function executionCmd () 
{
	local argv="$testcasesDir/$1.txt $outputDir/$1.txt"
	local log=$( timeout $timeLimit time -p ./$binaryName $argv 2>&1 > /dev/null )
	if [[ $log =~ "real " ]] && [[ $log =~ "user " ]] && [[ $log =~ "sys " ]]; then
		echo "$( echo "$log" | grep real | tail -1 | cut -d ' ' -f 2 )"
	else
		echo "TLE"
	fi
}

function verifyCmd ()
{
	local argv="$testcasesDir/$1.txt $outputDir/$1.txt"
	local log=$( $verifierBin $argv 2>&1 | cat )
	if [[ $log =~ "[Error!]" ]]; then
		echo "$1 has something wrong."
	else
		echo "$( echo "$log" | grep "Final score" | cut -d '=' -f 2 )"
	fi
}

echo "|----------------------------------------------------------|"
echo "|                                                          |"
echo "|    This script is used for PDA $homeworkName grading.    |"
echo "|                                                          |"
echo "|----------------------------------------------------------|"

csvTitle="group name"
for testcase in $testcasePool; do
	csvTitle="$csvTitle, $testcase score, $testcase runtime"
done
echo "$csvTitle, status" > $csvFile

cd $groupDir/
for groupName in *; do
	if [[ -d $groupName ]]; then 
		printf "grading on %s:\n" $groupName
		cd $groupName/

		rm -rf $( find . -mindepth 1 -maxdepth 1 -type d )
		tar -zxvf CS6135_${homeworkName}.tar.gz > /dev/null
		cd $homeworkName/

		cd src/
		make clean > /dev/null
		rm -rf ./bin/*
		make > /dev/null
		cd ..

		rm -rf $outputDir/*
		cd bin/
		csvContent="$groupName"
		statusList=""
		printf "%10s | %15s | %10s | %s\n" testcase score runtime status
		for testcase in $testcasePool; do
			printf "%10s | " $testcase

			score=fail
			status="$testcase failed."
			runtime=$( executionCmd $testcase )
			if [[ $runtime != "TLE" ]]; then
				status=$( verifyCmd $testcase )

				if [[ $status =~ "wrong." ]]; then
					statusList="$statusList $status"
				else
					score=$status
					status="success"
				fi
			else
				statusList="$statusList $status"
			fi

			printf "%15s | %10s | %s\n" $score $runtime "$status"
			csvContent="$csvContent, $score, $runtime"
		done
		cd ..

		echo "$csvContent,$statusList" >> $csvFile
		cd $groupDir/
	fi
done

echo "|---------------------------------------------------------------|"
echo "|                                                               |"
echo "|    Successfully generate grades to ${homeworkName}_grade.csv    |"
echo "|                                                               |"
echo "|---------------------------------------------------------------|"
