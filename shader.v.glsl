#version 120

// world posotion
varying vec3 EPos;
// texture coordinates
varying vec2 Coord;

void main()
{
    // position in world coordinates
    vec4 pos = gl_ModelViewMatrix * gl_Vertex;
    EPos = pos.xyz;
    gl_Position = gl_ProjectionMatrix * pos;
    Coord = gl_MultiTexCoord0.st;
}

