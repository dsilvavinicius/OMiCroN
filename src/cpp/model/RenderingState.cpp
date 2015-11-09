#include "RenderingState.h"

namespace model
{
	RenderingState::RenderingState() {}
	
	void RenderingState::clearAttribs()
	{
		m_positions.clear();
		m_colors.clear();
		m_normals.clear();
		m_indices.clear();
	}
	
	void RenderingState::clearIndices()
	{
		m_indices.clear();
	}
}