#ifndef SCAN_H
#define SCAN_H

#include <vector>
#include <memory>
//#include <QOpenGLContext>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_4_3_Compatibility>

using namespace std;

namespace omicron
{
	/** Parallel Scan in GPU algorithm described in Eficient Parallel Scan Algorithms for GPUs
	 * ( http://mgarland.org/files/papers/nvr-2008-003.pdf ) */
	class Scan
	{
	public:
		enum BufferType
		{
			/** Original values to be scanned. */
			ORIGINAL,
			/** Scan result. Also used to save a temp per-block scan. */
			SCAN_RESULT,
			/** Global prefixes computed while scanning. */
			GLOBAL_PREFIXES,
			/** Receives the number of elements to be scanned, which is used as input to the 3rd pass shader. */
			N_ELEMENTS,
			/** Final reduction of the values in buffer ORIGINAL. */
			REDUCTION,
			N_BUFFER_TYPES
		};
		
		/** @param shaderFolder is the path to the folder with shaders.
		 * @param values are the values that should be scanned.
		 * @param openGL is the openGL functions wrapper ( assumed to be already initialized properly ).*/
		Scan( const string& shaderFolder, unsigned int nMaxElements, QOpenGLFunctions_4_3_Compatibility* openGL );
		~Scan();
		
		/** Do the actual scan in values.
		 * @returns The reduction of values, i.e. the parallel sum of its elements. */
		unsigned int doScan( const vector< unsigned int >& values );
		
		/** Transfer the results back to main memory and return a pointer for them. The transfer is costly, so this method
		 * should be used judiciously. Also, this method should be called after doScan() so the results are available. */
		vector< unsigned int > getResultCPU();
		 
		/** Dump the buffer with given buffer type to the stream for debug reasons. Transfers data from GPU to main memory,
		 * so it is costly.*/
		void dumpBuffer( const BufferType& bufferType, ostream& out );
		
		/** Returns the buffer id of one the buffers used to scan. */
		GLuint bufferId( const BufferType& bufferType ){ return m_buffers[ bufferType ]; }
		
	private:
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
		
		/** OpenGL Context. */
		QOpenGLFunctions_4_3_Compatibility* m_openGL;
		
		/** Compute Shader programs used to do the passes of the algorithm. */
		QOpenGLShaderProgram* m_programs[ N_PROGRAM_TYPES ];
		
		/** Buffers used in the scan. */
		
		GLuint m_buffers[ N_BUFFER_TYPES ];
		
		/** Number of input elements of current input values. */
		unsigned int m_nElements;
		
		/** Number of blocks necessary to launch to scan the current input values. */
		unsigned int m_nBlocks;
		
		/** Maximum number of elements ( used to allocate initial buffers ). */
		unsigned int m_nMaxElements;
	};
}

#endif
