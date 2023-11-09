#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>

int main(int argc, const char* argv[])
{
  if (argc != 2) {
    std::cerr << "json-combine <iso3166-json-path>\n";
    return 1;
  }

  namespace fs = std::filesystem;
  using namespace nlohmann;

  fs::path iso3166DataPath = argv[1];

  json outJson, iso3166Json;
  outJson["countries"] = nlohmann::json::array();

  iso3166Json = json::parse(std::ifstream{iso3166DataPath});

  for (const auto& country : iso3166Json) {
    auto countryCode = std::stoi(country["country-code"].get<std::string>());
    auto data = country;
    data["country-code"] = countryCode;
    outJson["countries"].push_back(data);
  }
  
  std::ofstream outStream{argv[1], std::ios::out | std::ios::trunc};
  outStream << std::setw(4) << outJson;
}
