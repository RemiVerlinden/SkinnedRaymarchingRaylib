#include "DemoScene.h"
#include "demo_utils.h"
#include "rcamera.h"
#include "raymath.h"

#include <raygizmo.h>
#include "rlgl.h"

Transform clippingVolumeGizmoTransform = GizmoIdentity();
Transform shaderLightGizmoTransform = GizmoIdentity();


namespace DQ
{
	void BeginDrawing(Camera& camera);
	void EndDrawing(ShaderTypes activeShader, int activeAnimId, int animsCount, int renderingMode = 0, const char* animationName = "");
	void DrawEllipsoidWires(Transform transform, int rings, int slices, Color color);

}

void DQ::DemoScene::Init()
{
	m_Camera = DQ::GetCamera();
	clippingVolumeGizmoTransform.translation = { 0,0.92,0 };
	clippingVolumeGizmoTransform.scale = { 0.15,0.20,0.15 };

	shaderLightGizmoTransform.translation = {0,1,-1};
	
	// Initialize rendering mode to shaded
	m_RenderingMode = 0;

	m_ResourceManager.LoadAllShaders();
	m_ResourceManager.LoadModel("models/gltf/pirate/pirate.glb");
	m_ResourceManager.LoadTextureSDF("textures/SDF/pirate_SDF_50U.exr");
	m_ResourceManager.LoadTextureWeight("textures/weight/Pirate_4ChannelWeightsN_289.exr", "textures/weight/Pirate_4ChannelWeightIndex_289.exr");
	
	// Debug: Save the loaded image data to verify loading
	//m_ResourceManager.DebugSaveImageData("textures/SDF/pirate_SDF_50U.exr", "debug_pirate_sdf.png");

	Model& model = m_ResourceManager.GetModelData().model;
	model.materials[1].maps[MATERIAL_MAP_SDF].texture = m_ResourceManager.GetTextureSDF();
	model.materials[1].maps[MATERIAL_MAP_BONEWEIGHT].texture = m_ResourceManager.GetTextureBoneWeight();
	model.materials[1].maps[MATERIAL_MAP_BONEINDEX].texture = m_ResourceManager.GetTextureBoneIndex();
	
}

