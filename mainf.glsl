uniform vec3 u_color;
uniform vec3 u_directional_light1; // (direction to light)
uniform vec3 u_directional_light1_color;
uniform vec3 u_directional_light2;
uniform vec3 u_directional_light2_color;
uniform vec3 u_ambient_light;
uniform int u_lighting_enabled;

varying vec3 normal;

void main() {
	vec3 N = normalize(normal);
	vec3 L;
	if (u_lighting_enabled != 0) {
		L = max(0.0, dot(u_directional_light1, N)) * u_directional_light1_color
			+ max(0.0, dot(u_directional_light2, N)) * u_directional_light2_color
			+ u_ambient_light;
	} else {
		L = vec3(1.0, 1.0, 1.0);
	}
	gl_FragColor = vec4(L * u_color, 1.0);
}
