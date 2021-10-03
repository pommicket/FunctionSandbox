// @TODO:
//  - readme
//  - more examples
/*
Anyone is free to modify/distribute/use/sell/etc
this software for any purpose by any means.
Any copyright protections are relinquished by the author(s).
This software is provided "as is", without any warranty.
The author(s) shall not be held liable in connection with this software.
*/
#define V_GL 1
#define V_WINDOWED 1
#include "vlib.h"

static mat4 g_camera;
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
	float color_scale;
	vec4 color1, color2; // color1 = color of still grain, color2 = color of grain moving at 1/color_scale m/s
	GLProgram grain_program, update_program;
	GLVBO update_vbo, grain_vbo;
	GLVAO update_vao, grain_vao;
	uint32_t tex_height;
} Function;

// im calling this player because i originally had a game in mind and i dont know what else to call this
typedef struct {
	vec3 pos; // position of player's feet
	float yaw, pitch;
	float fov;
	float z_near, z_far;
	float speed;
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

static bool function_init(Function *f, const char *config_filename, int config_line_number, const char *wind_formula, uint32_t ngrains) {
	memset(f, 0, sizeof *f);
	{
		GLuint tex[2] = {0};
		gl.GenTextures(2, tex);
		f->grains_tex1 = tex[0];
		f->grains_tex2 = tex[1];
	}
	f->ngrains = ngrains;
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
			"    vec3 p = texelFetch(u_tex, src_pixel, 0).xyz;\n"
			"    float x = p.x, y = p.y, z = p.z;\n"
			"    vec3 wind = %s;\n"
			"    vec3 new_pos = p + wind * u_dt;\n"
			"    gl_FragColor = vec4(new_pos, 0.0);\n"
			"  }\n"
			"}\n",
			wind_formula
		);

		GLuint fshader = gl.CreateShader(GL_FRAGMENT_SHADER);
		const GLchar *sources[2];
		GLint status = 0;

		char header[256];
		strbuf_print(header,
			"#version %d\n"
			"#define GLSL_VERSION %d\n"
			"#define PI 3.14159265\n"
			"#define pi PI\n"
			"#line 1\n", V_glsl_version(), V_glsl_version());
		sources[0] = header;
		sources[1] = fshader_code;

		gl.ShaderSource(fshader, 2, sources, NULL);
		gl.CompileShader(fshader);
		{
			gl.GetShaderiv(fshader, GL_COMPILE_STATUS, &status);
			if (status == 1) {
				// all good!
			} else {
				char log[256];
				strbuf_print(log, "Your formula on line %d of %s has errors. Details:\n", config_line_number, config_filename);
				gl.GetShaderInfoLog(fshader, (GLsizei)(sizeof log - strlen(log)), NULL, log + strlen(log));
				window_message_box_error("Formula error", log);
				gl.DeleteShader(fshader);
				return false;
			}
		}

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
			"#define pi PI\n"
			"uniform vec4 u_color1, u_color2;\n"
			"uniform float u_color_scale;\n"
			"varying vec3 pos;\n"
			"void main() {\n"
			"  vec3 p = pos;\n"
			"  float x = p.x, y = p.y, z = p.z;\n"
			"  float wind_speed = length(%s);\n"
			"  gl_FragColor = mix(u_color1, u_color2, clamp(wind_speed * u_color_scale, 0.0, 1.0));\n"
			"}\n",
			wind_formula
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
			return false;
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
	return true;
}

static void sandbox_clear(Function **fs) {
	arr_foreachp(*fs, Function, f) {
		gl_vbo_delete(&f->update_vbo);
		gl_vbo_delete(&f->grain_vbo);
		gl_vao_delete(&f->update_vao);
		gl_vao_delete(&f->grain_vao);
		gl.DeleteFramebuffers(1, &f->fbo);
		GLuint tex[2] = {f->grains_tex1, f->grains_tex2};
		gl.DeleteTextures(2, tex);
		gl.DeleteProgram(f->update_program.id);
		gl.DeleteProgram(f->grain_program.id);
	}
	arr_clear(*fs);
}


