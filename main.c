/* Copyright © 2022
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "wlr-layer-shell-unstable-v1-client-protocol.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <bits/getopt_core.h>
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wayland-egl.h>

static char usage[] = {
		"shaderbg [-h|--fps F|--layer l|--speed S|--shaderA A|--shaderB B] output-name shader.frag\n"
		"The provided fragment shaders should follow the Shadertoy API\n"};

static const struct option options[] = {{"help", no_argument, NULL, 'h'},
		{"speed", required_argument, NULL, 's'},
		{"fps", required_argument, NULL, 'f'},
		{"layer", required_argument, NULL, 'l'},
		{"shaderA", required_argument, NULL, 'A'},
		{"shaderB", required_argument, NULL, 'B'},
		{0, 0, NULL, 0}};

PFNGLCREATESHADERPROC glCreateShader;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
PFNGLCLEARBUFFERUIVPROC glClearBufferuiv;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM3FPROC glUniform3f;
PFNGLUNIFORM4FPROC glUniform4f;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;

#define load_gl_func(type, name) name = (type)eglGetProcAddress(#name)
void load_gl_funcs()
{
	load_gl_func(PFNGLCREATESHADERPROC, glCreateShader);
	load_gl_func(PFNGLCOMPILESHADERPROC, glCompileShader);
	load_gl_func(PFNGLSHADERSOURCEPROC, glShaderSource);
	load_gl_func(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog);
	load_gl_func(PFNGLLINKPROGRAMPROC, glLinkProgram);
	load_gl_func(PFNGLGETSHADERIVPROC, glGetShaderiv);
	load_gl_func(PFNGLCREATEPROGRAMPROC, glCreateProgram);
	load_gl_func(PFNGLATTACHSHADERPROC, glAttachShader);
	load_gl_func(PFNGLGETPROGRAMIVPROC, glGetProgramiv);
	load_gl_func(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog);
	load_gl_func(PFNGLBINDATTRIBLOCATIONPROC, glBindAttribLocation);
	load_gl_func(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation);
	load_gl_func(PFNGLUSEPROGRAMPROC, glUseProgram);
	load_gl_func(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays);
	load_gl_func(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray);
	load_gl_func(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer);
	load_gl_func(PFNGLGENBUFFERSPROC, glGenBuffers);
	load_gl_func(PFNGLBINDBUFFERPROC, glBindBuffer);
	load_gl_func(PFNGLBUFFERDATAPROC, glBufferData);
	load_gl_func(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers);
	load_gl_func(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer);
	load_gl_func(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D);
	load_gl_func(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus);
	load_gl_func(PFNGLCLEARBUFFERUIVPROC, glClearBufferuiv);
	load_gl_func(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers);
	load_gl_func(PFNGLUNIFORM1FPROC, glUniform1f);
	load_gl_func(PFNGLUNIFORM2FPROC, glUniform2f);
	load_gl_func(PFNGLUNIFORM3FPROC, glUniform3f);
	load_gl_func(PFNGLUNIFORM4FPROC, glUniform4f);
	load_gl_func(PFNGLUNIFORM1IPROC, glUniform1i);
	load_gl_func(PFNGLDELETESHADERPROC, glDeleteShader);
	load_gl_func(PFNGLENABLEVERTEXATTRIBARRAYPROC,
			glEnableVertexAttribArray);
}
#undef load_gl_func

struct shader {
	GLuint shader_prog;
	GLuint attr_pos;
	GLuint unif_iResolution;
	GLuint unif_iTime;
	GLuint unif_iTimeDelta;
	GLuint unif_iFrame;
	GLuint unif_iMouse;
	struct {
		GLint unif_A;
		GLint unif_B;
	} buffers;
};

struct state {
	float fps;   // how often to update output
	float speed; // ratio of real time to shader time
	enum zwlr_layer_shell_v1_layer layer;
	char *output_name;
	char *shader_path;
	struct {
		char *A;
		char *B;
	} texture_shader_paths;

	struct wl_display *display;
	struct wl_registry *registry;
	EGLDisplay egl_display;
	EGLConfig egl_config;
	EGLContext egl_context;
	struct wl_compositor *compositor;
	struct zwlr_layer_shell_v1 *layer_shell;

	float current_time;
	float delta_time;
	uint64_t frame_no;

	struct shader main_shader;
	struct {
		struct shader A;
		struct shader B;
	} texture_shaders;

	GLuint vertex_buffer;
	GLuint vertex_array;

	struct wl_list outputs;
};

struct output {
	struct wl_list link;

	struct state *state;
	uint32_t output_name;
	struct wl_output *output;
	char *str_name;

	/* surface and following elements are only set up if output name matches
	 * request */
	struct wl_surface *surface;
	struct zwlr_layer_surface_v1 *layer_surface;
	struct wl_egl_window *egl_window;
	EGLSurface egl_surface;
	/* if frame_callback is nonzero, do not render frame yet */
	struct wl_callback *frame_callback;

	int width, height;
	bool needs_ack;
	bool needs_resize;
	uint32_t last_serial;

	struct {
		GLuint A;
		GLuint B;
	} textures;
	struct {
		GLuint A;
		GLuint B;
	} framebuffers;
};

