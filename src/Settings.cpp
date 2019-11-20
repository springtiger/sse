#include <sse/Settings.hpp>

namespace sse {

void Settings::read_file(fs::path file) {
  if (!fs::exists(file)) {
    spdlog::error("Error, config file " + file.string() + " does not exist");
    return;
  }

  spdlog::debug("Reading config file: " + file.string());

  config = toml::parse(file);

  auto printer = toml::find(config, "printer");

  spdlog::debug("Printer: " + toml::find<std::string>(printer, "name"));

  auto build_plate = toml::find(printer, "build_plate");
  if (toml::find<bool>(build_plate, "is_circle")) {
    spdlog::debug("Build plate is circular");
  } else {
    spdlog::debug("Build plate is rectangular");
  }
}

template <typename T> T Settings::get_setting(std::string setting) {

  return toml::find<T>(config, setting);
}

/**
 * @brief Settings::dump
 * @return A string representation of all current settings
 */
std::string Settings::dump() {

}

} // namespace sse
