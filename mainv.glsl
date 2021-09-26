attribute vec3 v_pos;
attribute vec3 v_normal;
uniform mat4 u_transform;
uniform vec3 u_offset;
uniform vec3 u_scale;
uniform float u_yaw, u_pitch;

varying vec3 normal;

void main() {
	float cy = cos(u_yaw),   sy = sin(u_yaw);
	float cp = cos(u_pitch), sp = sin(u_pitch);
	float x = v_pos.x, y = v_pos.y, z = v_pos.z;
	vec3 pos;
	pos = vec3(
		x,
		cp * y - sp * z,
		sp * y + cp * z
	);
	x = pos.x; y = pos.y; z = pos.z;
	pos = vec3(
		+cy * x + sy * z,
		y,
		-sy * x + cy * z
	);

	gl_Position = u_transform * vec4(pos * u_scale + u_offset, 1.0);
	normal = v_normal;
}