static bool check_gl_errors(const char *where)
{
	GLenum err;
	bool has_problems = false;
	while ((err = glGetError()) != GL_NO_ERROR) {
		fprintf(stderr, "GL error when %s: %x", where, err);
		has_problems = true;
	}
	return !has_problems;
}

static void destroy_output(struct output *output)
{
	if (output->frame_callback) {
		wl_callback_destroy(output->frame_callback);
	}
	if (output->egl_surface) {
		eglDestroySurface(output->state->egl_display,
				output->egl_surface);
	}
	if (output->surface) {
		wl_surface_destroy(output->surface);
	}
	if (output->layer_surface) {
		zwlr_layer_surface_v1_destroy(output->layer_surface);
	}
	if (output->egl_window) {
		wl_egl_window_destroy(output->egl_window);
	}

	free(output->str_name);
	wl_output_destroy(output->output);
	wl_list_remove(&output->link);
	free(output);
}

static void frame_done(void *data, struct wl_callback *wl_callback,
		uint32_t callback_data)
{
	struct output *output = data;
	if (output->frame_callback != wl_callback) {
		fprintf(stderr, "Frame callback tracking error\n");
		exit(EXIT_FAILURE);
	}
	/* clearing frame callback field indicates output is ready for drawing.
	 * Note that this callback will interrupt the call to poll in the main
	 * loop.
	 */
	wl_callback_destroy(wl_callback);
	output->frame_callback = NULL;
}

static struct wl_callback_listener frame_callback_listener = {
		.done = frame_done,
};

static void draw_frame(struct output *output) {
	struct state *state = output->state;
	if (!eglMakeCurrent(state->egl_display, output->egl_surface,
			    output->egl_surface, state->egl_context)) {
		fprintf(stderr, "Failed to make current\n");
		exit(EXIT_FAILURE);
	}

	glViewport(0, 0, output->width, output->height);

	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(state->vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, state->vertex_buffer);
	struct shader shader = state->main_shader;
	glUseProgram(shader.shader_prog);

	// todo: why do we need to call this here?
	glVertexAttribPointer(
			shader.attr_pos, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, output->textures.A);
	GLuint iBufferALocation = shader.buffers.unif_A;
	glUniform1i(iBufferALocation, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, output->textures.B);
	GLuint iBufferBLocation = shader.buffers.unif_B;
	glUniform1i(iBufferBLocation, 1);

	glUniform1f(shader.unif_iTime, state->current_time);
	glUniform1f(shader.unif_iTimeDelta, state->delta_time);
	GLfloat w = output->width, h = output->height;
	glUniform3f(shader.unif_iResolution, w, h, 0.);
	glUniform1i(shader.unif_iFrame, state->frame_no);
	glUniform4f(shader.unif_iMouse, 0., 0., 0., 0.);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 3);

	if (!check_gl_errors("drawing")) {
		exit(EXIT_FAILURE);
	}
}

