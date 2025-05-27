#include "DemoScene.h"
#include "demo_utils.h"

namespace DQ
{
void BeginDrawing(Camera& camera);
void EndDrawing(bool toggleSkinning);
}


void DQ::DemoScene::Init()
{
	m_Camera = DQ::GetCamera();

	m_ResourceManager.LoadAllShaders();
	m_ResourceManager.LoadModel("models/gltf/pirate/pirate.glb");
}

void DQ::DemoScene::Update(UpdateContext const& context)
{
	if (IsKeyPressed(KEY_F)) ++m_ActiveShader %= ShaderTypes::TOTALTYPES;
	if (IsKeyPressed(KEY_T)) m_ActiveAnimation++;
	if (IsKeyPressed(KEY_G)) m_ActiveAnimation--;

	UpdateCamera(&m_Camera, CAMERA_FREE);
	//------------------------------------------------------------
	// FETCH SHADER AND MODEL DATA
	Shader shader						= m_ResourceManager.GetShaders().at(m_ActiveShader);
	CombinedModelData const& modeldata	= m_ResourceManager.GetModelData();
	//------------------------------------------------------------
	// UPDATE MODEL ANIMATION
	ModelAnimation& anim				= modeldata.pAnimations[m_ActiveAnimation % modeldata.animcount];
	int	frame							= context.frame % anim.frameCount;
	DQ::UpdateModelAnimationBones(modeldata.model, anim, frame);
	//------------------------------------------------------------
	// UPDATE SHADER 
	modeldata.model.materials[1].shader = shader;
	if (m_ActiveShader == ShaderTypes::DUALQUATERNIONBLENDSKINNING)
	{
		Model const& model = modeldata.model;
		int loc = GetShaderLocation(model.materials[1].shader, "boneDualQuaternions");
		SetShaderValueV(model.materials[1].shader, loc, model.meshes[0].boneMotors, SHADER_UNIFORM_VEC4, anim.boneCount * 2);
	}
	//------------------------------------------------------------

}

void DQ::DemoScene::Draw()
{
	DQ::BeginDrawing(m_Camera);

	{
		Model const& model = m_ResourceManager.GetModelData().model;

		DQ::DrawMesh(model.meshes[0], model.materials[1], model.transform);
	}

	DQ::EndDrawing(m_ActiveShader);

}

void DQ::DemoScene::Shutdown()
{
	m_ResourceManager.Shutdown();
}





namespace DQ
{
	void BeginDrawing(Camera& camera)
	{
		::BeginDrawing();

		ClearBackground(BLACK);

		BeginMode3D(camera);
	}

	void EndDrawing(bool toggleSkinning)
	{
		DrawGrid(10, 1.0f);

		EndMode3D();

		DrawText("Use the T/G to switch animation", 10, 10, 20, GRAY);
		DrawText("press F to toggle skinning mode", 10, 30, 20, GRAY);
		DrawText("press P to pause", 10, 50, 20, GRAY);
		if (!toggleSkinning)
		{
			DrawText("LINEAR BLEND SKINNING", 10, 70, 20, GREEN);
		}
		else
		{
			DrawText("DUAL QUAT BLEND SKINNING", 10, 70, 20, SKYBLUE);
		}

		::EndDrawing();
	}
}