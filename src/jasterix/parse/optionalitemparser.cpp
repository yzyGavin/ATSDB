#include "optionalitemparser.h"

using namespace std;
using namespace nlohmann;

namespace jASTERIX
{

OptionalItemParser::OptionalItemParser (const nlohmann::json& item_definition)
 : ItemParser (item_definition)
{
    assert (type_ == "optional_item");

    if (item_definition.find("optional_bitfield_name") == item_definition.end())
        throw runtime_error ("optional item '"+name_+"' parsing without bitfield name");

    bitfield_name_ = item_definition.at("optional_bitfield_name");

    if (item_definition.find("optional_bitfield_index") == item_definition.end())
        throw runtime_error ("optional item '"+name_+"' parsing without bitfield index");

    bitfield_index_ = item_definition.at("optional_bitfield_index");

    if (item_definition.find("data_fields") == item_definition.end())
        throw runtime_error ("parsing optional item '"+name_+"' without sub-items");

    const json& data_fields = item_definition.at("data_fields");

    if (!data_fields.is_array())
        throw runtime_error ("parsing optional item '"+name_+"' data fields container is not an array");

    std::string item_name;
    ItemParser* item {nullptr};

    for (const json& data_item_it : data_fields)
    {
        item_name = data_item_it.at("name");
        item = ItemParser::createItemParser(data_item_it);
        assert (item);
        data_fields_.push_back(std::unique_ptr<ItemParser>{item});
    }
}

size_t OptionalItemParser::parseItem (const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                              nlohmann::json& target, bool debug)
{
    if (debug)
        loginf << "parsing optional item '" << name_ << "'";

    if (debug && target.find(bitfield_name_) == target.end())
        throw runtime_error ("parsing optional item '"+name_+"' without defined bitfield '"+bitfield_name_+"'");

    const json& bitfield = target.at(bitfield_name_);

    if (debug && !bitfield.is_array())
        throw runtime_error ("parsing optional item '"+name_+"' with non-array bitfield '"+bitfield_name_+"'");

    if (bitfield_index_ >= bitfield.size())
    {
        if (debug)
            loginf << "parsing optional item '" << name_ << "' bitfield length " << bitfield.size()
                   << " index " << bitfield_index_ << " out of fspec size";
        return 0;
    }

    if (debug && !bitfield.at(bitfield_index_).is_boolean())
        throw runtime_error ("parsing optional item '"+name_+"' with non-boolean bitfield '"+bitfield_name_+"' value");

    if (debug)
        loginf << "parsing optional item '" << name_ << "' bitfield length " << bitfield.size()
               << " index " << bitfield_index_;

    bool item_exists = bitfield.at(bitfield_index_);

    if (debug)
        loginf << "parsing optional item '" << name_ << "' with " << data_fields_.size() << " data fields, exists "
               << item_exists;

    if (!item_exists)
        return 0;

    size_t parsed_bytes {0};

    if (debug)
        loginf << "parsing optional item '"+name_+"' sub-items";

    for (auto& df_item : data_fields_)
    {
        parsed_bytes += df_item->parseItem(data, index+parsed_bytes, size, current_parsed_bytes, target[name_], debug);
    }

    if (debug)
        loginf << "parsing optional item '"+name_+"' done, " << parsed_bytes << " bytes parsed";

    return parsed_bytes;
}

}
