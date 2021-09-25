attribute vec3 v_pos;
uniform mat4 u_transform;

void main() {
	gl_Position = u_transform * vec4(v_pos, 1.0);
}
