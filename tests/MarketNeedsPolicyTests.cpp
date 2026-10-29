// Pure authored inputs test selection, not a paid gameplay success.
#include "OriginalProductionFixture.h"
#include "simulation/OriginalMarketPolicy.h"
#include <iostream>
#include <limits>

namespace sim=openemperor::simulation;
namespace orig=openemperor::original;
namespace f=oe_influence_fixture;
int main() {
    try {
        auto text=oe_production_fixture::authored_model_text();
        const auto field=[&](unsigned row,orig::HouseField name,int value){
            oe_production_fixture::set_house_field(text,row,static_cast<unsigned>(name),value);};
        field(6,orig::HouseField::DesirabilityHigh,20);
        field(7,orig::HouseField::Hemp,1);field(7,orig::HouseField::Ceramics,1);
        field(7,orig::HouseField::Tea,8);
        field(7,orig::HouseField::Water,1);field(7,orig::HouseField::Music,1);
        field(8,orig::HouseField::Silk,1);field(13,orig::HouseField::BronzewareOrLacquerware,1);
        f::Root root;f::write_text(root.data/"Model/EmperorBuildingModels.txt",text);
        const auto model=orig::load_model_data(root.data);f::check(bool(model),"Authored model preparation failed");
        const orig::HouseEvolutionRules rules(*model.model,orig::Difficulty::Normal);
        orig::HouseState house;house.type=orig::HouseType::LuxuriousApartment;house.occupants=3;
        house.water=true;house.music=1;
        std::array<int,orig::goods_count> incoming{};
        auto need=sim::original_market_need(rules,house,orig::Goods::Hemp,incoming);
        f::check(need&&need->tier==0&&need->stock==0,"Missing actual current need was not urgent");
        house.tea=5;need=sim::original_market_need(rules,house,orig::Goods::Tea,incoming);
        f::check(need&&need->tier==0,"Physically supported need8 with stock5 was blocked by the half-buffer threshold");house.tea=0;
        incoming[0]=std::numeric_limits<int>::max();bool refused=false;
        try{(void)sim::original_market_need(rules,house,orig::Goods::Hemp,incoming);}catch(const std::invalid_argument&){refused=true;}
        f::check(refused,"Oversized incoming commitment was accepted or overflowed");incoming[0]=0;
        incoming[0]=4;need=sim::original_market_need(rules,house,orig::Goods::Hemp,incoming);
        f::check(need&&need->tier==1&&need->stock==4,"Committed incoming cargo was not counted");
        house.hemp=1;f::check(!sim::original_market_need(rules,house,orig::Goods::Hemp,incoming),"More than half-full committed buffer was refilled");
        f::check(!sim::original_market_need(rules,house,orig::Goods::Silk,incoming),"Terminal Common10 invented an Elite next stage");
        house.occupants=0;f::check(!sim::original_market_need(rules,house,orig::Goods::Hemp,incoming),"Empty House gained distribution demand");
        house.occupants=3;house.type=orig::HouseType::OrnateApartment;house.desirability=19;incoming={};
        f::check(!sim::original_market_need(rules,house,orig::Goods::Ceramics,incoming),"DES-blocked next row was prefetched");
        house.desirability=20;house.music=0;
        f::check(!sim::original_market_need(rules,house,orig::Goods::Ceramics,incoming),"Missing real service was treated as usable next row");
        house.music=1;need=sim::original_market_need(rules,house,orig::Goods::Ceramics,incoming);
        f::check(need&&need->tier==2,"Actual usable next row was starved of its goods");
        house.type=orig::HouseType::ImpressiveCompound;house.lacquerware=1;
        f::check(!sim::original_market_need(rules,house,orig::Goods::Bronzeware,incoming),"Satisfied luxury OR requested a redundant alternative");
        house.lacquerware=0;incoming[5]=1;
        f::check(!sim::original_market_need(rules,house,orig::Goods::Bronzeware,incoming),"Incoming luxury OR commitment was ignored");
        incoming[5]=0;need=sim::original_market_need(rules,house,orig::Goods::Bronzeware,incoming);
        f::check(need&&need->tier==0,"Missing luxury OR was not supplied");
        const std::array choices{sim::MarketChoice{1,{0,0,1}},sim::MarketChoice{7,{0,0,1}},sim::MarketChoice{11,{2,0,1}}};
        f::check(sim::choose_original_market_pair(choices,0,12)==1&&sim::choose_original_market_pair(choices,2,12)==7&&
                 sim::choose_original_market_pair(choices,8,12)==1,"Saved pair cursor did not rotate equal current needs before next needs");
        const std::array normalized{sim::MarketChoice{1,{1,3,1}},sim::MarketChoice{2,{1,4,2}}};
        f::check(sim::choose_original_market_pair(normalized,0,12)==2,"Selection ignored normalized actual buffer coverage");
        std::cout<<"PASS: authored needs policy current/usable-next, real commitments, terminal family, OR and saved-cursor fairness\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
