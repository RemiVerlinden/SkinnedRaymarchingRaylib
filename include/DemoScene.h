#ifndef DEMOSCENE_H
#define DEMOSCENE_H

#include "ResourceManager.h"
#include "structs.h"

namespace DQ {

	class DemoScene final
	{
	public:
		void Init();
		void Update(UpdateContext const& context);
		void Draw();
		void Shutdown();
	private:
		ResourceManager m_ResourceManager;
		Camera m_Camera;

		int m_ActiveShader;
		int m_ActiveAnimation;
		int m_RenderingMode; // 0=shaded, 1=checker debug, 2=ray heatmap
	};
}
#endif // !DEMOSCENE_H

