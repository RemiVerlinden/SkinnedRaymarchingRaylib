#include "DemoScene.h"
#include "demo_utils.h"
#include "rcamera.h"
#include "raymath.h"


namespace DQ
{
void BeginDrawing(Camera& camera);
void EndDrawing(bool toggleSkinning);
Vector3 GetClippingvolumePosition(Camera& camera);

}


void DQ::DemoScene::Init()
{
	m_Camera = DQ::GetCamera();

	m_ResourceManager.LoadAllShaders();
	m_ResourceManager.LoadModel("models/gltf/pirate/pirate.glb");
}

void DQ::DemoScene::Update(UpdateContext const& context)
{
	if (IsKeyPressed(KEY_F))			++m_ActiveShader %= ShaderTypes::TOTALTYPES;
	if (IsKeyPressed(KEY_T))			m_ActiveAnimation++;
	if (IsKeyPressed(KEY_G))			m_ActiveAnimation--;


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
	Model const& model					= modeldata.model;
	if (m_ActiveShader == ShaderTypes::DUALQUATERNIONBLENDSKINNING)
	{
		int loc							= GetShaderLocation(shader, "boneDualQuaternions"); // this should not be done every frame, only once and cache
		SetShaderValueV(shader, loc, model.meshes[0].boneMotors, SHADER_UNIFORM_VEC4, anim.boneCount * 2);
	}
	else if (m_ActiveShader == ShaderTypes::RAYMARCHING)
	{
		Vector3 clippingVolumePosition = GetClippingvolumePosition(m_Camera);
		int loc	= GetShaderLocation(shader, "clippingVolumePosition"); // this should not be done every frame, only once and cache
		SetShaderValueV(shader, loc, &clippingVolumePosition, SHADER_UNIFORM_VEC3, 1);

		
		Vector3 clippingVolumeScale{0.05,0.05,0.15 };
		loc		= GetShaderLocation(shader, "clippingVolumeScale"); // this should not be done every frame, only once and cache 
		SetShaderValueV(shader, loc, &clippingVolumeScale, SHADER_UNIFORM_VEC3, 1);

		loc = GetShaderLocation(shader, "cameraPosition"); // this should not be done every frame, only once and cache 
		SetShaderValueV(shader, loc, &m_Camera.position, SHADER_UNIFORM_VEC3, 1);
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

	Vector3 GetClippingvolumePosition(Camera& camera)
	{
		static Vector3 clippingVolumePosition{ 0, 1, 0 };
		Vector3 wishDir{};
		if (IsKeyDown(KEY_I))  wishDir += GetCameraForward(&camera);
		if (IsKeyDown(KEY_L)) wishDir += GetCameraRight(&camera);
		if (IsKeyDown(KEY_K)) wishDir -= GetCameraForward(&camera);
		if (IsKeyDown(KEY_J)) wishDir -= GetCameraRight(&camera);

		const float MOVESPEED = 1.f;
		clippingVolumePosition += wishDir * MOVESPEED * GetFrameTime();

		return clippingVolumePosition;
	}
}