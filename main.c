// @TODO:
//  - run grain computations on GPU
//  - config file
#define V_GL 1
#define V_WINDOWED 1
#include "vlib.h"

static const float PLAYER_HEIGHT = 1.5f;

static GLProgram *program_grain;
static mat4 g_camera;
static bool g_wireframe;

typedef struct {
	vec3 *grains; // dynamic array in order of creation
} World;

typedef struct {
	vec3 pos; // position of player's feet
	float yaw, pitch;
} Player;

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
	if (gl_version_major * 100 + gl_version_minor < 310) {
		window_message_box_error("Error", "Couldn't get OpenGL 3.1 context (your graphics drivers are too old).");
		return -1;
	}

	Player player_data = {0}, *player = &player_data;
	World world_data = {0}, *world = &world_data;

	program_grain = gl_program_new("grainv.glsl", "grainf.glsl");
	typedef struct {
		vec3 pos;
	} UnlitVertex;

	GLVBO grain_vbo = gl_vbo_new(UnlitVertex, "grain");
	GLVAO grain_vao = gl_vao_new(program_grain, "grain");
	{
		float h = 0.005f;
		float HS3 = 0.866025403784f; // sqrt(3)/2
		UnlitVertex vertices[] = {
			{0,h*.5f,0},
			{-h*HS3,-h*.5f,0},
			{+h*HS3,-h*.5f,0},
			{0,h*.5f,0},
			{0,-h*.5f,-h*HS3},
			{0,-h*.5f,+h*HS3},
		};
		gl_vbo_set_static_data(&grain_vbo, vertices, static_arr_len(vertices));
		gl_vao_add_data3f(&grain_vao, grain_vbo, "v_pos", UnlitVertex, pos);
	}
	GLuint grains_texture = 0;
	gl.GenTextures(1, &grains_texture);

	window_set_relative_mouse(1);

	float grain_gen_radius = 2;
	for (int i = 0; i < 100000; ++i) {
		arr_add(world->grains, scale3(addc3(rand_vec3(), -.5f), grain_gen_radius * 2));
	}
	const uint32_t tex_width = 1024;
	uint32_t tex_chunk_size = tex_width * 4; // height must be a multiple of 4
	uint32_t tex_area = ((arr_len(world->grains) + tex_chunk_size-1) / tex_chunk_size) * tex_chunk_size; // width * height of grain pos texture
	if (tex_area < tex_chunk_size) tex_area = tex_chunk_size;
	uint32_t tex_height = tex_area / tex_width;

	arr_reserve(world->grains, tex_area);
	arr_foreachp(world->grains, vec3, g) {
		g->x = g->y = g->z = NAN;
	}

	float leftover_time = 0;

	while (1) {
		SDL_Event event = {0};

		float dt = window_frame();
		float timestep = 0.01f;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				goto quit;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_z:
					g_wireframe = !g_wireframe;
					break;
				}
				break;
			case SDL_MOUSEMOTION: {
				float dx = (float)event.motion.xrel;
				float dy = (float)event.motion.yrel;
				const float rot_speed = 0.001f;
				player->yaw   -= dx * rot_speed;
				player->pitch -= dy * rot_speed;
				player->yaw = fmodf(player->yaw, 2 * PI);
				player->pitch = clamp(player->pitch, -PI * 0.5f, PI * 0.5f);
			} break;
			}
		}
		
		{
			int dx = (window_is_key_down(KEY_D) || window_is_key_down(KEY_RIGHT)) - (window_is_key_down(KEY_A) || window_is_key_down(KEY_LEFT));
			int dy = window_is_key_down(KEY_PAGEUP) - window_is_key_down(KEY_PAGEDOWN);
			int dz = (window_is_key_down(KEY_S) || window_is_key_down(KEY_DOWN)) - (window_is_key_down(KEY_W) || window_is_key_down(KEY_UP));
			if (dx || dy || dz) {
				const float player_speed = 3;
				vec3 dp = scale3(normalize3(Vec3((float)dx, (float)dy, (float)dz)), player_speed * dt);
				mat3 yaw = mat3_yaw(player->yaw);
				dp = transform3(&yaw, dp);
				player->pos = add3(player->pos, dp);
			}
		}


		gl.PolygonMode(GL_FRONT_AND_BACK, g_wireframe ? GL_LINE : GL_FILL);
		if (!g_wireframe)
			gl.Enable(GL_DEPTH_TEST);
		
		{
			uint n_new_grains = 100;
			uint32_t ngrains = arr_len(world->grains);
			vec3 *p, *end;
			memmove(world->grains, world->grains + n_new_grains, (ngrains - n_new_grains) * sizeof *world->grains);
			end = world->grains + ngrains;
			for (p = end - n_new_grains; p < end; ++p) {
				*p = scale3(addc3(rand_vec3(), -.5f), grain_gen_radius * 2);
			}
		}

		{ // fixed time step (for consistency)
			float t = dt + leftover_time;
			while (t >= timestep) {
				arr_foreachp(world->grains, vec3, g) {
					float x = g->x, y = g->y, z = g->z;
					(void)x; (void)y; (void)z;
					vec3 wind = Vec3(-tanf(y)*x, cosf(z)*y, sinf(z)*x);
					*g = add3(*g, scale3(wind, timestep));
				}
				t -= timestep;
			}
			leftover_time = t;
		}

		{
			vec3 p = player->pos;
			p.y += PLAYER_HEIGHT;
			g_camera = mat4_camera(p, player->yaw, player->pitch, degree2rad(45), 1, 50);
		}
		gl_program_use(program_grain);
		gl_uniformM4(program_grain, "u_transform", &g_camera);
		gl_uniform4f(program_grain, "u_color", Vec4(1.0f,0.8f,.6f,1));
		gl.BindTexture(GL_TEXTURE_2D, grains_texture);
		gl.TexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, (GLsizei)tex_width, (GLsizei)tex_height, 0, GL_RGB, GL_FLOAT, world->grains);
		gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gl.ActiveTexture(GL_TEXTURE0);
		gl.BindTexture(GL_TEXTURE_2D, grains_texture);

		gl_uniform1i(program_grain, "u_offset_tex", 0);
		
		
		gl.BindVertexArray(grain_vao.id);
		gl.DrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)arr_len(world->grains));

	}
	
quit:
	return 0;
}
