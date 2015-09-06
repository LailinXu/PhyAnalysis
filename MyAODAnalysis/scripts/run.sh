
datetag=`date +%b%d`
inputlist="Runlist/test_Zjets_Sherpa"
output="test_"`basename ${inputlist}`_${datetag}

readTruth -inputlist ${inputlist} -output ${output} 
