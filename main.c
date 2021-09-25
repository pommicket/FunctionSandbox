#define V_GL 1
#define V_WINDOWED 1
#include "vlib.h"

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

	vec3 player_pos = {0};
	float player_yaw = 0;
	float player_pitch = 0;

	GLProgram *unlit = gl_program_new("unlitv.glsl", "unlitf.glsl");

	while (1) {
		SDL_Event event = {0};

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				goto quit;
			}
		}

		window_frame();

		mat4 camera = mat4_camera(player_pos, player_yaw, player_pitch, degree2rad(45), 0.1f, 50);
		gl_program_use(unlit);
		gl_uniformM4(unlit, "u_transform", &camera);
	}
	
quit:
	return 0;
}