static void redraw_texture(struct output *output, GLuint framebuffer, struct shader *shader) {
	struct state *state = output->state;
	if (!eglMakeCurrent(state->egl_display, output->egl_surface,
			    output->egl_surface, state->egl_context)) {
		fprintf(stderr, "Failed to make current\n");
		exit(EXIT_FAILURE);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(0, 0, output->width, output->height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader->shader_prog);

	// todo: why do we need to call this here?
	glVertexAttribPointer(
			shader->attr_pos, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

	glUniform1f(shader->unif_iTime, state->current_time);
	glUniform1f(shader->unif_iTimeDelta, state->delta_time);
	GLfloat w = output->width, h = output->height;
	glUniform3f(shader->unif_iResolution, w, h, 0.);
	glUniform1i(shader->unif_iFrame, state->frame_no);
	glUniform4f(shader->unif_iMouse, 0., 0., 0., 0.);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 3);

    if (!check_gl_errors("drawing to texture")) {
        exit(EXIT_FAILURE);
    }

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void redraw_textures(struct output *output) {
	if(output->state->texture_shaders.A.shader_prog) redraw_texture(output, output->framebuffers.A, &output->state->texture_shaders.A);
	if(output->state->texture_shaders.B.shader_prog) redraw_texture(output, output->framebuffers.B, &output->state->texture_shaders.B);
}

static void redraw(struct output *output)
{
	redraw_textures(output);
	draw_frame(output);
}

static void create_framebuffer(struct output *output, GLuint texture, GLuint *framebuffer) {
	glGenFramebuffers(1, framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, *framebuffer);

	glBindTexture(GL_TEXTURE_2D, texture);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        fprintf(stderr, "Error creating texture: %x\n", err);
        exit(EXIT_FAILURE);
    }


	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, output->width, output->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	GLuint color[4] = {255, 1, 0, 0};
	glClearBufferuiv(GL_COLOR, 0, color);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "Framebuffer not complete (%x) but %x one of (%x)", GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER),
		  GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
		exit(EXIT_FAILURE);
	}

	

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

static void layer_surface_configure(void *data,
		struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1,
		uint32_t serial, uint32_t width, uint32_t height)
{
	struct output *output = data;
	struct state *state = output->state;
	if (width > 0) {
		output->width = width;
	}
	if (height > 0) {
		output->height = height;
	}

	glGenTextures(1, &output->textures.A);
	glGenTextures(1, &output->textures.B);
	create_framebuffer(output, output->textures.A, &output->framebuffers.A);
	create_framebuffer(output, output->textures.B, &output->framebuffers.B);

	if (!output->egl_window) {
		zwlr_layer_surface_v1_ack_configure(
				zwlr_layer_surface_v1, serial);

		output->egl_window = wl_egl_window_create(
				output->surface, output->width, output->height);

		output->egl_surface = eglCreateWindowSurface(state->egl_display,
				state->egl_config,
				(EGLNativeWindowType)output->egl_window, NULL);
		if (!output->egl_surface) {
			fprintf(stderr, "Failed to create window surface\n");
			exit(EXIT_FAILURE);
		}

		// first draw provides contents
		redraw(output);

		/* Manage swap intervals ourselves; a blocking eglSwapBuffers
		 * doesn't handle mixed frame rates or occluded windows
		 * properly. (This uses the current context/surface from call to
		 * redraw().)
		 */
		if (!eglSwapInterval(state->egl_display, 0)) {
			fprintf(stderr, "Failed to set swap interval\n");
			exit(EXIT_FAILURE);
		}

		output->frame_callback = wl_surface_frame(output->surface);
		wl_callback_add_listener(output->frame_callback,
				&frame_callback_listener, output);

		if (!eglSwapBuffers(state->egl_display, output->egl_surface)) {
			fprintf(stderr, "Failed to swap buffers\n");
			exit(EXIT_FAILURE);
		}
	} else {
		output->needs_ack = true;
		output->needs_resize = true;
		output->last_serial = serial;
	}
}
static void layer_surface_closed(
		void *data, struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1)
{
	struct output *output = data;
	destroy_output(output);
}

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
		.configure = layer_surface_configure,
		.closed = layer_surface_closed,
};

static void output_geometry(void *data, struct wl_output *wl_output, int32_t x,
		int32_t y, int32_t physical_width, int32_t physical_height,
		int32_t subpixel, const char *make, const char *model,
		int32_t transform)
{
	// ignore
}
static void output_mode(void *data, struct wl_output *wl_output, uint32_t flags,
		int32_t width, int32_t height, int32_t refresh)
{
	// ignore
}

