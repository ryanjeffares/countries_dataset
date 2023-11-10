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
    auto& values = country[category];

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

    auto countryData = std::find_if(countriesData.begin(), countriesData.end(),
        [&countryName] (const auto& data) {
          return data["name"] == countryName; 
        });

    if (countryData == countriesData.end()) {
      if (countryName == "USA") {
        countries[countryName]["country-code"] = 840;
      } else if (countryName == "Hong Kong SAR. China") {
        countries[countryName]["country-code"] = 344;
      } else if (countryName == "United Kingdom") {
        countries[countryName]["country-code"] = 826;
      } else if (countryName == "Venezuela") {
        countries[countryName]["country-code"] = 862;
      } else if (countryName == "Russia") {
        countries[countryName]["country-code"] = 643;
      } else if (countryName == "Bolivia") {
        countries[countryName]["country-code"] = 68;
      } else if (countryName == "Czech Republic") {
        countries[countryName]["country-code"] = 203;
      } else if (countryName == "Swaziland") {
        countries[countryName]["country-code"] = 748;
      } else if (countryName == "South Korea") {
        countries[countryName]["country-code"] = 410;
      } else if (countryName == "Macedonia") {
        countries[countryName]["country-code"] = 807;
      } else if (countryName == "Taiwan. ROC") {
        countries[countryName]["country-code"] = 158;
      } else if (countryName == "Iran") {
        countries[countryName]["country-code"] = 364;
      } else if (countryName == "Tanzania") {
        countries[countryName]["country-code"] = 834;
      } else if (countryName == "Vietnam") {
        countries[countryName]["country-code"] = 704;
      } else {
        std::cerr << "Couldn't find country code for " << countryName << std::endl;
      }
    } else {
      countries[countryName]["country-code"] = (*countryData)["country-code"];
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
