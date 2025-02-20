#version 330

out vec3 WorldPos;
uniform mat4 gVP = mat4(1.0);
uniform vec3 gCameraWorldPos;

const vec3 Pos[4] = vec3[4](
	vec3(-1.0, 0.0, -1.0), //bottom left
	vec3(1.0, 0.0, -1.0), //bottom right
	vec3(1.0, 0.0, 1.0), //top right
	vec3(-1.0, 0.0, 1.0)
);

const int Indices[6] = int[6](
	0, 1, 2,
	0, 2, 3
);

void main()
{
	int Index = Indices[gl_VertexID];

	vec3 vPos3 = Pos[Index];
	vPos3.x += gCameraWorldPos.x;
	vPos3.y = 2.0f;
	vPos3.z += gCameraWorldPos.z;

	vec4 vPos4 = vec4(vPos3, 1.0);

	gl_Position = gVP * vPos4;
	WorldPos = vPos3;
}