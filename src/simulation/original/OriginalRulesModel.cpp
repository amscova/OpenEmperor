#include "simulation/original/OriginalRulesModel.h"

#include <algorithm>
#include <array>
#include <stdexcept>

namespace openemperor::original {
namespace {

constexpr std::array<ModelBuilding,7> buildings{
    ModelBuilding::ClayPit,ModelBuilding::Kiln,ModelBuilding::Warehouse,
    ModelBuilding::CommonMarketSquare,ModelBuilding::Well,ModelBuilding::Farmhouse,
    ModelBuilding::HerbalistsStall};

ModelProvenance checked_provenance(const ModelData& model,Difficulty difficulty) {
    if (static_cast<std::size_t>(difficulty)>=difficulty_count)
        throw std::out_of_range("invalid original rules model difficulty");
    const auto& source=model.provenance();
    if (source.format_version!=model_schema_version || source.source_kind!=ModelSourceKind::File ||
        source.source_path.empty() || source.sha256.size()!=64 ||
        !std::all_of(source.sha256.begin(),source.sha256.end(),[](char c) {
            return (c>='0' && c<='9') || (c>='a' && c<='f');
        }) || source.byte_count==0 || source.byte_count>max_model_bytes)
        throw std::invalid_argument("original rules model requires valid loaded file provenance");
    return source;
}

std::array<BuildingRow,buildings.size()> prepare_buildings(const ModelData& model,Difficulty difficulty) {
    std::array<BuildingRow,buildings.size()> rows{};
    for (std::size_t row=0;row<buildings.size();++row) {
        if (!model.has_building(model_id(buildings[row])))
            throw std::invalid_argument("original rules model is missing named building: "+
                std::string(model_label(buildings[row])));
        for (std::size_t field=0;field<building_field_count;++field)
            rows[row][field]=original::building_value(model,buildings[row],
                static_cast<BuildingField>(field),difficulty);
    }
    return rows;
}

std::size_t building_index(ModelBuilding building) {
    const auto found=std::find(buildings.begin(),buildings.end(),building);
    if (found==buildings.end()) throw std::out_of_range("invalid prepared original building ID");
    return static_cast<std::size_t>(found-buildings.begin());
}

} // namespace

OriginalRulesModel::OriginalRulesModel(const ModelData& model,Difficulty difficulty)
    : provenance_(checked_provenance(model,difficulty)),difficulty_(difficulty),
      house_rules_(model,difficulty),building_rows_(prepare_buildings(model,difficulty)) {}

std::shared_ptr<const OriginalRulesModel> OriginalRulesModel::prepare(ModelData model,Difficulty difficulty) {
    return std::shared_ptr<const OriginalRulesModel>(new OriginalRulesModel(model,difficulty));
}

std::int32_t OriginalRulesModel::building_value(ModelBuilding building,BuildingField field) const {
    const auto column=static_cast<std::size_t>(field);
    if (column>=building_field_count) throw std::out_of_range("invalid prepared original building field");
    return building_rows_.at(building_index(building)).at(column);
}

} // namespace openemperor::original
