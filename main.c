#define V_GL 1
#define V_WINDOWED 1
#include "vlib.h"

static const float PLAYER_HEIGHT = 1.5f;

static GLProgram *program_unlit, *program_main;
static Model model_rod;
static mat4 g_camera;
static bool g_wireframe;

typedef enum {
	OBJ_NONE,
	OBJ_METAL_ROD
} ObjectType;

typedef struct {
	ObjectType type;
	vec3 pos;
	float yaw, pitch;
} ObjectAny;

typedef union {
	ObjectType type;
	ObjectAny  any;
} Object;

typedef struct {
	Object *objects; // dynamic array
} World;

typedef struct {
	vec3 pos; // position of player's feet
	ObjectType placing; // which object player is placing / OBJ_NONE if the player isn't placing anything
	float place_dist; // distance from player to place object (in the direction of player's vision)
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

static void object_render_sequence_begin(void) {
	gl_program_use(program_main);
	gl_uniformM4(program_main, "u_transform", &g_camera);
	gl_uniform1i(program_main, "u_lighting_enabled", !g_wireframe);
	gl_uniform3f(program_main, "u_directional_light1", normalize3(Vec3(1, 1, 1)));
	gl_uniform3f(program_main, "u_directional_light2", normalize3(Vec3(-1, 0.2f, -0.5f)));
	gl_uniform3f(program_main, "u_directional_light1_color", Vec3(1,1,.9f));
	gl_uniform3f(program_main, "u_directional_light2_color", Vec3(.9f,.9f,.8f));
	gl_uniform3f(program_main, "u_ambient_light", Vec3(.3f,.3f,.3f));
}

// returns true if player is placing an object, and fills out *obj with the details
static bool player_object_placing(const Player *player, Object *obj) {
	if (player->placing) {
		vec3 view = {0, 0, -1};
		mat3 yaw = mat3_yaw(player->yaw);
		mat3 pitch = mat3_pitch(player->pitch);
		mat3 rot = mat3_mul(&yaw, &pitch);
		view = transform3(&rot, view);

		vec3 player_hand_pos = player->pos;
		player_hand_pos.y += 0.6f * PLAYER_HEIGHT;


		obj->type = player->placing;
		obj->any.pos = add3(player_hand_pos, scale3(view, player->place_dist));
		obj->any.yaw = player->yaw;
		obj->any.pitch = player->pitch;
		return true;
	} else {
		return false;
	}
}

// call object_render_sequence_begin before this
enum {
	OBJ_RENDER_GHOST = 0x01
};
static void object_render(const Object *obj, uint flags) {
	vec3 pos = obj->any.pos;
	float alpha = (flags & OBJ_RENDER_GHOST) ? 0.5f : 1;

	gl_uniform3f(program_main, "u_offset", pos);
	gl_uniform1f(program_main, "u_yaw", obj->any.yaw);
	gl_uniform1f(program_main, "u_pitch", obj->any.pitch);

	switch (obj->type) {
	case OBJ_NONE:
		break;
	case OBJ_METAL_ROD:
		gl_uniform4f(program_main, "u_color", Vec4(.2f, .2f, .2f, alpha));
		gl_uniform3f(program_main, "u_scale", Vec3(.2f, .2f, .2f));
		model_render(&model_rod);
		break;
	}
}

int main(int argc, char **argv) {
	if (!window_create("sandbox", 1280, 720, 0)) {
		return -1;
	}

	Player player_data = {0}, *player = &player_data;
	World world_data = {0}, *world = &world_data;

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

	model_load(&model_rod, program_main, "assets/rod.obj");

	window_set_relative_mouse(1);

	player->place_dist = 3;

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
				case SDLK_r:
					player->placing = OBJ_METAL_ROD;
					break;
				case SDLK_ESCAPE:
					if (player->placing)
						player->placing = OBJ_NONE;
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
			case SDL_MOUSEBUTTONDOWN: {
				Object obj = {0};
				if (player_object_placing(player, &obj)) {
					arr_add(world->objects, obj);
				}
			} break;
			}
		}
		
		{
			int dx = window_is_key_down(KEY_D) - window_is_key_down(KEY_A);
			int dz = window_is_key_down(KEY_S) - window_is_key_down(KEY_W);
			if (dx || dz) {
				const float player_speed = 3;
				vec3 dp = scale3(normalize3(Vec3((float)dx, 0, (float)dz)), player_speed * dt);
				mat3 yaw = mat3_yaw(player->yaw);
				dp = transform3(&yaw, dp);
				player->pos = add3(player->pos, dp);
			}
		}


		gl.PolygonMode(GL_FRONT_AND_BACK, g_wireframe ? GL_LINE : GL_FILL);
		if (!g_wireframe) {
			gl.Enable(GL_DEPTH_TEST);
			gl.Enable(GL_CULL_FACE);
		}
		
		{
			vec3 p = player->pos;
			p.y += PLAYER_HEIGHT;
			g_camera = mat4_camera(p, player->yaw, player->pitch, degree2rad(45), 0.1f, 50);
		}
		gl_program_use(program_unlit);
		gl_uniformM4(program_unlit, "u_transform", &g_camera);

		gl_vao_render(ground_vao, &ground_ibo);
		gl_uniform4f(program_unlit, "u_color", Vec4(.5f,.5f,.5f,1));
		
//		Object test_obj;
//		test_obj.type = OBJ_METAL_ROD;
//		test_obj.any.pos = Vec3(1,1,1);
		
		object_render_sequence_begin();
		arr_foreachp(world->objects, Object, obj) {
			object_render(obj, 0);
		}


		Object ghost = {0};
		if (player_object_placing(player, &ghost)) {
			gl.Enable(GL_BLEND);
			object_render_sequence_begin();
			object_render(&ghost, OBJ_RENDER_GHOST);
			gl.Disable(GL_BLEND);
		}

	}
	
quit:
	return 0;
}