static void output_done(void *data, struct wl_output *wl_output)
{
	struct output *output = data;
	struct state *state = output->state;
	if (output->surface) {
		/* output has already been given a surface */
		return;
	}

	bool wildcard = !strcmp(state->output_name, "*");
	if (wildcard || (output->str_name &&
					!strcmp(output->str_name,
							state->output_name))) {
		output->surface =
				wl_compositor_create_surface(state->compositor);
		output->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
				state->layer_shell, output->surface,
				output->output, state->layer, "shaderbg");
		// todo: maybe propose size equal to given output size?

		const uint32_t center = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
					ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
					ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
					ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
		zwlr_layer_surface_v1_set_anchor(output->layer_surface, center);
		zwlr_layer_surface_v1_set_exclusive_zone(
				output->layer_surface, -1);
		zwlr_layer_surface_v1_add_listener(output->layer_surface,
				&layer_surface_listener, output);
		wl_surface_commit(output->surface);
	}
}
static void output_scale(
		void *data, struct wl_output *wl_output, int32_t factor)
{
	// ignore
}

static void output_name(
		void *data, struct wl_output *wl_output, const char *name)
{
	struct output *output = data;
	free(output->str_name);
	output->str_name = strdup(name);
}
static void output_description(void *data, struct wl_output *wl_output,
		const char *description)
{
	// ignore
}

static const struct wl_output_listener output_listener = {
		.geometry = output_geometry,
		.mode = output_mode,
		.done = output_done,
		.scale = output_scale,
		.name = output_name,
		.description = output_description,
};

static void registry_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version)
{
	struct state *state = data;

	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		state->compositor = wl_registry_bind(
				registry, name, &wl_compositor_interface, 1);
	} else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		state->layer_shell = wl_registry_bind(registry, name,
				&zwlr_layer_shell_v1_interface, 1);
	} else if (strcmp(interface, wl_output_interface.name) == 0 &&
			version >= 4) {
		/* we only accept version >= 4 outputs, as those provide their
		 * name/description */

		struct output *output = calloc(1, sizeof(struct output));
		if (!output) {
			fprintf(stderr, "Failed to allocate output\n");
			return;
		}

		struct wl_output *wl_output = wl_registry_bind(
				registry, name, &wl_output_interface, 4);
		wl_output_add_listener(wl_output, &output_listener, output);

		output->output = wl_output;
		output->state = state;
		output->output_name = name;

		wl_list_insert(&state->outputs, &output->link);
	}
}

static void registry_global_remove(
		void *data, struct wl_registry *registry, uint32_t name)
{
	struct state *state = data;
	struct output *o, *tmp;
	wl_list_for_each_safe(o, tmp, &state->outputs, link)
	{
		if (o->output_name == name) {
			destroy_output(o);
		}
	}
}

static const struct wl_registry_listener registry_listener = {
		registry_global, registry_global_remove};

static float timespec_diff(struct timespec to, struct timespec from)
{
	return 1e-9f * (to.tv_nsec - from.tv_nsec) +
	       1.f * (to.tv_sec - from.tv_sec);
}

static const char vertex_shader_text[] =
		"attribute vec2 pos;\n"
		"void main() {\n"
		"  gl_Position = vec4(pos.x, pos.y, 0, 1);\n"
		"}\n";

static const char frag_prologue_version[] = "#version 330\n";
static const char frag_prologue_buffer_A[] = "uniform sampler2D iBufferA; ";
static const char frag_prologue_buffer_B[] = "uniform sampler2D iBufferB; ";
static const char frag_prologue_buffer_C[] = "uniform sampler2D iBufferC; ";
static const char frag_prologue_buffer_D[] = "uniform sampler2D iBufferD; ";
/* Only one newline -- these can mess up the line count in the composed shader
and make debugging harder */
static const char frag_prologue[] = "uniform vec3 iResolution; "
				    "uniform float iTime; "
				    "uniform float iTimeDelta; "
				    "uniform float iFrame; "
				    "uniform vec4 iMouse;\n";

static const char frag_coda[] =
		"void main() {\n"
		"    mainImage(gl_FragColor, gl_FragCoord.xy);\n"
		"}\n";

