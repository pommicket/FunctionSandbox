attribute vec3 v_pos;
attribute vec3 v_normal;
uniform mat4 u_transform;
uniform vec3 u_offset;
uniform vec3 u_scale;

varying vec3 normal;

void main() {
	gl_Position = u_transform * vec4(v_pos * u_scale + u_offset, 1.0);
	normal = v_normal;
}
