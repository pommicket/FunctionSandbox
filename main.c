#define V_GL 1
#define V_WINDOWED 1
#include "vlib.h"

static const float PLAYER_HEIGHT = 1.5f;

static GLProgram *program_unlit, *program_main;

static void APIENTRY gl_message_callback(GLenum source, GLenum type, unsigned int id, GLenum severity,
	GLsizei length, const char *message, const void *userParam) {
	static int n = 0;
	(void)source; (void)type; (void)id; (void)length; (void)userParam;
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
	if (n++ < 10)
		debug_print("Message from OpenGL: %s.\n", message);
}

int main(int argc, char **argv) {
	if (!window_create("sandbox", 1280, 720, 0)) {
		return -1;
	}

	vec3 player_pos = {0}; // position of player's feet
	float player_yaw = 0;
	float player_pitch = 0;

	program_unlit = gl_program_new("unlitv.glsl", "unlitf.glsl");
	program_main  = gl_program_new("mainv.glsl", "mainf.glsl");
	typedef struct {
		vec3 pos;
	} UnlitVertex;

	GLVBO ground_vbo = gl_vbo_new(UnlitVertex, "ground");
	GLVAO ground_vao = gl_vao_new(program_unlit, "ground");
	GLIBO ground_ibo = gl_ibo_new("ground");
	{
		UnlitVertex vertices[] = {
			{-100, 0, -100},
			{-100, 0, +100},
			{+100, 0, +100},
			{+100, 0, -100},
		};
		GLuint indices[] = {
			0, 1, 2, 0, 2, 3
		};
		gl_vbo_set_static_data(&ground_vbo, vertices, static_arr_len(vertices));
		gl_ibo_set_static_data(&ground_ibo, indices,  static_arr_len(indices));
		gl_vao_add_data3f(&ground_vao, ground_vbo, "v_pos", UnlitVertex, pos);
	}

	Model model_rod;
	model_load(&model_rod, program_main, "assets/rod.obj");

	window_set_relative_mouse(1);

	bool wireframe = false;

	//gl.Enable(GL_MULTISAMPLE);

	while (1) {
		SDL_Event event = {0};

		float dt = window_frame();

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				goto quit;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_z:
					wireframe = !wireframe;
				}
				break;
			case SDL_MOUSEMOTION: {
				float dx = (float)event.motion.xrel;
				float dy = (float)event.motion.yrel;
				const float rot_speed = 0.001f;
				player_yaw   -= dx * rot_speed;
				player_pitch -= dy * rot_speed;
				player_yaw = fmodf(player_yaw, 2 * PI);
				player_pitch = clamp(player_pitch, -PI * 0.5f, PI * 0.5f);
			} break;
			}
		}
		
		{
			int dx = window_is_key_down(KEY_D) - window_is_key_down(KEY_A);
			int dz = window_is_key_down(KEY_S) - window_is_key_down(KEY_W);
			if (dx || dz) {
				const float player_speed = 3;
				vec3 dp = scale3(normalize3(Vec3((float)dx, 0, (float)dz)), player_speed * dt);
				mat3 yaw = mat3_yaw(player_yaw);
				dp = transform3(&yaw, dp);
				player_pos = add3(player_pos, dp);
			}
		}


		gl.PolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
		if (!wireframe) {
			gl.Enable(GL_DEPTH_TEST);
			gl.Enable(GL_CULL_FACE);
		}
		
		mat4 camera;
		{
			vec3 p = player_pos;
			p.y += PLAYER_HEIGHT;
			camera = mat4_camera(p, player_yaw, player_pitch, degree2rad(45), 0.1f, 50);
		}
		gl_program_use(program_unlit);
		gl_uniformM4(program_unlit, "u_transform", &camera);

		gl_uniform4f(program_unlit, "u_color", Vec4(.5f,.5f,.5f,1));
		gl_vao_render(ground_vao, &ground_ibo);
		
		gl_program_use(program_main);
		gl_uniformM4(program_main, "u_transform", &camera);
		gl_uniform1i(program_main, "u_lighting_enabled", !wireframe);
		gl_uniform3f(program_main, "u_directional_light1", normalize3(Vec3(1, 1, 1)));
		gl_uniform3f(program_main, "u_directional_light2", normalize3(Vec3(-1, 0.2f, -0.5f)));
		gl_uniform3f(program_main, "u_directional_light1_color", Vec3(1,1,.9f));
		gl_uniform3f(program_main, "u_directional_light2_color", Vec3(.9f,.9f,.8f));
		gl_uniform3f(program_main, "u_ambient_light", Vec3(.3f,.3f,.3f));
		gl_uniform3f(program_main, "u_color", Vec3(.2f,.2f,.2f));
		gl_uniform3f(program_main, "u_scale", Vec3(.2f,.2f,.2f));
		gl_uniform3f(program_main, "u_offset", Vec3(0,0.5f,0));
		model_render(&model_rod);
	}
	
quit:
	return 0;
}
