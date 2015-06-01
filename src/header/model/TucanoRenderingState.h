#ifndef TUCANO_RENDERING_STATE_H
#define TUCANO_RENDERING_STATE_H

#include <tucano.hpp>
#include <phongshader.hpp>
#include <imgSpacePBR.hpp>
#include "RenderingState.h"
#include <utils/frustum.hpp>

using namespace Tucano;
using namespace Effects;

namespace model
{
	/** RenderingState using Tucano library ( @link http://terra.lcg.ufrj.br/tucano/ ). */
	class TucanoRenderingState
	: public RenderingState
	{
		using Box = AlignedBox< Float, 3 >;
	public:
		enum Effect
		{
			PHONG,
			JUMP_FLOODING
		};
		
		TucanoRenderingState( Camera* camera, Camera* lightCam , Mesh* mesh, const Attributes& attribs,
							  const string& shaderPath, const int& jfpbrFrameskip = 1, const Effect& effect = PHONG );
		
		~TucanoRenderingState();
		
		/** Updates the frustum after changes on camera. */
		void updateFrustum();
		
		virtual unsigned int render();
		
		virtual bool isCullable( const pair< Vec3, Vec3 >& box ) const;
		
		/** This implementation will compare the size of the maximum box diagonal in window coordinates with the projection
		 *	threshold.
		 *	@param projThresh is the threshold of the squared size of the maximum box diagonal in window coordinates. */
		virtual bool isRenderable( const pair< Vec3, Vec3 >& box, const Float& projThresh ) const;
	
		/** Gets the image space pbr effect. The caller is reponsable for the correct usage.*/
		ImgSpacePBR& getJumpFlooding() { return *m_jfpbr; }
		
		/** Gets the phong effect. The caller is reponsable for the correct usage.*/
		Phong& getPhong() { return *m_phong; }
		
		/** Changes the effect used to render the points. */
		void selectEffect( const Effect& effect ) { m_effect = effect; }
		
		void setJfpbrFrameskip( const int& value ) { m_jfpbrFrameskip = value; }
		
		void clearAttribs()
		{
			RenderingState::clearAttribs();
			m_mesh->reset();
		}
		
	protected:
		/** Acquires current traball's view-projection matrix. */
		Matrix4f getViewProjection() const;
		
		/** Projects the point in world coordinates to window coordinates. */
		Vector2f projToWindowCoords( const Vector4f& point, const Matrix4f& viewProjection, const Vector2i& viewportSize )
		const;
		
		Frustum* m_frustum;
		Camera* m_camera;
		Camera* m_lightCamera;
		
		Matrix4f m_viewProj;
		
		Mesh* m_mesh;
		Phong* m_phong;
		ImgSpacePBR *m_jfpbr;
		
		Effect m_effect;
		
		/** Frameskip for the Jump Flooding effect. */
		int m_jfpbrFrameskip;
		
		/** Frame counter. Used in order to skip frames properly. */
		unsigned int m_nFrames;
	};
}

#endif