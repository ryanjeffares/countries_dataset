#include <nlohmann/json.hpp>

#include <rapidcsv.h>

#include <filesystem>
#include <iostream>
#include <map>

static const std::map<std::string, std::string> s_cleanedCategoryNames = {
  {"Lamb & Goat", "Lamb"},
  {"Milk - inc. cheese", "Milk"},
  {"Nuts inc. Peanut Butter", "Nuts"},
  {"Wheat and Wheat Products", "Wheat"},
};

struct CountryData
{
  int countryCode;
  std::string region;
  std::string subRegion;
};

static const std::map<std::string, CountryData> s_errorCountryCodes = {
  {"USA", { 840, "Americas", "Northern America" }},
  {"Hong Kong SAR. China", { 344, "Asia", "Eastern Asia" }},
  {"United Kingdom", { 826, "Europe", "Northern Europe" }},
  {"Venezuela", { 862, "Americas", "Latin America and the Caribbean" }},
  {"Russia", { 643, "Europe", "Eastern Europe" }},
  {"Bolivia", { 68, "Americas", "Latin America and the Caribbean" }},
  {"Czech Republic", { 203, "Europe", "Eastern Europe" }},
  {"Swaziland", { 748, "Africa", "Sub-Saharan Africa" }},
  {"South Korea", { 410, "Asia", "Eastern Asia" }},
  {"Macedonia", { 807, "Europe", "Southern Europe" }},
  {"Taiwan. ROC", { 158, "Asia", "Eastern Asia" }},
  {"Iran", { 364, "Asia", "Southern Asia" }},
  {"Tanzania", { 834, "Africa", "Sub-Saharan Africa" }},
  {"Vietnam", { 704, "Asia", "South-eastern Asia" }},
};

int main(int argc, const char* argv[])
{
  if (argc != 4) {
    std::cerr << "csv-transform <csv-path> <countries-json-path> <output-json-path>\n";
    return 1;
  }

  namespace fs = std::filesystem;

  fs::path csvPath = argv[1];
  rapidcsv::Document csvDoc{csvPath, rapidcsv::LabelParams{0, 0}};

  nlohmann::json outJson, countriesJson;

  countriesJson = nlohmann::json::parse(std::ifstream{argv[2]});
  const auto& countriesData = countriesJson["countries"];

  outJson["countries"] = nlohmann::json();
  auto& countries = outJson["countries"];

  auto rowNames = csvDoc.GetRowNames();
  for (auto i = 0zu; i < rowNames.size(); i++) {
    const auto row = csvDoc.GetRow<std::string>(i);

    const auto& countryName = rowNames[i];
    const auto& category = s_cleanedCategoryNames.contains(row[0]) ? s_cleanedCategoryNames.at(row[0]) : row[0];

    const auto consumption = std::stod(row[1]);
    const auto emissions = std::stod(row[2]);

    auto& country = countries[countryName];
    auto& values = country["categories"][category];

    values["consumption"] = consumption;
    values["emissions"] = emissions;

    if (!country.contains("total_emissions")) {
      country["total_emissions"] = 0.0;
    }

    if (!country.contains("total_consumption")) {
      country["total_consumption"] = 0.0;
    }

    country["total_consumption"] = country["total_consumption"].get<double>() + consumption;
    country["total_emissions"] = country["total_emissions"].get<double>() + emissions;

    if (s_errorCountryCodes.contains(countryName)) {
      countries[countryName]["country-code"] = s_errorCountryCodes.at(countryName).countryCode;
      countries[countryName]["region"] = s_errorCountryCodes.at(countryName).region;
      countries[countryName]["sub-region"] = s_errorCountryCodes.at(countryName).subRegion;
    } else {
      auto countryData = std::find_if(countriesData.begin(), countriesData.end(),
        [&countryName] (const auto& data) {
          return data["name"] == countryName; 
        });
      countries[countryName]["country-code"] = (*countryData)["country-code"];
      countries[countryName]["region"] = (*countryData)["region"];
      countries[countryName]["sub-region"] = (*countryData)["sub-region"];
    }
  }

  nlohmann::json finalJson;
  finalJson["countries"] = nlohmann::json::array();

  for (auto country = countries.begin(); country != countries.end(); country++) {
    auto countryName = country.key();
    auto countryData = country.value();
    countryData["name"] = countryName;
    finalJson["countries"].push_back(countryData);
  }

  std::ofstream outStream{argv[3]};
  outStream << std::setw(4) << finalJson;
}
