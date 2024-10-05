#include "taunt.h"

#include <vector>

#include "DataObjectGenerator.h"

#include "common/util/FileUtil.h"
#include "common/util/json_util.h"
#include <common/util/Assert.h>

#include "fmt/core.h"

struct Taunt {
  std::string menu_name;
  std::string ambient_name;
  s32 menu_idx;
  s32 play_idx;
  bool valid;
};

void from_json(const nlohmann::json& j, Taunt& taunt) {
  j.at("menu_name").get_to(taunt.menu_name);
  j.at("ambient_name").get_to(taunt.ambient_name);
  j.at("menu_idx").get_to(taunt.menu_idx);
  j.at("play_idx").get_to(taunt.play_idx);
  j.at("valid").get_to(taunt.valid);
}

void to_json(nlohmann::json& j, const Taunt& taunt) {
  j = {
      {"menu_name", taunt.menu_name}, {"ambient_name", taunt.ambient_name},
      {"menu_idx", taunt.menu_idx},   {"play_idx", taunt.play_idx},
      {"valid", taunt.valid},
  };
}

void generate_taunt(const Taunt& taunt, DataObjectGenerator& gen) {
  gen.add_ref_to_string_in_pool(taunt.menu_name);
  gen.add_ref_to_string_in_pool(taunt.ambient_name);
  gen.add_word(taunt.menu_idx);
  gen.add_word(0);
  gen.add_word(taunt.play_idx);
  gen.add_word(0);
}

void compile_taunts(const std::string& file_name, const std::string& game_version) {
  auto file = file_util::get_file_path({file_name});
  if (!fs::exists(file)) {
    throw std::runtime_error(fmt::format("taunt-info: file {} not found.", file));
  }
  auto json_str = file_util::read_text_file(file_util::get_file_path({file_name}));
  std::vector<Taunt> taunts = parse_commented_json(json_str, "taunts");
  if (taunts.size() != 16) {
    throw std::runtime_error("taunt-info: taunt size must be 16!");
  }
  for (auto& t : taunts) {
    if (!t.valid) {
      throw std::runtime_error(fmt::format("taunt-info: taunt {} is invalid.", t.menu_name));
    }
  }
  DataObjectGenerator gen;
  gen.add_type_tag("taunt-info");
  gen.add_word(taunts.size());
  for (auto& t : taunts) {
    generate_taunt(t, gen);
  }
  auto result = gen.generate_v4();
  file_util::write_binary_file(
      file_util::get_jak_project_dir() / "out" / game_version / "obj" / "taunt-info.go",
      result.data(), result.size());
}
