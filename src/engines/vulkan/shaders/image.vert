# version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texcoord;
layout (push_constant) uniform _pushConstants
{
    mat3 mvp;
    vec2 viewPort;
} pushConstants;

layout(location = 0) out vec2 fragTexCoord;

void main()
{
    vec3 tPosition = pushConstants.mvp * vec3(position.xy, 1.0);
    tPosition.xy = 2.0 * tPosition.xy / pushConstants.viewPort.xy - 1.0;
    gl_Position = vec4(tPosition.xy, 1.0, 1.0);
    fragTexCoord = texcoord;
}
