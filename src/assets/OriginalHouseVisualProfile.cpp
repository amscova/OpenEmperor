#include "assets/BuildingVisualProfile.h"
#include "assets/Sg3ImageLoader.h"
#include "core/PerformanceDiagnostics.h"
#include "maps/ResourceGroupLookup.h"

#include <nlohmann/json.hpp>
#include <openssl/evp.h>
#include <algorithm>
#include <array>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>

namespace openemperor::assets {
namespace {
namespace fs=std::filesystem;
using Json=nlohmann::json;
[[noreturn]] void fail(const std::string& message) {
    throw std::runtime_error("Original House profile: "+message);
}
void keys(const Json& value,std::initializer_list<const char*> allowed) {
    if (!value.is_object()) fail("expected an object");
    for (auto it=value.begin();it!=value.end();++it)
        if (std::none_of(allowed.begin(),allowed.end(),[&](const char* name){return it.key()==name;}))
            fail("unknown key "+it.key());
}
std::string string(const Json& value,const char* name,std::size_t limit=512) {
    if (!value.contains(name) || !value.at(name).is_string()) fail(std::string("missing string ")+name);
    const auto result=value.at(name).get<std::string>();
    if (result.empty() || result.size()>limit) fail(std::string("invalid string ")+name);
    return result;
}
std::uint64_t number(const Json& value,const char* name,std::uint64_t maximum) {
    if (!value.contains(name) || !value.at(name).is_number_unsigned()) fail(std::string("invalid unsigned ")+name);
    const auto result=value.at(name).get<std::uint64_t>();
    if (result>maximum) fail(std::string("out of range ")+name);
    return result;
}
fs::path relative(const std::string& name) {
    const fs::path path(name);
    if (path.empty() || path.is_absolute() || path.has_root_name() || name.find('\\')!=std::string::npos)
        fail("unsafe relative asset path");
    for (const auto& part:path) if (part.empty() || part=="." || part=="..") fail("asset path traversal");
    return path;
}
fs::path checked_file(const fs::path& root,const fs::path& path) {
    auto current=root;
    for (const auto& part:path) {
        current/=part;
        std::error_code error;
        const auto status=fs::symlink_status(current,error);
        if (error || fs::is_symlink(status)) fail("missing or linked asset "+path.generic_string());
    }
    std::error_code error;const auto result=fs::canonical(current,error);
    if (error || !fs::is_regular_file(result)) fail("missing asset "+path.generic_string());
    const auto mismatch=std::mismatch(root.begin(),root.end(),result.begin(),result.end());
    if (mismatch.first!=root.end()) fail("asset escapes data root");
    return result;
}
std::string sha256(const fs::path& path) {
    using Context=std::unique_ptr<EVP_MD_CTX,decltype(&EVP_MD_CTX_free)>;
    Context context(EVP_MD_CTX_new(),EVP_MD_CTX_free);
    if (!context || EVP_DigestInit_ex(context.get(),EVP_sha256(),nullptr)!=1) fail("SHA256 initialization");
    std::ifstream input(path,std::ios::binary);if (!input) fail("fingerprint source unavailable");
    performance::increment(performance::Counter::FileReads);
    std::array<char,65536> buffer{};
    while (input) {
        input.read(buffer.data(),static_cast<std::streamsize>(buffer.size()));
        const auto count=input.gcount();
        if (count>0 && EVP_DigestUpdate(context.get(),buffer.data(),static_cast<std::size_t>(count))!=1)
            fail("SHA256 update");
    }
    if (!input.eof()) fail("fingerprint source unreadable");
    std::array<unsigned char,EVP_MAX_MD_SIZE> bytes{};unsigned length=0;
    if (EVP_DigestFinal_ex(context.get(),bytes.data(),&length)!=1 || length!=32) fail("SHA256 finalization");
    std::ostringstream result;result<<std::hex<<std::setfill('0');
    for (unsigned i=0;i<length;++i) result<<std::setw(2)<<static_cast<unsigned>(bytes[i]);
    return result.str();
}
Json read_manifest(const fs::path& path) {
    std::error_code error;const auto size=fs::file_size(path,error);
    if (error || !fs::is_regular_file(path) || size>1024U*1024U) fail("metadata missing or exceeds1MiB");
    std::ifstream input(path,std::ios::binary);if (!input) fail("metadata unavailable");
    performance::increment(performance::Counter::FileReads);
    std::vector<std::set<std::string>> names;
    return Json::parse(input,[&](int,Json::parse_event_t event,Json& value) {
        if (event==Json::parse_event_t::object_start) names.emplace_back();
        else if (event==Json::parse_event_t::key && !names.back().insert(value.get<std::string>()).second)
            fail("duplicate metadata key");
        else if (event==Json::parse_event_t::object_end) names.pop_back();
        return true;
    });
}
std::uint64_t image_bytes(const RgbaImage& image) {
    const auto bytes=std::uint64_t(image.width)*image.height*4U;
    if (image.pixels.size()!=bytes) fail("existing image size differs from metadata");
    return bytes;
}
std::optional<std::uint32_t> evidenced_group(std::uint64_t type) {
    // Pinned House setter518de0, not the generic Building defaults. Keep the
    // semantic selector guard independent of the metadata's physical records.
    // Vacant2's constructor/setter ambiguity has no admitted sprite identity.
    if (type>=3 && type<=10) return 0x471U+static_cast<std::uint32_t>(type-3);
    switch (type) {
    case 11:return 0x479U;
    case 12:return 0x47bU;
    case 13:return 0x47cU;
    case 14:return 0x4acU;
    default:if (type>=15 && type<=17) return 0x47eU+static_cast<std::uint32_t>(type-15);
    }
    return std::nullopt;
}
}

void append_original_house_visual_profile(const fs::path& data_root,const fs::path& manifest,
    BuildingVisualProfile& profile,std::uint64_t max_final_rgba) {
    if (profile.original_house_prepared) fail("remove an existing extension before replacement");
    max_final_rgba=std::min(max_final_rgba,building_max_rgba_bytes);
    std::uint64_t total=0;
    for (const auto& image:profile.unique_images) {
        const auto bytes=image_bytes(image);
        if (bytes>max_final_rgba-total) fail("combined building RGBA budget exceeded");
        total+=bytes;
    }
    std::error_code error;const auto root=fs::canonical(data_root,error);
    if (error || !fs::is_directory(root)) fail("invalid data root");
    const auto json=read_manifest(manifest);
    keys(json,{"schema_version","mode","selection","anchor_policy","evidence","files","stages"});
    if (number(json,"schema_version",1)!=1 || string(json,"mode")!="original_house_stages" ||
        string(json,"selection")!="canonical_static_variant_0" || string(json,"anchor_policy")!="type30_front_cell")
        fail("unsupported schema, selection or anchor policy");
    (void)string(json,"evidence");
    if (!json.contains("files") || !json.at("files").is_array() || json.at("files").empty() || json.at("files").size()>16)
        fail("bounded asset fingerprints required");
    std::map<fs::path,fs::path> verified;
    for (const auto& file:json.at("files")) {
        keys(file,{"path","sha256"});const auto path=relative(string(file,"path",4096));
        const auto hash=string(file,"sha256",64);
        if (hash.size()!=64 || !std::all_of(hash.begin(),hash.end(),[](char c){return (c>='0' && c<='9') || (c>='a' && c<='f');}))
            fail("invalid lowercase SHA256");
        const auto actual=checked_file(root,path);
        if (!verified.emplace(path,actual).second) fail("duplicate fingerprint path");
        if (sha256(actual)!=hash) fail("fingerprint mismatch for "+path.generic_string());
    }
    if (!json.contains("stages") || !json.at("stages").is_array() || json.at("stages").size()!=original_house_type_count)
        fail("exactly16 explicit House stages required");
    std::array<bool,original_house_type_count> seen{};
    auto candidate=profile;
    candidate.original_house_entries.fill(std::nullopt);
    candidate.original_house_fallbacks.fill({});
    std::map<fs::path,Sg3Archive> archives;
    std::map<fs::path,maps::RuntimeArchiveLayout> layouts;
    std::vector<std::optional<AssetId>> ids(candidate.unique_images.size());
    for (const auto& entry:candidate.entries) if (entry) {
        if (entry->image_index>=ids.size()) fail("existing role image index invalid");
        if (ids[entry->image_index] && ids[entry->image_index]!=entry->id) fail("existing physical identity conflict");
        ids[entry->image_index]=entry->id;
    }
    for (const auto& stage:json.at("stages")) {
        keys(stage,{"house_type","original_side","fallback","archive","image_index","width","height","base_bytes","ground_anchor","group_key","variant","evidence"});
        const auto type=number(stage,"house_type",17);
        if (type<original_house_first_type) fail("House type outside2..17");
        const auto index=static_cast<std::size_t>(type-original_house_first_type);
        if (seen[index]) fail("duplicate House type");seen[index]=true;
        const auto side=number(stage,"original_side",4);
        if (side!=(type<11 ? 2U:4U)) fail("House type/family side mismatch");
        candidate.original_house_fallbacks[index]=string(stage,"fallback");
        if (!stage.contains("archive")) {
            if (stage.size()!=3) fail("fallback stage must not declare partial sprite metadata");
            continue;
        }
        BuildingVisualEntry entry;entry.id.archive_relative_path=relative(string(stage,"archive",4096));
        if (entry.id.archive_relative_path.extension()!=".sg3") fail("House archive must be.sg3");
        const auto physical=number(stage,"image_index",UINT32_MAX);
        if (!physical) fail("physical record zero is not a House body");
        entry.id.image_index=static_cast<std::uint32_t>(physical);
        const auto width=number(stage,"width",4096),height=number(stage,"height",4096);
        const auto base=number(stage,"base_bytes",51200);
        if (width!=80U*side-2U || height<40U*side || base!=3200U*side*side)
            fail("House native Type30 geometry mismatch");
        if (!stage.contains("ground_anchor") || !stage.at("ground_anchor").is_array() || stage.at("ground_anchor").size()!=2)
            fail("House anchor missing");
        const auto& anchor=stage.at("ground_anchor");
        if (!anchor[0].is_number_integer() || !anchor[1].is_number_integer() ||
            anchor[0].get<std::int64_t>()!=static_cast<std::int64_t>(40U*side-1U) ||
            anchor[1].get<std::int64_t>()!=static_cast<std::int64_t>(height-20U))
            fail("House anchor differs from evidenced native front cell");
        entry.ground_x=static_cast<double>(40U*side-1U);entry.ground_y=static_cast<double>(height-20U);
        entry.footprint_side=static_cast<std::uint8_t>(side);entry.evidence=string(stage,"evidence");
        const auto group=number(stage,"group_key",0x7fffffffU);
        const auto variant=number(stage,"variant",1);
        const auto expected=evidenced_group(type);
        if (!expected || group!=*expected || variant!=0)
            fail("unsupported House registration/static variant");
        const auto verified_archive=verified.find(entry.id.archive_relative_path);
        if (verified_archive==verified.end()) fail("House archive lacks fingerprint");
        auto found_archive=archives.find(entry.id.archive_relative_path);
        if (found_archive==archives.end()) found_archive=archives.emplace(entry.id.archive_relative_path,
            read_sg3_archive(verified_archive->second)).first;
        const auto& archive=found_archive->second;
        auto layout=layouts.find(entry.id.archive_relative_path);
        if (layout==layouts.end()) {
            auto prepared=maps::build_runtime_archive_layout(2,archive,maps::RuntimeLayoutEvidence::TerrainElevationAndGeneral2);
            if (!prepared) fail("House archive registration unsupported");
            layout=layouts.emplace(entry.id.archive_relative_path,std::move(*prepared)).first;
        }
        const auto resolution=maps::resolve_resource_group({static_cast<std::uint32_t>(group)},{{2,{&layout->second}}});
        const auto packed=resolution.packed_base ? maps::group_variant(*resolution.packed_base,static_cast<std::uint32_t>(variant)):std::nullopt;
        const auto resolved=packed ? layout->second.physical_record_for_local(packed->value & 0x3fffU):std::nullopt;
        if (resolution.status!=maps::GroupLookupStatus::Resolved || !resolved || *resolved!=physical)
            fail("House physical record differs from registered group/variant");
        if (physical>=archive.images.size()) fail("House physical record outside SG3 table");
        const auto& record=archive.images[static_cast<std::size_t>(physical)];
        if (record.image_type!=30 || record.width!=static_cast<int>(width) || record.height!=static_cast<int>(height) ||
            record.isometric_size_flag!=side || record.uncompressed_length!=base ||
            record.horizontal_mirror_offset || record.animation_sprites || record.animation_x_offset ||
            record.animation_y_offset || record.reversible_animation_flag || record.animation_speed_id ||
            record.alpha_offset || record.alpha_length || !record.data_length) fail("House SG3 metadata mismatch");
        const auto bitmap=resolve_sg3_image_bitmap(verified_archive->second,archive,record);
        if (bitmap.status!=Sg3BitmapStatus::Resolved) fail("House bitmap unavailable or unsafe");
        const auto source=fs::relative(bitmap.path,root,error);
        if (error || verified.find(source)==verified.end() || checked_file(root,source)!=bitmap.path)
            fail("House bitmap lacks contained fingerprint");
        const auto existing=std::find_if(ids.begin(),ids.end(),[&](const auto& id){return id && *id==entry.id;});
        if (existing!=ids.end()) {
            entry.image_index=static_cast<std::size_t>(existing-ids.begin());
            const auto& image=candidate.unique_images.at(entry.image_index);
            if (image.width!=width || image.height!=height) fail("shared House image metadata mismatch");
        } else {
            const auto bytes=width*height*4U;
            if (bytes>max_final_rgba-total) fail("combined building RGBA budget exceeded");
            auto image=load_sg3_image({verified_archive->second,entry.id.image_index});
            if (image.width!=width || image.height!=height || image_bytes(image)!=bytes)
                fail("House decoded image metadata mismatch");
            entry.image_index=candidate.unique_images.size();ids.push_back(entry.id);
            candidate.unique_images.push_back(std::move(image));total+=bytes;
        }
        candidate.original_house_entries[index]=std::move(entry);
    }
    if (std::find(seen.begin(),seen.end(),false)!=seen.end()) fail("missing House type");
    candidate.original_house_prepared=true;
    profile=std::move(candidate);
}
} // namespace openemperor::assets
