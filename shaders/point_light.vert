// Point light vertex shader
#version 450
#extension GL_KHR_vulkan_glsl : enable

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout (location = 0) out vec2  fragOffset;

struct PointLight {
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} ubo;

layout(push_constant) uniform Push {
    vec4 position;
    vec4 color;
    float radius;
} push;

void main() {    
    //vec3 lightPositionInCameraSpace = (ubo.view * vec4(ubo.lightPosition.xyz, 1.0)).xyz;
    //vec3 positionInCameraSpace = lightPositionInCameraSpace + LIGHT_RADIUS * vec3(fragOffset, 0.0);
    //gl_Position = ubo.projection * vec4(positionInCameraSpace, 1.0);
    fragOffset = OFFSETS[gl_VertexIndex];
    vec3 cameraRightWorld = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
    vec3 cameraUpWorld = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

    vec3 positionWorld = push.position.xyz
        + push.radius * fragOffset.x * cameraRightWorld
        + push.radius * fragOffset.y * cameraUpWorld;

    gl_Position = ubo.projection * ubo.view * vec4(positionWorld, 1.0);
}