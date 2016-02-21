testExample()
{
	currentTestIdx=0
	nThreads=8
	while [[ $nThreads -ge 2 ]]; do
		maxLvl=20
		while [[ $maxLvl -ge 5 ]]; do
			worklistSize=2048
			while [[ $worklistSize -ge 128 ]]; do
				memQuota=7516192768
				while [[ $memQuota -ge 2147483648 ]]; do
					if [[ currentTestIdx -ge $2 ]]
						then echo "Test " $currentTestIdx #" Params: file " $1 " threads " $nThreads " lvl " $maxLvl " workitem " $worklistSize " quota " $memQuota
						./Tests "--gtest_filter=*FastParallelOctreeStressTest*" --octree_stress_filename $1 --octree_stress_threads $nThreads --octree_stress_lvl $maxLvl --octree_stress_workitem $worklistSize --octree_stress_quota $memQuota
					fi
					let currentTestIdx=currentTestIdx+1
					let memQuota=memQuota-2147483648
				done
				let worklistSize=worklistSize/2
			done
			let maxLvl=maxLvl-5
		done
		let nThreads=nThreads-2
	done
}

#testExample ../../../src/data/real/tempietto_sub_tot.ply 193
testExample ../../../src/data/real/tempietto_all.ply 181