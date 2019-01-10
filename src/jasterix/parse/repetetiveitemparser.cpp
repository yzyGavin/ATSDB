#include "repetetiveitemparser.h"

using namespace std;
using namespace nlohmann;

namespace jASTERIX
{

RepetetiveItemParser::RepetetiveItemParser (const nlohmann::json& item_definition)
 : ItemParser (item_definition)
{
    assert (type_ == "repetitive");

    if (item_definition.find("repetition_item") == item_definition.end())
        throw runtime_error ("repetitive item '"+name_+"' parsing without repetition item specification");

    const json& repetition_item = item_definition.at("repetition_item");

    if (!repetition_item.is_object())
        throw runtime_error ("parsing repetitive item '"+name_+"' repetition item specification is not an object");

    if (repetition_item.at("name") != "rep")
        throw runtime_error ("parsing repetitive item '"+name_+"' repetition item specification has to be named 'rep'");

    repetition_item_.reset(ItemParser::createItemParser(repetition_item));
    assert (repetition_item_);

    if (item_definition.find("items") == item_definition.end())
        throw runtime_error ("parsing repetitive item '"+name_+"' without items");

    const json& items = item_definition.at("items");

    if (!items.is_array())
        throw runtime_error ("parsing repetitive item '"+name_+"' items specification is not an array");

    std::string item_name;
    ItemParser* item {nullptr};

    for (const json& data_item_it : items)
    {
        item_name = data_item_it.at("name");
        item = ItemParser::createItemParser(data_item_it);
        assert (item);
        items_.push_back(std::unique_ptr<ItemParser>{item});
    }
}

size_t RepetetiveItemParser::parseItem (const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                              nlohmann::json& target, nlohmann::json& parent, bool debug)
{
    if (debug)
        loginf << "parsing repetitive item '" << name_ << "' with " << items_.size() << " items";

    size_t parsed_bytes {0};

    if (debug)
        loginf << "parsing repetitive item '"+name_+"' repetition item";

    parsed_bytes = repetition_item_->parseItem(data, index+parsed_bytes, size, parsed_bytes, target, target, debug);

    unsigned int rep = target.at("rep");

    assert (target.find(name_) == target.end());
    json j_data = json::array();

    if (debug)
        loginf << "parsing repetitive item '"+name_+"' items " << rep << " times";

    for (unsigned int cnt=0; cnt < rep; ++cnt)
    {
        for (auto& data_item_it : items_)
        {
            if (debug)
                loginf << "parsing repetitive item '" << name_ << "' data item '" << data_item_it->name() << "' index "
                       << index+parsed_bytes << " cnt " << cnt;

            parsed_bytes += data_item_it->parseItem(data, index+parsed_bytes, size, parsed_bytes,
                                                    j_data[cnt], target, debug);
        }
    }

    target.emplace(name_, std::move(j_data));

    return parsed_bytes;
}

}