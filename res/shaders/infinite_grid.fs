#version 330

in vec3 WorldPos;

layout(location = 0) out vec4 FragColor;

// Camera position in world space
uniform vec3  gCameraWorldPos;

// Base cell size for the grid
uniform float gGridCellSize   = 0.025;

// Distance over which to fade out the grid
uniform float gGridSize       = 1.1;

// Color of the grid lines (can be any color you like)
uniform vec4  gGridColorThick = vec4(0.5, 0.5, 0.5, 1.0);

// ---------------------------------------------------------
// Helper functions
// ---------------------------------------------------------
float log10(float x)
{
    return log(x) / log(10.0);
}

float satf(float x)
{
    return clamp(x, 0.0, 1.0);
}

vec2 satv(vec2 x)
{
    return clamp(x, vec2(0.0), vec2(1.0));
}

float max2(vec2 v)
{
    return max(v.x, v.y);
}

// ---------------------------------------------------------
// Main function
// ---------------------------------------------------------
void main()
{
    // Compute partial derivatives in x and z
    vec2 dvx = vec2(dFdx(WorldPos.x), dFdy(WorldPos.x));
    vec2 dvy = vec2(dFdx(WorldPos.z), dFdy(WorldPos.z));

    // Length of partials for line thickness
    float lx = length(dvx);
    float ly = length(dvy);
    vec2 dudv = vec2(lx, ly);

    // Adjust line thickness
    dudv *= 2.0;

    // Distance from the center of each cell, normalized by line thickness
    vec2 mod_div_dudv = mod(WorldPos.xz, gGridCellSize) / dudv;

    // How close we are to the grid line (0.0 -> away, 1.0 -> on the line)
    float Lod0a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)));

    // Start with the grid color and set alpha based on line proximity
    vec4 Color = gGridColorThick;
    Color.a *= Lod0a;

    // Compute 3D distance from the camera, then fade out
    float distFromCamera = length(WorldPos - gCameraWorldPos);
    float OpacityFalloff = 1.0 - satf(distFromCamera / gGridSize);
    Color.a *= OpacityFalloff;

    // Final fragment color
    FragColor = Color;
}
