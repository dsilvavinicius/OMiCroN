#ifndef SCAN_H
#define SCAN_H

#include <vector>
#include <memory>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

using namespace std;

namespace model
{
	/** Parallel Scan in GPU algorithm described in Eficient Parallel Scan Algorithms for GPUs
	 * ( http://mgarland.org/files/papers/nvr-2008-003.pdf ) */
	class Scan
	{
	public:
		Scan( const string& shaderFolder, const vector< unsigned int >& values );
		~Scan();
		
		/** Do the actual scan. */
		void doScan();
		
		/** Transfer the results back to the CPU and return a pointer for them. The transfer is costly, so this method
		 * should be used judiciously. Also, the results will be available only after doScan() is called. */
		shared_ptr< vector< unsigned int > > getResultCPU();
		
	private:
		enum BufferType
		{
			/** Original values to be scanned. */
			ORIGINAL,
			/** Scan result. Also used to save a temp per-block scan. */
			SCAN_RESULT,
			/** Global prefixes computed while scanning. */
			GLOBAL_PREFIXES,
			N_BUFFER_TYPES
		};
		
		enum ProgramType
		{
			/** 1st pass. */
			PER_BLOCK_SCAN,
			/** 2nd pass. */
			GLOBAL_SCAN,
			/** Last pass. */
			FINAL_SUM,
			N_PROGRAM_TYPES
		};
		
		static const int BLOCK_SIZE = 1024;
		
		/** Compute Shader programs used to do the passes of the algorithm. */
		QOpenGLShaderProgram* m_programs[ N_PROGRAM_TYPES ];
		
		/** Buffers used in the scan. */
		QOpenGLBuffer* m_buffers[ N_BUFFER_TYPES ];
		
		/** Number of blocks used to compute the scan. */
		int m_nBlocks;
		
		/** Number of input elements. */
		unsigned int m_nElements;
	};
}

#endif