static GLuint create_frag_shader(char *path, int A, int B, int C, int D) {
	if (!path) return 0;
	FILE *frag_file = fopen(path, "rb");
	if (!frag_file) {
		fprintf(stderr, "Failed to read shader file at '%s'\n", path);
		return EXIT_FAILURE;
	}
	fseek(frag_file, 0, SEEK_END);
	GLint frag_len = (GLint)ftell(frag_file);
	fseek(frag_file, 0, SEEK_SET);
	char *frag_text = (char *)malloc((size_t)frag_len);
	if (!frag_text) {
		fprintf(stderr, "Failed to allocate space to read shader file\n");
		return EXIT_FAILURE;
	}
	fread(frag_text, (size_t)frag_len, 1, frag_file);
	fclose(frag_file);

	GLint glstatus;

	GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	const char *frag_parts[] = {
			frag_prologue_version,
			A ? frag_prologue_buffer_A : "",
			B ? frag_prologue_buffer_B : "",
			C ? frag_prologue_buffer_C : "",
			D ? frag_prologue_buffer_D : "",
			frag_prologue,
			frag_text,
			frag_coda,
	};
	int frag_lengths[] = {
			sizeof(frag_prologue_version) - 1,
			(sizeof(frag_prologue_buffer_A) - 1) * A,
			(sizeof(frag_prologue_buffer_B) - 1) * B,
			(sizeof(frag_prologue_buffer_C) - 1) * C,
			(sizeof(frag_prologue_buffer_D) - 1) * D,
			sizeof(frag_prologue) - 1,
			frag_len,
			sizeof(frag_coda) - 1,
	};
	glShaderSource(frag_shader, sizeof(frag_parts) / sizeof(frag_parts[0]), frag_parts, frag_lengths);
	free(frag_text);
	glCompileShader(frag_shader);

	glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &glstatus);
	if (!glstatus) {
		char log[1024] = {0};
		GLsizei len;
		glGetShaderInfoLog(frag_shader, 1024, &len, log);
		fprintf(stderr, "Failed to compile fragment shader: %.*s\n",
				len, log);
		return EXIT_FAILURE;
	}

	return frag_shader;
}

