// Excluded, explicitly invoked original-map starter/restart acceptance tool.
#include "../tests/OriginalStarterFixture.h"
#include <iostream>

namespace {
namespace q=oe_starter_fixture;
void output_target(const q::fs::path& output,const q::fs::path& data) {
    const auto root=q::fs::canonical(q::fs::path(__FILE__).parent_path().parent_path());
    const auto allowed=root/".local/original-economy/oe30";
    const auto below=[](const auto& child,const auto& parent){auto c=child.begin();
        for(auto p=parent.begin();p!=parent.end();++p,++c)if(c==child.end()||*c!=*p)return false;
        return true;
    };
    q::check(output!=allowed&&below(output,allowed)&&!below(output,data)&&!below(data,output),"Starter output must be a new private OE30 directory disjoint from original data");
    auto part=output.root_path();for(const auto& cell:output.relative_path()) {
        q::check(cell!="."&&cell!="..","Starter output path must be canonical");part/=cell;std::error_code error;
        q::check(!q::fs::is_symlink(q::fs::symlink_status(part,error)),"Starter output traverses a symlink");
    }
    q::check(!q::fs::exists(output),"Starter output already exists");q::save::validate_save_target(output/"fixture.json",data);
}
q::Json checkpoint(const q::Context& context,const q::fs::path& output,const char* label,const q::sim::World& world) {
    const auto save=output/(std::string(label)+".save.json"),report=output/(std::string(label)+".report.json");
    q::save::write_save(save,context.document(world),context.data,context.mask);q::f::write_text(report,q::observe(world).dump(2)+"\n");
    const auto restored=context.restore(q::save::read_save(save));q::check(restored.snapshot()==world.snapshot(),"Process checkpoint changed complete real original World");
    return {{"save",save.filename().string()},{"report",report.filename().string()},{"tick",world.ticks()}};
}
void create(const q::Context& context,const q::fs::path& output) {
    auto world=context.world();const auto receipt=q::sim::place_original_economy_starter(world);q::paid_empty(context,world,receipt);
    q::Json checkpoints=q::Json::array();checkpoints.push_back(checkpoint(context,output,"paid-empty",world));
    q::admission_and_priorities(world,receipt);checkpoints.push_back(checkpoint(context,output,"actual-admission",world));
    q::upgrade(world,receipt);checkpoints.push_back(checkpoint(context,output,"supplied-up",world));
    const auto operations=q::operations(world,receipt,[&](const char* label,const q::sim::World& state){checkpoints.push_back(checkpoint(context,output,label,state));});
    const auto input=output/"restart.save.json";q::save::write_save(input,context.document(world),context.data,context.mask);
    q::fs::create_directories(output/"expected");auto expected=context.restore(q::save::read_save(input));q::continuation(context,output/"expected",expected);
    q::f::write_text(output/"fixture.json",q::Json{{"status","PASS"},{"original_map",true},{"map",context.map_relative.generic_string()},
        {"original_model",true},{"model_modified",false},{"snapshot_injection",false},{"native_actions",false},{"human_acceptance","NOT_RUN"},
        {"difficulty","Normal"},{"origin",{receipt.origin.x,receipt.origin.y}},{"paid_construction",receipt.construction_spent},
        {"commands_executed",receipt.commands_executed},{"checkpoints",checkpoints},{"operations",operations},
        {"restart_input",input.filename().string()},{"restart_begin_tick",world.ticks()},{"continuation_ticks",20000},
        {"model_sha256",context.model->provenance().sha256}}.dump(2)+"\n");
}
}
int main(int argc,char** argv) {
    try {
        q::check(argc>=4,"Usage: original-starter-acceptance --create DATA OUT | --resume DATA OUT SAVE");
        const std::string mode=argv[1];q::check((mode=="--create"&&argc==4)||(mode=="--resume"&&argc==5),"Unknown starter acceptance mode/arity");
        const auto data=q::fs::canonical(argv[2]),output=q::fs::absolute(argv[3]);output_target(output,data);
        const auto context=q::Context::real(data);q::fs::create_directories(output);
        if(mode=="--create")create(context,output);
        else {
            const auto input=q::fs::canonical(argv[4]);const auto bytes=q::f::read_text(input);const auto document=q::save::read_save(input);
            q::check(document.map_relative==context.map_relative,"Starter replay input is not the actual Xia map");auto world=context.restore(document);
            q::check(world.snapshot()==document.world,"Separate process original starter restore changed complete authority");
            q::continuation(context,output,world);q::check(q::f::read_text(input)==bytes,"Starter separate process rewrote supplied save bytes");
        }
        std::cout<<"Original real-map starter process "<<mode<<" PASS; native/human NOT_RUN\n";return 0;
    }catch(const std::exception& error){std::cerr<<"Original starter process FAIL: "<<error.what()<<'\n';return 1;}
}
