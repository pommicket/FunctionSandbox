// @TODO:
//  - config file
//  - color coding based on speed
#define V_GL 1
#define V_WINDOWED 1
#include "vlib.h"

static mat4 g_camera;
static bool g_wireframe;
static const uint32_t tex_width = 1024;

typedef struct {
	vec3 pos;
} UnlitVertex;
typedef struct {
	vec2 pos;
} UpdateVertex;

typedef struct {
	GLuint grains_tex1, grains_tex2;
	GLuint fbo;
	uint32_t ngrains;
	uint32_t new_grains_per_second;
	float grain_gen_radius;
	float color_speed_scale;
	vec4 color1, color2; // color1 = color of still grain, color2 = color of grain moving at color_speed_scale m/s
	GLProgram grain_program, update_program;
	GLVBO update_vbo, grain_vbo;
	GLVAO update_vao, grain_vao;
	uint32_t tex_height;
} Function;

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

static GLuint g_grain_vshader, g_update_vshader;

static void function_create(Function *f, const char *wind_formula) {
	{
		GLuint tex[2] = {0};
		gl.GenTextures(2, tex);
		f->grains_tex1 = tex[0];
		f->grains_tex2 = tex[1];
	}
	f->color_speed_scale = 10;
	f->ngrains = 500000;
	f->new_grains_per_second = 1000;
	f->grain_gen_radius = 2;
	f->color1 = Vec4(1.0f,0.8f,.6f,1);
	f->color2 = Vec4(1.0f,0,0,1);
	{

		static char fshader_code[65536];
		strbuf_print(fshader_code,
			"uniform sampler2D u_tex;\n"
			"uniform float u_dt;\n"
			"uniform int u_new_grains_per_second;\n"
			"uniform float u_rand_seed;\n"
			"uniform float u_grain_gen_radius;\n"
			"vec3 rand(vec3 co) {\n"
			"   float x = 2920.0 * sin(dot(co, vec3(1123.4, 5454.8, 3454.3)))\n"
			"		* cos(dot(co, vec3(5487.0, 4873.0, 3948.0)));\n"
			"   float y = 1232.0 * sin(dot(co, vec3(4413.4, 6154.8, 3212.3)))\n"
			"		* cos(dot(co, vec3(2222.0, 3111.0, 4542.0)));\n"
			"   float z = 1111.0 * sin(dot(co, vec3(2282.4, 1101.8, 4744.3)))\n"
			"       * cos(dot(co, vec3(1452.0, 3894.0, 1111.0)));\n"
			"   return mod(vec3(x,y,z), 2.0) - 1.0;\n"
			"}\n"
			"void main() {\n"
			"  ivec2 pixel = ivec2(gl_FragCoord.xy);\n"
			"  int pixel_idx = (pixel.y << 10) | pixel.x;\n"
			"  if (pixel_idx < u_new_grains_per_second) {\n"
			"    gl_FragColor = vec4(u_grain_gen_radius * rand(vec3(gl_FragCoord.xy * 0.001, u_rand_seed)), 0.0);\n"
			"  } else {\n"
			"    int src_pixel_idx = pixel_idx - u_new_grains_per_second;\n"
			"    ivec2 src_pixel = ivec2(src_pixel_idx & 1023, src_pixel_idx >> 10);\n"
			"    vec3 prev_pos = texelFetch(u_tex, src_pixel, 0).xyz;\n"
			"    float x = prev_pos.x, y = prev_pos.y, z = prev_pos.z;\n"
			"    vec3 wind = vec3(%s);\n"
			"    vec3 new_pos = prev_pos + wind * u_dt;\n"
			"    gl_FragColor = vec4(new_pos, 0.0);\n"
			"  }\n"
			"}\n",
			wind_formula
		);

		GLuint fshader = V_gl_shader_compile_code("updatef.glsl", NULL, fshader_code, GL_FRAGMENT_SHADER);
		GLint status = 0;
		GLuint prog = gl.CreateProgram();
		gl.AttachShader(prog, g_update_vshader);
		gl.AttachShader(prog, fshader);
		gl.LinkProgram(prog);
		gl.GetProgramiv(prog, GL_LINK_STATUS, &status);
		gl.DetachShader(prog, g_update_vshader);
		gl.DetachShader(prog, fshader);
		gl.DeleteShader(fshader);
		assert(status);
		f->update_program.id = prog;
	}

	{
		static char fshader_code[65536];
		strbuf_print(fshader_code,
			"uniform vec4 u_color1, u_color2;\n"
			"varying vec3 pos;\n"
			"void main() {\n"
			"  float x = pos.x, y = pos.y, z = pos.z;\n"
			"  float wind_speed = length(vec3(%s));\n"
			"  gl_FragColor = mix(u_color1, u_color2, clamp(wind_speed * %f, 0.0, 1.0));\n"
			"}\n",
			wind_formula,
			f->color_speed_scale == 0 ? 1 : 1.0f / f->color_speed_scale
		);
		GLuint fshader = V_gl_shader_compile_code("grainf.glsl", NULL, fshader_code, GL_FRAGMENT_SHADER);
		GLint status = 0;
		GLuint prog = gl.CreateProgram();
		gl.AttachShader(prog, g_grain_vshader);
		gl.AttachShader(prog, fshader);
		gl.LinkProgram(prog);
		gl.GetProgramiv(prog, GL_LINK_STATUS, &status);
		gl.DetachShader(prog, g_grain_vshader);
		gl.DetachShader(prog, fshader);
		gl.DeleteShader(fshader);
		assert(status);
		f->grain_program.id = prog;
	}
	f->update_vbo = gl_vbo_new(UpdateVertex, "update");
	f->update_vao = gl_vao_new(&f->update_program, "update");
	{
		UpdateVertex vertices[] = {
			{-1,-1},
			{+1,-1},
			{+1,+1},
			{-1,-1},
			{+1,+1},
			{-1,+1}
		};
		gl_vbo_set_static_data(&f->update_vbo, vertices, static_arr_len(vertices));
		gl_vao_add_data2f(&f->update_vao, f->update_vbo, "v_pos", UpdateVertex, pos);
	}

	f->grain_vbo = gl_vbo_new(UnlitVertex, "grain");
	f->grain_vao = gl_vao_new(&f->grain_program, "grain");
	{
		float h = 0.005f;
		float S3 = 0.57735026919f; // sqrt(1/3)
		UnlitVertex vertices[] = {
			{0,h*.5f,0},
			{-h*S3,-h*.5f,0},
			{+h*S3,-h*.5f,0},
			{0,h*.5f,0},
			{0,-h*.5f,-h*S3},
			{0,-h*.5f,+h*S3},
		};
		gl_vbo_set_static_data(&f->grain_vbo, vertices, static_arr_len(vertices));
		gl_vao_add_data3f(&f->grain_vao, f->grain_vbo, "v_pos", UnlitVertex, pos);
	}

	uint32_t tex_chunk_size = tex_width * 4; // height must be a multiple of 4
	uint32_t tex_area = ((f->ngrains + tex_chunk_size-1) / tex_chunk_size) * tex_chunk_size; // width * height of grain pos texture
	if (tex_area < tex_chunk_size) tex_area = tex_chunk_size;
	uint32_t tex_height = tex_area / tex_width;
	f->tex_height = tex_height;

	{
		size_t ndata = (size_t)((f->ngrains + tex_chunk_size - 1) / tex_chunk_size) * tex_chunk_size;
		vec3 *data = calloc(ndata, sizeof *data); // required to initialize textures to NAN
		if (!data) {
			window_message_box_error("Out of memory", "Not enough memory for grains. Try reducing the max number of grains.");
			exit(-1);
		}
		for (int i = 0; i < ndata; ++i) {
			data[i] = Vec3(NAN, NAN, NAN);
		}

		gl.BindTexture(GL_TEXTURE_2D, f->grains_tex1);
		gl.TexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, (GLsizei)tex_width, (GLsizei)tex_height, 0, GL_RGB, GL_FLOAT, data);
		gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		gl.BindTexture(GL_TEXTURE_2D, f->grains_tex2);
		gl.TexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, (GLsizei)tex_width, (GLsizei)tex_height, 0, GL_RGB, GL_FLOAT, data);
		gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		free(data);
	}
	gl.GenFramebuffers(1, &f->fbo);
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
	Function *functions = NULL;


	window_set_relative_mouse(1);

	// global shader programs
	{
		const char *vshader_code =
			"attribute vec2 v_pos;\n"
			"void main() {\n"
			"  gl_Position = vec4(v_pos, 0.0, 1.0);\n"
			"}\n";
		g_update_vshader = V_gl_shader_compile_code("updatev.glsl", NULL, vshader_code, GL_VERTEX_SHADER);
	}
	{
		const char *vshader_code =
			"attribute vec3 v_pos;\n"
			"uniform mat4 u_transform;\n"
			"uniform sampler2D u_offset_tex;\n"
			"varying vec3 pos;\n"
			"void main() {\n"
			"  ivec2 texel_pos = ivec2(gl_InstanceID & 1023, gl_InstanceID >> 10);\n"
			"  vec3 offset = texelFetch(u_offset_tex, texel_pos, 0).xyz;\n"
			"  gl_Position = u_transform * vec4(v_pos + offset, 1.0);\n"
			"  pos = v_pos + offset;\n"
			"}\n";
		g_grain_vshader = V_gl_shader_compile_code("grainv.glsl", NULL, vshader_code, GL_VERTEX_SHADER);
	}

	{
		Function *f = arr_addp(functions);
		function_create(f,  "y * y * tan(z), z * z, -x * x");
	}{
		Function *f = arr_addp(functions);
		function_create(f,  "-y * y * tan(z), -z * z, x * x");
	}



	bool fullscreen = false;

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
					g_wireframe = !g_wireframe;
					break;
				case SDLK_F11:
					fullscreen = !fullscreen;
					window_set_fullscreen(fullscreen);
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
		

		arr_foreachp(functions, Function, f) {
			gl.Viewport(0,0,(GLsizei)tex_width,(GLsizei)f->tex_height);
			gl.BindFramebuffer(GL_FRAMEBUFFER, f->fbo);
			gl.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, f->grains_tex2, 0);
			gl_program_use(&f->update_program);
			gl.ActiveTexture(GL_TEXTURE0);
			gl.BindTexture(GL_TEXTURE_2D, f->grains_tex1);
			gl_uniform1i(&f->update_program, "u_tex", 0);
			gl_uniform1i(&f->update_program, "u_new_grains_per_second", (GLint)f->new_grains_per_second);
			gl_uniform1f(&f->update_program, "u_dt", dt);
			gl_uniform1f(&f->update_program, "u_rand_seed", randf());
			gl_uniform1f(&f->update_program, "u_grain_gen_radius", f->grain_gen_radius);
			gl_vao_render(f->update_vao, NULL);
		}

		gl.BindFramebuffer(GL_FRAMEBUFFER, 0);
		{
			vec2i wsize = window_get_size();
			gl.Viewport(0,0,wsize.x,wsize.y);
		}
		{
			vec3 p = player->pos;
			g_camera = mat4_camera(p, player->yaw, player->pitch, degree2rad(45), 1, 50);
		}
		arr_foreachp(functions, Function, f) {
			gl_program_use(&f->grain_program);
			gl_uniformM4(&f->grain_program, "u_transform", &g_camera);
			gl_uniform4f(&f->grain_program, "u_color1", f->color1);
			gl_uniform4f(&f->grain_program, "u_color2", f->color2);
			gl.BindTexture(GL_TEXTURE_2D, f->grains_tex2);
			gl.ActiveTexture(GL_TEXTURE0);
			gl_uniform1i(&f->grain_program, "u_offset_tex", 0);
			gl.BindVertexArray(f->grain_vao.id);
			gl.DrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)f->ngrains);

			{ // swap "front and back" grain textures
				GLuint temp = f->grains_tex1;
				f->grains_tex1 = f->grains_tex2;
				f->grains_tex2 = temp;
			}
		}
		

	}
	
quit:
	return 0;
}
