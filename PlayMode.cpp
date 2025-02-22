#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <iostream>

GLuint grid_mesh_for_lit_color_texture_program = 0;
Load< MeshBuffer > grid_mesh(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("connect.pnct"));
	grid_mesh_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

// load initial grid scene
Load< Scene > grid_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("connect.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = grid_mesh->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = grid_mesh_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

PlayMode::PlayMode() : scene(*grid_scene) {
	auto red_erase = scene.drawables.end();
	auto green_erase = scene.drawables.end();
	auto blue_erase = scene.drawables.end();
	// save coin transforms and drawable before erasing them from initial scene
	for (auto it = scene.drawables.begin(); it != scene.drawables.end(); ++it){
		Scene::Drawable &drawable = *it;
		if(drawable.transform->name == "Red") {
			// no parent
			assert(drawable.transform->parent == nullptr);
			red_draw = drawable;
			red_erase = it;
		}
		if(drawable.transform->name == "Green") {
			// no parent
			assert(drawable.transform->parent == nullptr);
			green_draw = drawable;
			green_erase = it;
		}
		if(drawable.transform->name == "Blue") {
			// no parent
			assert(drawable.transform->parent == nullptr);
			blue_draw = drawable;
			blue_erase = it;
		}
	}
	// double check before erasing the drawables in initial scene
	if (red_erase == scene.drawables.end()) throw std::runtime_error("Red coin not found.");
	if (green_erase == scene.drawables.end()) throw std::runtime_error("Green coin not found.");
	if (blue_erase == scene.drawables.end()) throw std::runtime_error("Blue coin not found.");
	scene.drawables.erase(red_erase);
	scene.drawables.erase(green_erase);
	scene.drawables.erase(blue_erase);

	// add a red coin above column 2
	//std::cout << "initial pos" << red_draw.transform->position.x << " " << red_draw.transform->position.y << " " << red_draw.transform->position.z << "\n";
	scene.transforms.emplace_back();
	Scene::Transform &transform = scene.transforms.back();
	transform.position = glm::vec3(-1.2f, 0.0f, 10.8f);
	transform.name = "Red";
	Scene::Drawable initial_red = Scene::Drawable(&transform);
	initial_red.pipeline = red_draw.pipeline;
	scene.drawables.push_back(initial_red);
	//std::cout << "after pos" << red_draw.transform->position.x << " " << red_draw.transform->position.y << " " << red_draw.transform->position.z << "\n";

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
}

PlayMode::~PlayMode() {
}

std::string PlayMode::name(int coin) {
	if(coin % 6 == 0 || coin % 6 == 5) {
		return "RED";
	}
	else if(coin % 6 == 1 || coin % 6 == 4) {
		return "GREEN";
	}
	else {
		return "BLUE";
	}
}

 bool PlayMode::check_win(int row, int col) {
	// check up 3
	if(row <= 1) {
		if(arr[row][col] == arr[row + 1][col] && arr[row][col] == arr[row + 2][col]) {
			winner = name(current_coin);
			return true;
		}
	}
	// check up right 3
	if(row <= 1 && col <= 3) {
		if(arr[row][col] == arr[row + 1][col + 1] && arr[row][col] == arr[row + 2][col + 2]) {
			winner = name(current_coin);
			return true;
		}
	}
	// check right 3
	if(col <= 3) {
		if(arr[row][col] == arr[row][col + 1] && arr[row][col] == arr[row][col + 2]) {
			winner = name(current_coin);
			return true;
		}
	}
	// check bot right 3
	if(row >= 2 && col <= 3) {
		if(arr[row][col] == arr[row - 1][col + 1] && arr[row][col] == arr[row - 2][col + 2]) {
			winner = name(current_coin);
			return true;
		}
	}
	// check bot 3
	if(row >= 2) {
		if(arr[row][col] == arr[row - 1][col] && arr[row][col] == arr[row - 2][col]) {
			winner = name(current_coin);
			return true;
		}
	}
	// check bot left 3
	if(row >= 2 && col >= 2) {
		if(arr[row][col] == arr[row - 1][col - 1] && arr[row][col] == arr[row - 2][col - 2]) {
			winner = name(current_coin);
			return true;
		}
	}
	// check left 3
	if(col >= 2) {
		if(arr[row][col] == arr[row][col - 1] && arr[row][col] == arr[row][col - 2]) {
			winner = name(current_coin);
			return true;
		}
	}
	// check up left 3
	if (row <= 1 && col >= 2) {
		if(arr[row][col] == arr[row + 1][col - 1] && arr[row][col] == arr[row + 2][col - 2]) {
			winner = name(current_coin);
			return true;
		}
	}

	// check middle 3 top, down
	if (1 <= row && row <= 2) {
		if(arr[row][col] == arr[row + 1][col] && arr[row][col] == arr[row - 1][col]) {
			winner = name(current_coin);
			return true;
		}
	}
	// check middle 3 left, right
	if (1 <= col && col <= 4) {
		if(arr[row][col] == arr[row][col + 1] && arr[row][col] == arr[row][col - 1]) {
			winner = name(current_coin);
			return true;
		}
	}
	// check middle 3 top left, down right
	if (1 <= row && row <= 2 && 1 <= col && col <= 4) {
		if(arr[row][col] == arr[row - 1][col + 1] && arr[row][col] == arr[row + 1][col - 1]) {
			winner = name(current_coin);
			return true;
		}
	}
	// check middle 3 top right, down left
	if (1 <= row && row <= 2 && 1 <= col && col <= 4) {
		if(arr[row][col] == arr[row - 1][col - 1] && arr[row][col] == arr[row + 1][col + 1]) {
			winner = name(current_coin);
			return true;
		}
	}

	return false;
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN && !finished) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			a_left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			d_right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			w_up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			s_down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.downs += 1;
			space.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP && !finished) {
		if (evt.key.keysym.sym == SDLK_a) {
			a_left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			d_right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			w_up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			s_down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs = 0;
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs = 0;
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.downs = 0;
			space.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN && !finished) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION && !finished) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	if(finished) {
		for(auto &trans : scene.transforms) {
			if (trans.name == "Red" || trans.name == "Green" || trans.name == "Blue") {
				trans.rotation = trans.rotation * glm::angleAxis(
					glm::radians(20 * elapsed),
					glm::vec3(0.0f, 0.0f, 1.0f));
			}
		}
	}
	// solutions to movement: whenever press space, round to nearest column
	if(left.pressed && left.downs == 1) {
		Scene::Drawable &drawable = scene.drawables.back();
		// make sure it's a coin
		assert(((drawable.transform->name == "Red") && (current_coin % 6 == 0 || current_coin % 6 == 5)) ||
			   ((drawable.transform->name == "Green") && (current_coin % 6 == 1 || current_coin % 6 == 4)) ||
			   ((drawable.transform->name == "Blue") && (current_coin % 6 == 2 || current_coin % 6 == 3)));
		// float x_coord = drawable.transform->position.x;
		// drawable.transform->position = glm::vec3(std::max(-2.4f * elapsed + x_coord, -6.0f), drawable.transform->position.y, drawable.transform->position.z);
		// column = std::max(column + elapsed, 0.0f);
		if(column > 0) {
			drawable.transform->position += glm::vec3(-2.4f, 0.f, 0.f);
			column -= 1;
		}
		left.downs += 1;
	}
	if(right.pressed && right.downs == 1) {
		Scene::Drawable &drawable = scene.drawables.back();
		// make sure it's a coin
		assert(((drawable.transform->name == "Red") && (current_coin % 6 == 0 || current_coin % 6 == 5)) ||
			   ((drawable.transform->name == "Green") && (current_coin % 6 == 1 || current_coin % 6 == 4)) ||
			   ((drawable.transform->name == "Blue") && (current_coin % 6 == 2 || current_coin % 6 == 3)));
		// float x_coord = drawable.transform->position.x;
		// drawable.transform->position = glm::vec3(std::min(2.4f * elapsed + x_coord, 6.0f), drawable.transform->position.y, drawable.transform->position.z);
		// column = std::min(column + elapsed, 5.0f);
		if(column < 5) {
			drawable.transform->position += glm::vec3(2.4f, 0.f, 0.f);
			column += 1;
		}
		right.downs += 1;
	}
	if(space.pressed && space.downs == 1) {
		// place coin into array and grid 
		int col = column;
		int row = heights[col];
		if(row < 4) {
			heights[col] += 1;
			int val = current_coin % 6;
			if (val == 3) {
				val = 2;
			}
			else if (val == 4) {
				val = 1;
			}
			else if (val == 5) {
				val = 0;
			}
			arr[row][col] = val;
			Scene::Transform &transform = scene.transforms.back();
			transform.position = glm::vec3(-6.0f + 2.4f * col, 0.0f, 1.2f + 2.4f * row);
			if(current_coin >= 6) {
				finished = check_win(row, col);
				if(finished) {
					a_left.pressed = false;
					w_up.pressed = false;
					d_right.pressed = false;	
					s_down.pressed = false;	
					left.pressed = false;	
					right.pressed = false;					
					space.pressed = false;	
					left.downs = 0;
					right.downs = 0;
					space.downs = 0;
				}
			}
			if (current_coin >= 23) {
				drawed = true;
				finished = true;
			} 
			// spawn next coin
			if(!finished) {
				current_coin += 1;

				scene.transforms.emplace_back();
				Scene::Transform &trans = scene.transforms.back();
				trans.position = glm::vec3(-6.0f + 2.4f * col, 0.0f, 10.8f);
				Scene::Drawable next = Scene::Drawable(&trans);
				if(current_coin % 6 == 0 || current_coin % 6 == 5) {
					trans.name = "Red";
					next.pipeline = red_draw.pipeline;
				}
				else if(current_coin % 6 == 1 || current_coin % 6 == 4) {
					trans.name = "Green";
					next.pipeline = green_draw.pipeline;
				}
				else {
					trans.name = "Blue";
					next.pipeline = blue_draw.pipeline;
				}
				scene.drawables.push_back(next);
				// ensures we only place one coin down at a time
				space.downs += 1;
			}
		}
	}

	//move camera:
	{
		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (a_left.pressed && !d_right.pressed) move.x =-1.0f;
		if (!a_left.pressed && d_right.pressed) move.x = 1.0f;
		if (s_down.pressed && !w_up.pressed) move.y =-1.0f;
		if (!s_down.pressed && w_up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 forward = -frame[2];

		camera->transform->position += move.x * right + move.y * forward;
	}
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));

		if(drawed && finished) {
			lines.draw_text("DRAW",
				glm::vec3(-0.4f, -0.1f, 0.0f),
				glm::vec3(0.4, 0.0f, 0.0f), glm::vec3(0.0f, 0.4, 0.0f),
				glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		}
		else if (finished) {
			lines.draw_text("WINNER IS " + winner,
				glm::vec3(-1.0f, -0.2f, 0.0f),
				glm::vec3(0.4f, 0.0f, 0.0f), glm::vec3(0.0f, 0.4f, 0.0f),
				glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		}
	}
}
