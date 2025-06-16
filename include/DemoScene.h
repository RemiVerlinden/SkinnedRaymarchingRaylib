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
		void Draw(UpdateContext const& context);
		void Shutdown();
	private:
		ResourceManager m_ResourceManager;
		Camera m_Camera;


		typedef enum
		{
			DrawWireframe = 1 << 0, // 1
			DrawBones = 1 << 1, // 2
			//Flag3 = 1 << 2, // 4
			//Flag4 = 1 << 3, // 8
			//Flag5 = 1 << 4, // 16
			//Flag6 = 1 << 5, // 32
			//Flag7 = 1 << 6, // 64
			//Flag8 = 1 << 7  //128
		}DemoFlags;

		int m_ActiveShader;
		int m_ActiveAnimation;
		int m_RenderingMode; // 0=shaded, 1=checker debug, 2=ray heatmap
		int m_Flags;
	};
}
#endif // !DEMOSCENE_H