static void create_shader(GLuint frag_shader, GLuint vertex_shader, struct shader *shader) {
	shader->shader_prog = glCreateProgram();
	glAttachShader(shader->shader_prog, frag_shader);
	glAttachShader(shader->shader_prog, vertex_shader);
	shader->attr_pos = 0;
	glBindAttribLocation(shader->shader_prog, shader->attr_pos, "pos");
	glLinkProgram(shader->shader_prog);
	GLint glstatus;
	glGetProgramiv(shader->shader_prog, GL_LINK_STATUS, &glstatus);
	if (!glstatus) {
		char log[1024] = {0};
		GLsizei len;
		glGetProgramInfoLog(shader->shader_prog, 1000, &len, log);
		fprintf(stderr, "Failed to link shader:\n%.*s\n", len, log);
		exit(1);
	}

	shader->unif_iResolution =
			glGetUniformLocation(shader->shader_prog, "iResolution");
	shader->unif_iTime = glGetUniformLocation(shader->shader_prog, "iTime");
	shader->unif_iTimeDelta =
			glGetUniformLocation(shader->shader_prog, "iTimeDelta");
	shader->unif_iFrame = glGetUniformLocation(shader->shader_prog, "iFrame");
	shader->unif_iMouse = glGetUniformLocation(shader->shader_prog, "iMouse");
	shader->buffers.unif_A = glGetUniformLocation(shader->shader_prog, "iBufferA");
	shader->buffers.unif_B = glGetUniformLocation(shader->shader_prog, "iBufferB");
	glVertexAttribPointer(
			shader->attr_pos, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
}

int main(int argc, char **argv)
{
	struct state state = {0};
	state.fps = INFINITY;
	state.speed = 1.f;
	state.layer = ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND;
	wl_list_init(&state.outputs);

	while (true) {
		int opt = getopt_long(argc, argv, "hf:l:A:B:", options, NULL);
		if (opt == -1) {
			break;
		}

		switch (opt) {
		case 'h':
			fprintf(stdout, "%s", usage);
			fprintf(stdout, "\nPrefix:\n\n%s", frag_prologue);
			fprintf(stdout, "\nSuffix:\n\n%s", frag_coda);
			return EXIT_SUCCESS;
		case 'f': {
			char *endptr = NULL;
			state.fps = strtof(optarg, &endptr);
			if (*endptr != '\0' || !(state.fps > 0)) {
				fprintf(stdout, "Invalid fps '%s'\n", optarg);
				return EXIT_FAILURE;
			}
		} break;
		case 's': {
			char *endptr = NULL;
			state.speed = strtof(optarg, &endptr);
			if (*endptr != '\0' || !(state.speed > 0)) {
				fprintf(stdout, "Invalid speed '%s'\n", optarg);
				return EXIT_FAILURE;
			}
		} break;

		case 'l':
			if (!strcmp(optarg, "background")) {
				state.layer = ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND;
			} else if (!strcmp(optarg, "bottom")) {
				state.layer = ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM;
			} else if (!strcmp(optarg, "top")) {
				state.layer = ZWLR_LAYER_SHELL_V1_LAYER_TOP;
			} else if (!strcmp(optarg, "overlay")) {
				state.layer = ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY;
			} else {
				fprintf(stdout, "Invalid layer '%s'; should be one of 'background', 'bottom', 'top', 'overlay'\n",
						optarg);
			}

			break;

		case 'A': {
			state.texture_shader_paths.A = optarg;
			} break;
		case 'B': {
			state.texture_shader_paths.B = optarg;
			} break;

		default:
			fprintf(stdout, "%s", usage);
			return EXIT_FAILURE;
		}
	}

	if (optind + 2 != argc) {
		fprintf(stdout, "%s", usage);
		return EXIT_FAILURE;
	}
	state.output_name = argv[optind];
	state.shader_path = argv[optind + 1];

	fprintf(stderr, "Running shaderbg with output = '%s' shader = '%s' fps = %f layer = %d\n",
			state.output_name, state.shader_path, state.fps,
			state.layer);

	state.display = wl_display_connect(NULL);
	state.registry = wl_display_get_registry(state.display);
	wl_registry_add_listener(state.registry, &registry_listener, &state);

	const char *extensions_list =
			eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	bool has_ext_platform = false, has_khr_platform = false;
	if (strstr(extensions_list, "EGL_EXT_platform_wayland")) {
		has_ext_platform = true;
	}
	if (strstr(extensions_list, "EGL_KHR_platform_wayland")) {
		has_khr_platform = true;
	}
	if (!has_khr_platform && !has_ext_platform) {
		fprintf(stderr, "No egl wayland\n");
		return EXIT_FAILURE;
	}

	state.egl_display = eglGetPlatformDisplay(
			has_ext_platform ? EGL_PLATFORM_WAYLAND_EXT
					 : EGL_PLATFORM_WAYLAND_KHR,
			state.display, NULL);
	if (state.egl_display == EGL_NO_DISPLAY) {
		fprintf(stderr, "Failed to get display\n");
		return EXIT_FAILURE;
	}

	int major_version = -1, minor_version = -1;
	if (!eglInitialize(state.egl_display, &major_version, &minor_version)) {
		fprintf(stderr, "Failed to init display\n");
		return EXIT_FAILURE;
	}

	if (!eglBindAPI(EGL_OPENGL_API)) {
		fprintf(stderr, "Failed to bind opengl api\n");
		return EXIT_FAILURE;
	}

	int count = 0;
	if (!eglGetConfigs(state.egl_display, NULL, 0, &count) || count < 1) {
		fprintf(stderr, "Failed to get configs\n");
		return EXIT_FAILURE;
	}

	EGLConfig *configs = calloc(count, sizeof(EGLConfig));

	EGLint config_attrib_list[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			/* no depth constraint, but see later */
			EGL_RED_SIZE, 1, EGL_GREEN_SIZE, 1, EGL_BLUE_SIZE, 1,
			/* use OpenGL */
			EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
			/* pick floating point */
			//			EGL_COLOR_COMPONENT_TYPE_EXT,
			//			EGL_COLOR_COMPONENT_TYPE_FLOAT_EXT,
			EGL_NONE};

	int nret = 0;
	if (!eglChooseConfig(state.egl_display, config_attrib_list, configs,
			    count, &nret) ||
			nret < 1) {
		fprintf(stderr, "Failed to get matching config\n");
		return EXIT_FAILURE;
	}
	int depth_r = 0, depth_b = 0, depth_g = 0, depth_a = 0;
	state.egl_config = configs[0];

	if (false) {
		eglGetConfigAttrib(state.egl_display, state.egl_config,
				EGL_RED_SIZE, &depth_r);
		eglGetConfigAttrib(state.egl_display, state.egl_config,
				EGL_GREEN_SIZE, &depth_g);
		eglGetConfigAttrib(state.egl_display, state.egl_config,
				EGL_BLUE_SIZE, &depth_b);
		eglGetConfigAttrib(state.egl_display, state.egl_config,
				EGL_ALPHA_SIZE, &depth_a);
		fprintf(stderr, "Bit depths: r=%d g=%d b=%d a=%d\n", depth_r,
				depth_g, depth_b, depth_a);
	}

	load_gl_funcs();

	/* request at least OpenGL 2.0 */
	EGLint context_attribs[] = {EGL_CONTEXT_MAJOR_VERSION, 2, EGL_NONE};

	state.egl_context = eglCreateContext(state.egl_display,
			state.egl_config, EGL_NO_CONTEXT, context_attribs);
	if (!state.egl_context) {
		fprintf(stderr, "Failed to create context: %x\n",
				eglGetError());
		return EXIT_FAILURE;
	}

	if (eglMakeCurrent(state.egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
			    state.egl_context) == EGL_FALSE) {
		fprintf(stderr, "Failed to make context current: %x\n",
				eglGetError());
		return EXIT_FAILURE;
	}

	fprintf(stderr, "GL version: %s\n", glGetString(GL_VERSION));
	if (glGetError() != GL_NO_ERROR) {
		fprintf(stderr, "Problem with OpenGL\n");
		return EXIT_FAILURE;
	}

	GLuint texture_shader_A = create_frag_shader(state.texture_shader_paths.A, 0,0,0,0);
	GLuint texture_shader_B = create_frag_shader(state.texture_shader_paths.B, 0,0,0,0);
	GLuint frag_shader = create_frag_shader(state.shader_path, !!texture_shader_A, !!texture_shader_B ,0,0);

	GLint glstatus;
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	const char *vtext = vertex_shader_text;

	glShaderSource(vertex_shader, 1, &vtext, NULL);
	glCompileShader(vertex_shader);

	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &glstatus);
	if (!glstatus) {
		char log[1024] = {0};
		GLsizei len;
		glGetShaderInfoLog(vertex_shader, 1024, &len, log);
		fprintf(stderr, "Failed to compile vertex shader: %.*s\n", len,
				log);
		return EXIT_FAILURE;
	}

	create_shader(frag_shader, vertex_shader, &state.main_shader);
	if (texture_shader_A) create_shader(texture_shader_A, vertex_shader, &state.texture_shaders.A);
	if (texture_shader_B) create_shader(texture_shader_B, vertex_shader, &state.texture_shaders.B);

	glDeleteShader(frag_shader);
	glDeleteShader(vertex_shader);
	if (texture_shader_A) glDeleteShader(texture_shader_A);
	if (texture_shader_B) glDeleteShader(texture_shader_B);

	glGenVertexArrays(1, &state.vertex_array);
	glBindVertexArray(state.vertex_array);

	glEnableVertexAttribArray(0);

	glGenBuffers(1, &state.vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, state.vertex_buffer);
	/* Use a single triangle for the entire scene; this is _very_ slightly
	 * more efficient than using two triangles due to only drawing things at
	 * the interface once */
	GLfloat vertex_data[3][2] = {{
						     -1.0f,
						     -3.0f,
				     },
			{
					-1.0f,
					1.0f,
			},
			{
					3.0f,
					1.0f,
			}};
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data,
			GL_STATIC_DRAW);

	if (!check_gl_errors("loading shaders")) {
		return EXIT_FAILURE;
	}

	/* bind all globals */
	wl_display_roundtrip(state.display);
	/* learn all output names, and create outputs if necessary */
	wl_display_roundtrip(state.display);

	struct timespec start_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);

	struct timespec next_draw_time = start_time;
	struct timespec last_frame_time = start_time;

	int64_t period_ns = 1e9f / state.fps;
	struct timespec period = {
			.tv_sec = period_ns / 1000000000,
			.tv_nsec = period_ns % 1000000000,
	};

	int display_fd = wl_display_get_fd(state.display);

	int ret = EXIT_SUCCESS;
	while (true) {
		if (wl_display_dispatch_pending(state.display) == -1) {
			fprintf(stderr, "Failed to dispatch events\n");
			break;
		}
		if (wl_display_flush(state.display) == -1 && errno != EAGAIN) {
			fprintf(stderr, "Failed to flush\n");
			break;
		}

		struct timespec cur_time;
		clock_gettime(CLOCK_MONOTONIC, &cur_time);
		int64_t ms_until_next_draw =
				1000 * (next_draw_time.tv_sec -
						       cur_time.tv_sec) +
				(next_draw_time.tv_nsec - cur_time.tv_nsec) /
						1000000;
		struct output *output, *tmp;
		bool any_output_ready = false;
		wl_list_for_each_safe(output, tmp, &state.outputs, link)
		{
			any_output_ready = any_output_ready ||
					   (!output->frame_callback);
		}

		/* If no outputs are ready, wait until the next Wayland event,
		 * which will hopefully mark something as ready. If there is a
		 * ready output, we wait _at least_ one millisecond, because
		 * the poll API does not provide sub-millisecond waits.
		 * (todo: use timerfd here instead -- lost milliseconds matter
		 *  at 240fps) */
		int timeout_ms;
		if (!any_output_ready) {
			timeout_ms = -1;
		} else {
			timeout_ms = (ms_until_next_draw > 1
							? ms_until_next_draw
							: 1);
		}

		struct pollfd pollfd;
		pollfd.events = POLLIN;
		pollfd.fd = display_fd;

		int nr = poll(&pollfd, 1, timeout_ms);
		if (nr < 0 && (errno == EAGAIN || errno == EINTR)) {
			continue;
		} else if (nr < 0) {
			fprintf(stderr, "poll failure\n");
			break;
		}

		if (pollfd.revents & POLLIN) {
			if (wl_display_prepare_read(state.display) == -1) {
				fprintf(stderr, "Failed to prepare read\n");
				break;
			}
			if (wl_display_read_events(state.display) == -1) {
				fprintf(stderr, "Failed to read events\n");
				break;
			}
		}

		/* Decide if all frames should be redrawn */
		bool any_resized = false;
		wl_list_for_each_safe(output, tmp, &state.outputs, link)
		{
			any_resized = any_resized || output->needs_resize;
		}

		clock_gettime(CLOCK_MONOTONIC, &cur_time);
		float time_until_next_draw =
				timespec_diff(next_draw_time, cur_time);
		if (time_until_next_draw > 0 && !any_resized) {
			continue;
		}
		/* update next draw time, skipping frames if we are behind */
		int frame_gap = (int)(-time_until_next_draw * state.fps);
		if (frame_gap < 1) {
			frame_gap = 1;
		}
		next_draw_time.tv_nsec +=
				(frame_gap * period.tv_nsec) % 1000000000;
		next_draw_time.tv_sec +=
				(frame_gap * period.tv_nsec) / 1000000000 +
				frame_gap * period.tv_sec;
		if (next_draw_time.tv_nsec > 1000000000) {
			next_draw_time.tv_nsec -= 1000000000;
			next_draw_time.tv_sec += 1;
		}

		state.current_time = timespec_diff(cur_time, start_time) *
				     state.speed;
		state.delta_time = timespec_diff(cur_time, last_frame_time) *
				   state.speed;
		last_frame_time = cur_time;
		state.frame_no++;

		/* Submit redraw information */
		wl_list_for_each_safe(output, tmp, &state.outputs, link)
		{
			if (!output->egl_window || output->frame_callback) {
				continue;
			}

			if (output->needs_ack) {
				output->needs_ack = false;
				zwlr_layer_surface_v1_ack_configure(
						output->layer_surface,
						output->last_serial);
			}
			if (output->needs_resize) {
				output->needs_resize = false;
				wl_egl_window_resize(output->egl_window,
						output->width, output->height,
						0, 0);
			}

			redraw(output);
		}

		/* Batch swap buffer calls after all redraw computations */
		wl_list_for_each_safe(output, tmp, &state.outputs, link)
		{
			if (!output->egl_window || output->frame_callback) {
				continue;
			}

			if (!eglMakeCurrent(state.egl_display,
					    output->egl_surface,
					    output->egl_surface,
					    state.egl_context)) {
				fprintf(stderr, "Failed to make current\n");
				exit(EXIT_FAILURE);
			}

			output->frame_callback =
					wl_surface_frame(output->surface);
			wl_callback_add_listener(output->frame_callback,
					&frame_callback_listener, output);

			if (!eglSwapBuffers(state.egl_display,
					    output->egl_surface)) {
				fprintf(stderr, "Failed to swap buffers\n");
				exit(EXIT_FAILURE);
			}
		}
	}

	return ret;
}
