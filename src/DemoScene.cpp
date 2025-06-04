#include "DemoScene.h"
#include "demo_utils.h"
#include "rcamera.h"
#include "raymath.h"

#include <raygizmo.h>
#include "rlgl.h"

Transform clippingVolumeGizmoTransform = GizmoIdentity();


namespace DQ
{
	void BeginDrawing(Camera& camera);
	void EndDrawing(ShaderTypes activeShader, int activeAnimId, int animsCount);
	void DrawEllipsoidWires(Transform transform, int rings, int slices, Color color);

}

void DQ::DemoScene::Init()
{
	m_Camera = DQ::GetCamera();
	clippingVolumeGizmoTransform.translation = { 0,0.92,0 };
	clippingVolumeGizmoTransform.scale = { 0.15,0.20,0.15 };

	m_ResourceManager.LoadAllShaders();
	m_ResourceManager.LoadModel("models/gltf/pirate/pirate.glb");
	m_ResourceManager.LoadTextureSDF("textures/SDF/pirate_SDF_50U.exr");
	
	// Debug: Save the loaded image data to verify loading
	//m_ResourceManager.DebugSaveImageData("textures/SDF/pirate_SDF_50U.exr", "debug_pirate_sdf.png");

	Model& model = m_ResourceManager.GetModelData().model;
	model.materials[1].maps[MATERIAL_MAP_SDF].texture = m_ResourceManager.GetTextureSDF();
	
}

