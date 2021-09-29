attribute vec3 v_pos;
uniform mat4 u_transform;
uniform sampler2D u_offset_tex;

void main() {
	ivec2 texel_pos = ivec2(gl_InstanceID & 1023, gl_InstanceID >> 10);
	vec3 offset = texelFetch(u_offset_tex, texel_pos, 0).xyz;
	gl_Position = u_transform * vec4(v_pos + offset, 1.0);
}
