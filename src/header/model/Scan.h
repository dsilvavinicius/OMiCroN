#ifndef SCAN_H
#define SCAN_H

#include <vector>
#include <memory>
#include <QOpenGLContext>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_4_3_Compatibility>

using namespace std;

namespace model
{
	/** Parallel Scan in GPU algorithm described in Eficient Parallel Scan Algorithms for GPUs
	 * ( http://mgarland.org/files/papers/nvr-2008-003.pdf ) */
	class Scan
	{
	public:
		/** @param shaderFolder is the path to the folder with shaders.
		 * @param values are the values that should be scanned.
		 * @param openGL is the openGL functions wrapper ( assumed to be already initialized properly ).*/
		Scan( const string& shaderFolder, const unsigned int* values, unsigned int nValues,
			  QOpenGLFunctions_4_3_Compatibility* openGL );
		~Scan();
		
		/** Do the actual scan. */
		void doScan();
		
		/** Transfer the results back to the CPU and return a pointer for them. The transfer is costly, so this method
		 * should be used judiciously. Also, the results will be available only after doScan() is called. */
		vector< unsigned int > getResultCPU();
		
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
		
		/** OpenGL Context. */
		QOpenGLFunctions_4_3_Compatibility* m_openGL;
		
		/** Compute Shader programs used to do the passes of the algorithm. */
		QOpenGLShaderProgram* m_programs[ N_PROGRAM_TYPES ];
		
		/** Buffers used in the scan. */
		
		GLuint m_buffers[ N_BUFFER_TYPES ];
		
		/** Number of blocks used to compute the scan. */
		int m_nBlocks;
		
		/** Number of input elements. */
		unsigned int m_nElements;
	};
}

#endif