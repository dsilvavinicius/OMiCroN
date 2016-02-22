FILE=FastParallelOctreeStressTest.log

while read line; do
	for token in "${line[@]}"; do
		echo $token
	done
done < $FILE