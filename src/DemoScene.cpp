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
	void EndDrawing(ShaderTypes activeShader);
	void DrawEllipsoidWires(Transform transform, int rings, int slices, Color color);

}

void DQ::DemoScene::Init()
{
	m_Camera = DQ::GetCamera();
	clippingVolumeGizmoTransform.translation = { 0,1,0 };
	clippingVolumeGizmoTransform.scale = { 0.10,0.15,0.20 };

	m_ResourceManager.LoadAllShaders();
	m_ResourceManager.LoadModel("models/gltf/pirate/pirate.glb");
	m_ResourceManager.LoadTextureSDF("textures/SDF/pirate_SDF_50U.exr");
}

void DQ::DemoScene::Update(UpdateContext const& context)
{
	if (IsKeyPressed(KEY_F))			++m_ActiveShader %= ShaderTypes::TOTALTYPES;
	if (IsKeyPressed(KEY_T))			m_ActiveAnimation++;
	if (IsKeyPressed(KEY_G))			m_ActiveAnimation--;

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
		int loc = GetShaderLocation(shader, "clippingVolumePosition"); // this should not be done every frame, only once and cache
		SetShaderValueV(shader, loc, &clippingVolumeGizmoTransform.translation, SHADER_UNIFORM_VEC3, 1);

		loc		= GetShaderLocation(shader, "clippingVolumeScale"); // this should not be done every frame, only once and cache 
		SetShaderValueV(shader, loc, &clippingVolumeGizmoTransform.scale, SHADER_UNIFORM_VEC3, 1);

		loc		= GetShaderLocation(shader, "cameraPosition"); // this should not be done every frame, only once and cache 
		SetShaderValueV(shader, loc, &m_Camera.position, SHADER_UNIFORM_VEC3, 1);

		Texture bindPoseSDF = m_ResourceManager.GetTextureSDF();
		loc = GetShaderLocation(shader, "bindPoseSDF"); // this should not be done every frame, only once and cache 
		//SetShaderValueV(shader, loc, &m_Camera.position, SHADER_UNIFORM_VEC3, 1);
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
	{
		if (m_ActiveShader == ShaderTypes::RAYMARCHING)
		{
			if (!IsCursorHidden())
			{
				DrawGizmo3D(GIZMO_TRANSLATE | GIZMO_SCALE, &clippingVolumeGizmoTransform);
				DrawEllipsoidWires(clippingVolumeGizmoTransform, 8, 8, GREEN);
			}
		}
	}

	DQ::EndDrawing(static_cast<ShaderTypes>(m_ActiveShader));

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

	void EndDrawing(ShaderTypes activeShader)
	{
		DrawGrid(10, 1.0f);

		EndMode3D();

		DrawText("T/G to switch animation", 10, 10, 20, GRAY);
		DrawText("F to toggle skinning mode", 10, 32, 20, GRAY);
		DrawText("P to pause", 10, 54, 20, GRAY);
		DrawText("RIGHT CLICK to toggle camera controls", 10, 76, 20, GRAY);

		switch (activeShader)
		{
			case ShaderTypes::LINEARBLENDSKINNING:
				DrawText("LINEAR BLEND SKINNING", 10, 110, 20, GREEN);
				break;
			case ShaderTypes::DUALQUATERNIONBLENDSKINNING:
				DrawText("DUAL QUAT BLEND SKINNING", 10, 110, 20, SKYBLUE);
				break;
			case ShaderTypes::RAYMARCHING:
				DrawText("STATIC RAYMARCH", 10, 110, 20, ORANGE);
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