// sets various things in `player` according to the configuration
// and returns a dynamic array of functions, or NULL on failure
static Function *sandbox_create(const char *config_filename, Player *player) {
	Function *functions = NULL;
	FILE *fp = fopen(config_filename, "r");
	if (fp) {
		int line_number = 0;
		uint32_t ngrains = 100000;
		float grain_refresh_rate = 0.01f;
		float grain_gen_radius = 2;
		vec4 color1 = Vec4(1.0f,0.8f,.6f,1);
		vec4 color2 = Vec4(1.0f,0,0,1);
		float color_scale = 0;
		char line[1024];
		
		player->fov = degree2rad(45);
		player->z_near = 1;
		player->z_far  = 50;
		player->speed  = 3;


		while (fgets(line, sizeof line, fp)) {
			++line_number;
		#define error(...) do {\
			char _buf1[256], _buf2[256];\
			strbuf_print(_buf1, __VA_ARGS__);\
			strbuf_print(_buf2, "%s line %d: %s", config_filename, line_number, _buf1);\
			window_message_box_error("Error loading sandbox", _buf2);\
			sandbox_clear(&functions);\
			return NULL;\
		} while (0)

			line[strcspn(line, "\r\n")] = '\0';
			if (line[0] == '#' || line[0] == '\0') continue;

			char *command = line;
			char *args = strchr(command, ' ');
			if (args) {
				*args++ = '\0';
				while (isspace(*args)) ++args;
			} else {
				args = command + strlen(command);
			}
			if (strcmp(command, "add") == 0) {
				Function *f = arr_addp(functions);
				if (!function_init(f, config_filename, line_number, args, ngrains)) {
					sandbox_clear(&functions);
					return NULL;
				}
				uint32_t nnew = (uint32_t)(grain_refresh_rate * (float)ngrains);
				f->new_grains_per_second = nnew == 0 ? 1 : nnew;
				f->grain_gen_radius = grain_gen_radius;
				f->color_scale = color_scale;
				f->color1 = color1;
				f->color2 = color2;
			} else if (strcmp(command, "grains") == 0) {
				ngrains = (unsigned)atoi(args);
				if (ngrains == 0 || ngrains > 1000000000) {
					error("Invalid number of grains: '%s'.", args);
				}
			} else if (strcmp(command, "grain_refresh_rate") == 0) {
				char *endptr = 0;
				double d = strtod(args, &endptr);
				if (*args == '\0' || *endptr != '\0' || d < 0.0 || d > 0.5)
					error("Invalid grain refresh rate: '%s' (should be from 0 to 0.5).", args);
				else
					grain_refresh_rate = (float)d;
			} else if (strcmp(command, "start_radius") == 0) {
				char *endptr = 0;
				double d = strtod(args, &endptr);
				if (*args == '\0' || *endptr != '\0' || d <= 0.0 || d > 1000)
					error("Invalid start radius: '%s'.", args);
				else
					grain_gen_radius = (float)d;
			} else if (strcmp(command, "color") == 0 || strcmp(command, "colour") == 0) {
				uint r=0, g=0, b=0;
				if (sscanf(args, "#%02x%02x%02x", &r, &g, &b) == 3 && r < 256 && g < 256 && b < 256)
					color1 = scale4(Vec4((float)r,(float)g,(float)b,255), 1.0f/255.0f);
				else
					error("Invalid color: '%s'.", args);
			} else if (strcmp(command, "color_scale") == 0 || strcmp(command, "colour_scale") == 0) {
				char *endptr = 0;
				double d = strtod(args, &endptr);
				if (*args == '\0' || *endptr != '\0' || d < 0.0 || d > 1000)
					error("Invalid color scale: '%s'.", args);
				else
					color_scale = (float)d;
			} else if (strcmp(command, "color2") == 0 || strcmp(command, "colour2") == 0) {
				uint r=0, g=0, b=0;
				if (sscanf(args, "#%02x%02x%02x", &r, &g, &b) == 3 && r < 256 && g < 256 && b < 256)
					color2 = scale4(Vec4((float)r,(float)g,(float)b,255), 1.0f/255.0f);
				else
					error("Invalid color: '%s'.", args);
			} else if (strcmp(command, "fov") == 0) {
				char *endptr = 0;
				double d = strtod(args, &endptr);
				if (*args == '\0' || *endptr != '\0' || d < 1 || d > 90)
					error("Invalid FOV: '%s' (must be 1-90).", args);
				else
					player->fov = degree2rad((float)d);
			} else if (strcmp(command, "clipping_planes") == 0) {
				if (sscanf(args, "%f %f", &player->z_near, &player->z_far) != 2 || player->z_near >= player->z_far || player->z_near < 0.01f || player->z_far > 1000)
					error("Invalid clipping planes: '%s'.", args);
			} else if (strcmp(command, "move_speed") == 0) {
				char *endptr = 0;
				double d = strtod(args, &endptr);
				if (*args == '\0' || *endptr != '\0' || d < 0.0 || d > 100)
					error("Invalid move speed: '%s'.", args);
				else
					player->speed = (float)d;
			} else {
				error("Command not recognized: '%s'.", command);
			}
		}
		fclose(fp);
	}
	return functions;
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

	GLText menu_text = {0};
	size_t menu_sel = 0;
	char config_name[256] = {0};
	Time config_last_modified = {0};
	Function *functions = NULL;

	bool fullscreen = false;

	while (1) {
	top:;

		SDL_Event event = {0};

		float dt = window_frame();

		if (*config_name) {
			char filename[512];
			strbuf_print(filename, "sandboxes/%s.txt", config_name);
			if (!time_eq(fs_last_modified(filename), config_last_modified)) {
				sleep_ms(10); // wait a bit to make sure whatever's writing here is done
				sandbox_clear(&functions);
				functions = sandbox_create(filename, player);
				config_last_modified = fs_last_modified(filename);
				if (!functions) {
					// return to menu if config failed to load
					*config_name = '\0';
					goto top;
				}
			}
		}

		size_t n_menu_items = 0;
		char selected_item[256] = {0};

		if (!*config_name) {
			DirEntry entries[1000] = {0};
			static char text[65536];
			size_t c = 0;
			size_t i, n_entries = fs_list_dir("sandboxes", entries, sizeof entries);

			qsort(entries, n_entries, sizeof *entries, fs_dir_entry_cmp_by_name_qsort);

			size_t y_offset = menu_sel < 10 ? 0 : menu_sel - 10;

			for (i = 0; i < n_entries; ++i) {
				char *dot = strchr(entries[i].name, '.');
				if (entries[i].type == FS_FILE && dot && strcmp(dot, ".txt") == 0) {
					if (n_menu_items >= y_offset) {
						*dot = '\0';
						if (n_menu_items == menu_sel) {
							// this is the currently selected item
							str_print(text + c, sizeof text - c, ">");
							++c;
							strbuf_cpy(selected_item, entries[i].name);
						}
						str_print(text + c, sizeof text - c, "%s\n", entries[i].name);
						c += strlen(text + c);
					}
					++n_menu_items;
				}
			}
			menu_sel %= n_menu_items;
			gl_text_set(&menu_text, text);
			fs_free_dir_entries(entries, n_entries);
		}

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				goto quit;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_F11:
					fullscreen = !fullscreen;
					window_set_fullscreen(fullscreen);
					break;
				case SDLK_UP:
					if (n_menu_items)
						menu_sel = (menu_sel + n_menu_items - 1) % n_menu_items;
					break;
				case SDLK_DOWN:
					if (n_menu_items)
						menu_sel = (menu_sel + 1) % n_menu_items;
					break;
				case SDLK_RETURN:
					if (!*config_name && menu_sel < n_menu_items) {
						strbuf_cpy(config_name, selected_item);
						config_last_modified = (Time){0};
						goto top; // force sandbox reload
					}
					break;
				case SDLK_ESCAPE:
					if (*config_name) {
						*config_name = '\0';
						sandbox_clear(&functions);
						goto top;
					} else {
						goto quit;
					}
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
		
		if (*config_name) {
			{
				int dx = (window_is_key_down(KEY_D) || window_is_key_down(KEY_RIGHT)) - (window_is_key_down(KEY_A) || window_is_key_down(KEY_LEFT));
				int dy = (window_is_key_down(KEY_PAGEUP) || window_is_key_down(KEY_Q)) - (window_is_key_down(KEY_PAGEDOWN) || window_is_key_down(KEY_E));
				int dz = (window_is_key_down(KEY_S) || window_is_key_down(KEY_DOWN)) - (window_is_key_down(KEY_W) || window_is_key_down(KEY_UP));
				if (dx || dy || dz) {
					vec3 dp = scale3(normalize3(Vec3((float)dx, (float)dy, (float)dz)), player->speed * dt);
					mat3 pitch = mat3_pitch(player->pitch);
					mat3 yaw = mat3_yaw(player->yaw);
					mat3 rot = mat3_mul(&yaw, &pitch);
					dp = transform3(&rot, dp);
					player->pos = add3(player->pos, dp);
				}
			}
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
				g_camera = mat4_camera(p, player->yaw, player->pitch, player->fov, player->z_near, player->z_far);
			}
			arr_foreachp(functions, Function, f) {
				gl_program_use(&f->grain_program);
				gl_uniformM4(&f->grain_program, "u_transform", &g_camera);
				gl_uniform4f(&f->grain_program, "u_color1", f->color1);
				gl_uniform4f(&f->grain_program, "u_color2", f->color2);
				gl_uniform1f(&f->grain_program, "u_color_scale", f->color_scale);
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
		} else {
			gl_text_render(&menu_text, Vec2(0,-1), 1, Vec3(1,1,1), Vec4(0,0,0,1));
		}

	}
	
quit:
	return 0;
}