void DQ::DemoScene::Update(UpdateContext const& context)
{
	if (IsKeyPressed(KEY_F))			m_ActiveShader = (m_ActiveShader == ShaderTypes::LINEARBLENDSKINNING) ? ShaderTypes::DUALQUATERNIONBLENDSKINNING : ShaderTypes::LINEARBLENDSKINNING;
	if (IsKeyPressed(KEY_G))			m_ActiveShader = (m_ActiveShader == ShaderTypes::SKINNEDRAYMARCHING) ? ShaderTypes::STATICRAYMARCHING : ShaderTypes::SKINNEDRAYMARCHING;
	if (IsKeyPressed(KEY_T))			m_ActiveAnimation = std::min(++m_ActiveAnimation, m_ResourceManager.GetModelData().animcount - 1);
	if (IsKeyPressed(KEY_R))			m_ActiveAnimation = std::max(--m_ActiveAnimation, 0); 

	// Toggle camera controls
	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
	{
		if (IsCursorHidden()) EnableCursor();
		else DisableCursor();
	}

	if (IsCursorHidden()) UpdateCamera(&m_Camera, CAMERA_FREE);          // Update camera

	//------------------------------------------------------------
	// FETCH SHADER AND MODEL DATA
	Shader shader						= m_ResourceManager.GetShaders().at(m_ActiveShader);
	CombinedModelData const& modeldata	= m_ResourceManager.GetModelData();
	//------------------------------------------------------------
	// UPDATE MODEL ANIMATION

	ModelAnimation& anim				= modeldata.pAnimations[m_ActiveAnimation];
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
	else if (m_ActiveShader == ShaderTypes::STATICRAYMARCHING)
	{
		int loc = GetShaderLocation(shader, "clippingVolumePosition"); // this should not be done every frame, only once and cache
		SetShaderValueV(shader, loc, &clippingVolumeGizmoTransform.translation, SHADER_UNIFORM_VEC3, 1);

		loc		= GetShaderLocation(shader, "clippingVolumeScale"); // this should not be done every frame, only once and cache 
		SetShaderValueV(shader, loc, &clippingVolumeGizmoTransform.scale, SHADER_UNIFORM_VEC3, 1);

		loc		= GetShaderLocation(shader, "cameraPosition"); // this should not be done every frame, only once and cache 
		SetShaderValueV(shader, loc, &m_Camera.position, SHADER_UNIFORM_VEC3, 1);
		
		// SDF bounds for 50U pirate mesh (from box_bounds.txt)
		Vector3 minBounds{ -0.7175f, -0.0325f, -0.7275f };
		Vector3 maxBounds{ 0.7225f, 1.4075f, 0.7125f };
		// Set SDF bounds uniforms
		loc = GetShaderLocation(shader, "minBounds3DTextureSDF"); // this should not be done every frame, only once and cache 
		SetShaderValueV(shader, loc, &minBounds, SHADER_UNIFORM_VEC3, 1);
		loc = GetShaderLocation(shader, "maxBounds3DTextureSDF"); // this should not be done every frame, only once and cache 
		SetShaderValueV(shader, loc, &maxBounds, SHADER_UNIFORM_VEC3, 1);

		float time = GetTime();
		loc = GetShaderLocation(shader, "time"); // this should not be done every frame, only once and cache 
		SetShaderValueV(shader, loc, &time, SHADER_UNIFORM_FLOAT, 1);

	}
	else if(m_ActiveShader == ShaderTypes::SKINNEDRAYMARCHING)
	{
		int loc = GetShaderLocation(shader, "boneDualQuaternions"); // this should not be done every frame, only once and cache
		SetShaderValueV(shader, loc, model.meshes[0].boneMotors, SHADER_UNIFORM_VEC4, anim.boneCount * 2);

		loc = GetShaderLocation(shader, "clippingVolumePosition"); // this should not be done every frame, only once and cache
		SetShaderValueV(shader, loc, &clippingVolumeGizmoTransform.translation, SHADER_UNIFORM_VEC3, 1);

		loc = GetShaderLocation(shader, "clippingVolumeScale"); // this should not be done every frame, only once and cache 
		SetShaderValueV(shader, loc, &clippingVolumeGizmoTransform.scale, SHADER_UNIFORM_VEC3, 1);

		loc = GetShaderLocation(shader, "cameraPosition"); // this should not be done every frame, only once and cache 
		SetShaderValueV(shader, loc, &m_Camera.position, SHADER_UNIFORM_VEC3, 1);

		// SDF bounds for 50U pirate mesh (from box_bounds.txt)
		Vector3 minBounds{ -0.7175f, -0.0325f, -0.7275f };
		Vector3 maxBounds{ 0.7225f, 1.4075f, 0.7125f };
		// Set SDF bounds uniforms
		loc = GetShaderLocation(shader, "minBounds3DTextureSDF"); // this should not be done every frame, only once and cache 
		SetShaderValueV(shader, loc, &minBounds, SHADER_UNIFORM_VEC3, 1);
		loc = GetShaderLocation(shader, "maxBounds3DTextureSDF"); // this should not be done every frame, only once and cache 
		SetShaderValueV(shader, loc, &maxBounds, SHADER_UNIFORM_VEC3, 1);

		float time = GetTime();
		loc = GetShaderLocation(shader, "time"); // this should not be done every frame, only once and cache 
		SetShaderValueV(shader, loc, &time, SHADER_UNIFORM_FLOAT, 1);

	}
	//------------------------------------------------------------

}

