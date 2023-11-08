#include <nlohmann/json.hpp>

#include <rapidcsv.h>

#include <filesystem>
#include <iostream>

int main()
{
  namespace fs = std::filesystem;

  fs::path csvPath = "/Users/ryanjeffares/Documents/masters/COMP40610/Assignment 2/dataset/csv-transform/food_consumption.csv";
  rapidcsv::Document csvDoc{csvPath, rapidcsv::LabelParams{0, 0}};

  nlohmann::json outJson, countriesJson;

  countriesJson = nlohmann::json::parse(std::ifstream{"/Users/ryanjeffares/Documents/masters/COMP40610/Assignment 2/dataset/countries_dataset/iso3166.json"});
  const auto& countriesData = countriesJson["countries"];

  outJson["countries"] = nlohmann::json();
  auto& countries = outJson["countries"];

  auto rowNames = csvDoc.GetRowNames();
  for (auto i = 0zu; i < rowNames.size(); i++) {
    const auto row = csvDoc.GetRow<std::string>(i);

    const auto& country = rowNames[i];
    const auto& category = row[0];

    const auto consumption = std::stod(row[1]);
    const auto emmissions = std::stod(row[2]);

    countries[country][category]["consumption"] = consumption;
    countries[country][category]["emmissions"] = emmissions;

    auto countryData = std::find_if(countriesData.begin(), countriesData.end(), [&country] (const auto& data) {
        return data["name"] == country;
      });
    if (countryData == countriesData.end()) {
      std::cout << "Couldn't find code for " << country << std::endl;
    } else {
      countries[country]["country-code"] = (*countryData)["country-code"];
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

  std::ofstream outStream{"/Users/ryanjeffares/Documents/masters/COMP40610/Assignment 2/dataset/csv-transform/food_consumption.json"};
  outStream << std::setw(4) << finalJson;
}