void DQ::DemoScene::Update(UpdateContext const& context)
{
	// Shader selection with number keys 1-5
	if (IsKeyPressed(KEY_ONE))			m_ActiveShader = ShaderTypes::LINEARBLENDSKINNING;
	if (IsKeyPressed(KEY_TWO))			m_ActiveShader = ShaderTypes::DUALQUATERNIONBLENDSKINNING;
	if (IsKeyPressed(KEY_THREE))		m_ActiveShader = ShaderTypes::STATICRAYMARCHING;
	if (IsKeyPressed(KEY_FOUR))			m_ActiveShader = ShaderTypes::SKINNEDRAYMARCHING;
	if (IsKeyPressed(KEY_FIVE))			m_ActiveShader = ShaderTypes::SKINNEDRAYMARCHINGNONLINEAR;
	
	if (IsKeyPressed(KEY_T))			m_ActiveAnimation = std::min(++m_ActiveAnimation, m_ResourceManager.GetModelData().animcount - 1);
	if (IsKeyPressed(KEY_R))			m_ActiveAnimation = std::max(--m_ActiveAnimation, 0);
	
	// Toggle rendering mode (only for non-linear raymarching)
	if (IsKeyPressed(KEY_J) && m_ActiveShader == ShaderTypes::SKINNEDRAYMARCHINGNONLINEAR) {
		m_RenderingMode = (m_RenderingMode + 1) % 3; // Cycle through 0, 1, 2
	} 

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
	// Use time-based animation at 60 FPS equivalent
	int	frame							= ((int)context.animationTime) % anim.frameCount;
	DQ::UpdateModelAnimationBones(modeldata.model, anim, frame);
	//------------------------------------------------------------
	// UPDATE SHADER 
	modeldata.model.materials[1].shader = shader;
	Model const& model					= modeldata.model;
	switch (m_ActiveShader)
	{
		case ShaderTypes::DUALQUATERNIONBLENDSKINNING:
		{
			int loc = GetShaderLocation(shader, "boneDualQuaternions"); // this should not be done every frame, only once and cache
			SetShaderValueV(shader, loc, model.meshes[0].boneMotors, SHADER_UNIFORM_VEC4, anim.boneCount * 2);
			break;
		}
		case ShaderTypes::STATICRAYMARCHING:
		{
			int loc = GetShaderLocation(shader, "clippingVolumePosition"); // this should not be done every frame, only once and cache
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
			break;
		}
		case ShaderTypes::SKINNEDRAYMARCHINGNONLINEAR:
		{
			int loc = GetShaderLocation(shader, "ellipsoidLightPosition"); // this should not be done every frame, only once and cache
			SetShaderValueV(shader, loc, &shaderLightGizmoTransform.translation, SHADER_UNIFORM_VEC3, 1);
			
			// Set rendering mode uniform
			loc = GetShaderLocation(shader, "renderingMode");
			SetShaderValue(shader, loc, &m_RenderingMode, SHADER_UNIFORM_INT);
			//break; Dont break, apply code from SKINNEDRAYMARCHING aswell
		}
		case ShaderTypes::SKINNEDRAYMARCHING:
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
			break;
		}
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
		if (m_ActiveShader >= ShaderTypes::STATICRAYMARCHING)
		{
			if (!IsCursorHidden())
			{
				DrawGizmo3D(GIZMO_TRANSLATE | GIZMO_SCALE, &clippingVolumeGizmoTransform);
				DrawEllipsoidWires(clippingVolumeGizmoTransform, 8, 8, GREEN);

				if(m_ActiveShader == ShaderTypes::SKINNEDRAYMARCHINGNONLINEAR)
					DrawGizmo3D(GIZMO_TRANSLATE , &shaderLightGizmoTransform);
			}
		}
	}

	ModelAnimation& anim = modelData.pAnimations[m_ActiveAnimation];
	DQ::EndDrawing(static_cast<ShaderTypes>(m_ActiveShader), m_ActiveAnimation+1, modelData.animcount, m_RenderingMode, anim.name);

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

	void EndDrawing(ShaderTypes activeShader, int activeAnimId, int animsCount, int renderingMode, const char* animationName)
	{
		DrawGrid(10, 1.0f);

		EndMode3D();

		// Animation name in top right corner
		if (animationName && strlen(animationName) > 0) {
			int textWidth = MeasureText(animationName, 16);
			DrawText(animationName, GetScreenWidth() - textWidth - 10, 10, 16, LIGHTGRAY);
		}

		DrawText(TextFormat("R/T to switch animation: %d/%d", activeAnimId, animsCount), 10, 10, 20, GRAY);
		DrawText(TextFormat("1-5 to switch shader: %d/5", (int)activeShader + 1), 10, 32, 20, GRAY);
		DrawText("C to pause, X to reset animation", 10, 54, 20, GRAY);
		DrawText("RIGHT CLICK to toggle camera controls", 10, 76, 20, GRAY);

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
			case ShaderTypes::SKINNEDRAYMARCHING:

				DrawText("RIGHT CLICK to toggle wound interaction", 10, activeShaderTextHeight - 25, 20, RED);
				DrawText("SKINNED RAYMARCH", 10, activeShaderTextHeight, 30, RED);
				break;
			case ShaderTypes::SKINNEDRAYMARCHINGNONLINEAR:
			{
				DrawText("RIGHT CLICK to toggle wound interaction", 10, activeShaderTextHeight - 50, 20, PURPLE);
				DrawText("J to toggle rendering mode", 10, activeShaderTextHeight - 25, 20, PURPLE);
				
				// Display current rendering mode
				const char* modeNames[] = {"SHADED", "CHECKER DEBUG", "RAY HEATMAP"};
				DrawText(TextFormat("Mode: %s", modeNames[renderingMode]), 10, activeShaderTextHeight - 75, 20, PURPLE);
				
				DrawText("SKINNED RAYMARCH NON-LINEAR", 10, activeShaderTextHeight, 30, PURPLE);
				break;
			}
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


