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
  int iso3166Code;
  std::string countryCode;
  std::string region;
  std::string subRegion;
};

static const std::map<std::string, CountryData> s_errorCountryCodes = {
  {"USA", { 840, "USA", "Americas", "Northern America" }},
  {"Hong Kong SAR. China", { 344, "HKG", "Asia", "Eastern Asia" }},
  {"United Kingdom", { 826, "GBR", "Europe", "Northern Europe" }},
  {"Venezuela", { 862, "VEN", "Americas", "Latin America and the Caribbean" }},
  {"Russia", { 643, "RUS", "Europe", "Eastern Europe" }},
  {"Bolivia", { 68, "BOL", "Americas", "Latin America and the Caribbean" }},
  {"Czech Republic", { 203, "CZE", "Europe", "Eastern Europe" }},
  {"Swaziland", { 748, "SWZ", "Africa", "Sub-Saharan Africa" }},
  {"South Korea", { 410, "KOR", "Asia", "Eastern Asia" }},
  {"Macedonia", { 807, "MKD", "Europe", "Southern Europe" }},
  {"Taiwan. ROC", { 158, "TWN", "Asia", "Eastern Asia" }},
  {"Iran", { 364, "IRN", "Asia", "Southern Asia" }},
  {"Tanzania", { 834, "TZA", "Africa", "Sub-Saharan Africa" }},
  {"Vietnam", { 704, "VNM", "Asia", "South-eastern Asia" }},
};

static const std::vector<std::string> s_keyNames = {
  "id",
  "name",
  "region",
  "population",
  "total_consumption",
  "total_emissions",
  "Beef_consumption",
  "Beef_emissions",
  "Eggs_consumption",
  "Eggs_emissions",
  "Fish_consumption",
  "Fish_emissions",
  "Lamb_consumption",
  "Lamb_emissions",
  "Milk_consumption",
  "Milk_emissions",
  "Nuts_consumption",
  "Nuts_emissions",
  "Pork_consumption",
  "Pork_emissions",
  "Poultry_consumption",
  "Poultry_emissions",
  "Rice_consumption",
  "Rice_emissions",
  "Soybeans_consumption",
  "Soybeans_emissions",
  "Wheat_consumption",
  "Wheat_emissions",
};

static const std::vector<std::string> s_categoryNames = {
  "Beef",
  "Eggs",
  "Fish",
  "Lamb",
  "Milk",
  "Nuts",
  "Pork",
  "Poultry",
  "Rice",
  "Soybeans",
  "Wheat",
};

/*
id,name,region,population,total_consumption,total_emissions,Beef_consumption,Beef_emissions,Eggs_consumption,Eggs_emissions,Fish_consumption,Fish_emissions,Lamb_consumption,Lamb_emissions,Milk_consumption,Milk_emissions,Nuts_consumption,Nuts_emissions,Pork_consumption,Pork_emissions,Poultry_consumption,Poultry_emissions,Rice_consumption,Rice_emissions,Soybeans_consumption,Soybeans_emissions,Wheat_consumption,Wheat_emissions
*/

int main(int argc, const char* argv[])
{
  if (argc != 5) {
    std::cerr << "csv-transform <csv-path> <countries-json-path> <population-csv-path> <output-json-path>\n";
    return 1;
  }

  namespace fs = std::filesystem;

  fs::path consumptionCsvPath = argv[1];
  fs::path iso3166JsonPath = argv[2];
  fs::path populationCsvPath = argv[3];
  fs::path outputJsonPath = argv[4];

  rapidcsv::Document consumptionCsvDoc{consumptionCsvPath, rapidcsv::LabelParams{0, 0}};
  rapidcsv::Document populationCsvDoc{populationCsvPath, rapidcsv::LabelParams{0, 0}};
  const auto countryCodeList = populationCsvDoc.GetColumn<std::string>("Country Code");
  const auto populationList = populationCsvDoc.GetColumn<int64_t>("2022");

  nlohmann::json outJson, countriesJson;

  countriesJson = nlohmann::json::parse(std::ifstream{iso3166JsonPath});
  const auto& countriesData = countriesJson["countries"];

  outJson["countries"] = nlohmann::json();
  auto& countries = outJson["countries"];

  auto rowNames = consumptionCsvDoc.GetRowNames();
  for (auto i = 0zu; i < rowNames.size(); i++) {
    const auto row = consumptionCsvDoc.GetRow<std::string>(i);
    const auto& countryName = rowNames[i];
    
    auto& countryData = countries[countryName];

    int iso3166Code;
    std::string countryCode;
    std::string region;

    if (s_errorCountryCodes.contains(countryName)) {
      const auto& data = s_errorCountryCodes.at(countryName);
      iso3166Code = data.iso3166Code;
      region = data.region;
      countryCode = data.countryCode;
    } else {
      const auto it = std::find_if(countriesData.begin(), countriesData.end(),
        [&countryName] (const auto& data) {
          return data["name"] == countryName;
        });

      if (it == countriesData.end()) {
        std::cerr << countryName << std::endl;
        continue;
      }

      const auto& data = *it;
      iso3166Code = data["country-code"].get<int>();
      region = data["region"].get<std::string>();
      countryCode = data["alpha-3"].get<std::string>();
    }

    countryData["id"] = iso3166Code;
    countryData["name"] = countryName;
    countryData["region"] = region;

    const auto populationRowIndex = std::distance(countryCodeList.begin(), std::find(countryCodeList.begin(), countryCodeList.end(), countryCode));
    const auto population = populationList[populationRowIndex];

    countryData["population"] = population;

    const auto& categoryName = s_cleanedCategoryNames.contains(row[0zu]) ? s_cleanedCategoryNames.at(row[0zu]) : row[0zu];
    const auto consumption = std::stod(row[1]);
    const auto emissions = std::stod(row[2]);

    if (!countryData.contains("Total")) {
      countryData["Total"]["consumption"] = 0;
      countryData["Total"]["emissions"] = 0;
    }

    countryData["Total"]["consumption"] = countryData["Total"]["consumption"].get<double>() + consumption;
    countryData["Total"]["emissions"] = countryData["Total"]["emissions"].get<double>() + consumption;

    for (const auto& category : s_categoryNames) {
      if (category == categoryName) {
        countryData[category]["consumption"] = consumption;
        countryData[category]["emissions"] = emissions;
        break;
      }
    }
  }

  nlohmann::json finalJson;
  finalJson["countries"] = nlohmann::json::array();
  finalJson["categories"] = nlohmann::json::array();

  for (auto country = countries.begin(); country != countries.end(); country++) {
    const auto& countryName = country.key();
    auto countryData = country.value(); // copy
    countryData["name"] = countryName;
    finalJson["countries"].push_back(countryData);
  }

  for (const auto& category : s_categoryNames) {
    auto data = nlohmann::json();
    auto totalConsumption = 0.0, totalEmissions = 0.0;
    for (const auto& country : finalJson["countries"]) {
      const auto consumption = country[category]["consumption"].get<double>();
      const auto emissions = country[category]["emissions"].get<double>();
      totalConsumption += consumption;
      totalEmissions += emissions;
    }

    data["category"] = category;
    data["emissions_per_kilo_consumption"] = totalEmissions / totalConsumption;

    finalJson["categories"].push_back(data);
  }

  std::ofstream outStream{outputJsonPath};
  outStream << std::setw(4) << finalJson;
}
