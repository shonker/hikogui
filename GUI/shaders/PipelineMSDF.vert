#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    vec2 windowExtent;
    vec2 viewportScale;
    vec2 atlasExtent;
    vec2 atlasScale;
} pushConstants;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inClippingRectangleOffset;
layout(location = 2) in vec2 inClippingRectangleExtent;
layout(location = 3) in uvec3 inAtlasPosition;
layout(location = 4) in uint inDepth;
layout(location = 5) in vec4 inColor;

layout(location = 0) out vec2 outClippingRectangleMinimum;
layout(location = 1) out vec2 outClippingRectangleMaximum;
layout(location = 2) out vec3 outAtlasPosition;
layout(location = 3) out float outAlpha;

vec2 flipY(vec2 windowPosition) {
    return vec2(windowPosition.x, pushConstants.windowExtent.y - windowPosition.y);
}

vec2 convertToViewport(vec2 windowPosition) {
    return (windowPosition * pushConstants.viewportScale) - vec2(1.0, 1.0);
}

vec3 convertToTexture(vec3 atlasPosition) {
    return vec3(atlasPosition.xy * pushConstants.atlasScale, atlasPosition.z);
}

void main() {
    vec2 position = convertToViewport(flipY(inPosition));

    gl_Position = vec4(position, 0.0, 1.0);

    outClippingRectangleMinimum = flipY(inClippingRectangleOffset) - vec2(0.0, inClippingRectangleExtent.y);
    outClippingRectangleMaximum = flipY(inClippingRectangleOffset) + vec2(inClippingRectangleExtent.x, 0.0);
    outAtlasPosition = convertToTexture(inAtlasPosition);
    outAlpha = inColor.r;
}
