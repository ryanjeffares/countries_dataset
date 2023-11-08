#include <fmt/core.h>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

int main()
{
  namespace fs = std::filesystem;
  using namespace nlohmann;

  fs::path iso3166DataPath = "/Users/ryanjeffares/Documents/masters/COMP40610/Assignment 2/dataset/countries_dataset/iso3166.json";

  json outJson, iso3166Json;
  outJson["countries"] = nlohmann::json::array();

  iso3166Json = json::parse(std::ifstream{iso3166DataPath});

  for (const auto& country : iso3166Json) {
    auto countryCode = std::stoi(country["country-code"].get<std::string>());
    auto data = country;
    data["country-code"] = countryCode;
    outJson["countries"].push_back(data);
  }
  
  std::ofstream outStream{"/Users/ryanjeffares/Documents/masters/COMP40610/Assignment 2/dataset/countries_dataset/iso3166.json", std::ios::out | std::ios::trunc};
  outStream << std::setw(4) << outJson;
}