void DQ::DemoScene::Draw()
{
	DQ::BeginDrawing(m_Camera);

	CombinedModelData const& modelData = m_ResourceManager.GetModelData();

	{
		Model const& model = modelData.model;

		DQ::DrawMesh(model.meshes[0], model.materials[1], model.transform);
	}
	{
		if (m_ActiveShader == ShaderTypes::STATICRAYMARCHING)
		{
			if (!IsCursorHidden())
			{
				DrawGizmo3D(GIZMO_TRANSLATE | GIZMO_SCALE, &clippingVolumeGizmoTransform);
				DrawEllipsoidWires(clippingVolumeGizmoTransform, 8, 8, GREEN);
			}
		}
	}

	DQ::EndDrawing(static_cast<ShaderTypes>(m_ActiveShader), m_ActiveAnimation+1, modelData.animcount);

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

	void EndDrawing(ShaderTypes activeShader, int activeAnimId, int animsCount)
	{
		DrawGrid(10, 1.0f);

		EndMode3D();

		DrawText(TextFormat("R/T to switch animation: %d/%d", activeAnimId, animsCount), 10, 10, 20, GRAY);
		DrawText("F to toggle skinning mode", 10, 32, 20, GRAY);
		DrawText("G to enable static raymarch shader", 10, 54, 20, GRAY);
		DrawText("C to pause", 10, 76, 20, GRAY);
		DrawText("RIGHT CLICK to toggle camera controls", 10, 98, 20, GRAY);

		float activeShaderTextHeight = GetScreenHeight() - 30;
		switch (activeShader)
		{
			case ShaderTypes::LINEARBLENDSKINNING:
				DrawText("LINEAR BLEND SKINNING", 10, activeShaderTextHeight, 30, GREEN);
				break;
			case ShaderTypes::DUALQUATERNIONBLENDSKINNING:
				DrawText("DUAL QUAT BLEND SKINNING", 10, activeShaderTextHeight, 30, SKYBLUE);
				break;
			case ShaderTypes::STATICRAYMARCHING:

				DrawText("RIGHT CLICK to toggle wound interaction", 10, activeShaderTextHeight - 25, 20, GOLD);
				DrawText("STATIC RAYMARCH", 10, activeShaderTextHeight, 30, ORANGE);
				break;
		}


		::EndDrawing();
	}



	// Draw ellipsoid wires given a Transform, ring/slice counts, and color
	// Transform encodes translation, rotation, and scale (where scale = ellipsoid radii)
	void DrawEllipsoidWires(Transform transform, int rings, int slices, Color color)
	{
		// Decompose transform
		Vector3 translation = transform.translation;
		Quaternion rotation = transform.rotation;
		Vector3 scale = transform.scale;

		// Optional: convert quaternion to axis-angle for rlRotatef
		Vector3 axis;
		float angle;
		QuaternionToAxisAngle(rotation, &axis, &angle);

		rlPushMatrix();
		// Apply scale -> rotate -> translate (inverse of matrix multiplication order)
		rlTranslatef(translation.x, translation.y, translation.z);
		rlRotatef(RAD2DEG * angle, axis.x, axis.y, axis.z);
		rlScalef(scale.x, scale.y, scale.z);

		rlBegin(RL_LINES);
		rlColor4ub(color.r, color.g, color.b, color.a);

		float ringAngle = DEG2RAD * (180.0f / (rings + 1));
		float sliceAngle = DEG2RAD * (360.0f / slices);
		float cosRing = cosf(ringAngle);
		float sinRing = sinf(ringAngle);
		float cosSlice = cosf(sliceAngle);
		float sinSlice = sinf(sliceAngle);

		Vector3 v2 = { 0.0f, 1.0f,      0.0f };
		Vector3 v3 = { sinRing, cosRing, 0.0f };
		Vector3 v0, v1;

		for (int i = 0; i < rings + 2; i++)
		{
			for (int j = 0; j < slices; j++)
			{
				v0 = v2; v1 = v3;

				// Rotate around Y axis
				Vector3 tmp = v2;
				v2.x = cosSlice * tmp.x - sinSlice * tmp.z;
				v2.y = tmp.y;
				v2.z = sinSlice * tmp.x + cosSlice * tmp.z;

				tmp = v3;
				v3.x = cosSlice * tmp.x - sinSlice * tmp.z;
				v3.y = tmp.y;
				v3.z = sinSlice * tmp.x + cosSlice * tmp.z;

				// Draw wires
				rlVertex3f(v0.x, v0.y, v0.z); rlVertex3f(v1.x, v1.y, v1.z);
				rlVertex3f(v1.x, v1.y, v1.z); rlVertex3f(v3.x, v3.y, v3.z);
				rlVertex3f(v0.x, v0.y, v0.z); rlVertex3f(v2.x, v2.y, v2.z);
			}
			// Step latitude (rotate around Z axis)
			Vector3 start = v3;
			v2 = start;
			v3.x = cosRing * start.x + sinRing * start.y;
			v3.y = -sinRing * start.x + cosRing * start.y;
			v3.z = start.z;
		}
		rlEnd();
		rlPopMatrix();
	}
}


