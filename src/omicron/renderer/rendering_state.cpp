#include "omicron/renderer/rendering_state.h"

namespace omicron::renderer
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
