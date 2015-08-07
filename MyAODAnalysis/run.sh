
datetag=`date +%b%d`
inputlist="Runlist/test"
output="test_"`basename ${inputlist}`_${datetag}

readTruth -inputlist ${inputlist} -output ${output} 
