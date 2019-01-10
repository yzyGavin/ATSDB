#include "itemparser.h"
#include "jasterix_logger.h"
#include "base64.h"

#include "compounditemparser.h"
#include "dynamicbytesitemparser.h"
#include "extendablebitsitemparser.h"
#include "extendableitemparser.h"
#include "fixedbitfielditemparser.h"
#include "fixedbytesitemparser.h"
#include "fixedbitsitemparser.h"
#include "optionalitemparser.h"
#include "repetetiveitemparser.h"
#include "skipbytesitemparser.h"

#include <iostream>
#include <cassert>
#include <string>
#include <exception>
#include <iomanip>

using namespace std;
using namespace nlohmann;

namespace jASTERIX
{

ItemParser::ItemParser (const nlohmann::json& item_definition)
 : item_definition_(item_definition)
{
    if (item_definition.find("name") == item_definition.end())
        throw runtime_error ("item construction without JSON name definition");

    name_ = item_definition.at("name");

    if (item_definition.find("type") == item_definition.end())
        throw runtime_error ("item '"+name_+"' construction without data type definition");

    type_ = item_definition.at("type");
}

ItemParser* ItemParser::createItemParser (const nlohmann::json& item_definition)
{
    if (item_definition.find("name") == item_definition.end())
        throw runtime_error ("item creation without JSON name definition");

    std::string name = item_definition.at("name");

    if (item_definition.find("type") == item_definition.end())
        throw runtime_error ("item '"+name+"' creation without data type definition");

    std::string type = item_definition.at("type");

    if (type == "fixed_bytes")
    {
        return new FixedBytesItemParser(item_definition);
    }
    else if (type == "skip_bytes")
    {
        return new SkipBytesItemParser(item_definition);
    }
    else if (type == "dynamic_bytes")
    {
        return new DynamicBytesItemParser(item_definition);
    }
    else if (type == "compound")
    {
        return new CompoundItemParser(item_definition);
    }
    else if (type == "extendable_bits")
    {
        return new ExtendableBitsItemParser(item_definition);
    }
    else if (type == "extendable")
    {
        return new ExtendableItemParser(item_definition);
    }
    else if (type == "fixed_bitfield")
    {
        return new FixedBitFieldItemParser(item_definition);
    }
    else if (type == "optional_item")
    {
        return new OptionalItemParser(item_definition);
    }
    else if (type == "repetitive")
    {
        return new RepetetiveItemParser(item_definition);
    }
    else
        throw runtime_error ("item creation name '"+name+"' with unknown type '"+type+"'");

}

std::string ItemParser::name() const
{
    return name_;
}

std::string ItemParser::type() const
{
    return type_;
}

//size_t parseFixedBitsItem (const std::string& name, const std::string& type, const nlohmann::json& item_definition,
//                           const char* data, size_t index, size_t size, size_t current_parsed_bytes,
//                           nlohmann::json& target, nlohmann::json& parent, bool debug)
//{
//    if (debug)
//    {
//        assert (type == "fixed_bits");
//        loginf << "parsing fixed bits item '" << name << "'";
//    }

//    if (debug && item_definition.find("start_bit") == item_definition.end())
//        throw runtime_error ("parsing fixed byte bitfield item '"+name+"' without start bit");

//    unsigned int start_bit = item_definition.at("start_bit");

//    if (debug && item_definition.find("bit_length") == item_definition.end())
//        throw runtime_error ("parsing fixed byte bitfield item '"+name+"' without bit length");

//    unsigned int bit_length = item_definition.at("bit_length");
//    unsigned int byte_length = size;

//    size_t tmp_data{0};

//    const char* current_data = &data[index];

//    if (byte_length == 1)
//        tmp_data = *reinterpret_cast<const unsigned char*> (&current_data[0]);
//    else
//    {
//        unsigned char tmp;
//        for (size_t cnt = 0; cnt < byte_length; ++cnt)
//        {
//            tmp = *reinterpret_cast<const unsigned char*> (&current_data[cnt]);

//            if (debug)
//                loginf << "fixed byte bitfield item '"+name+"' cnt " << cnt << " byte "
//                       << std::hex << static_cast<unsigned int> (tmp) << " data " << tmp_data;

//            tmp_data = (tmp_data << 8) + tmp;
//        }
//    }

//    if (debug)
//        loginf << "parsing fixed bits item '" << name << "'"
//               << " byte length " << byte_length
//               << " current data '" << hex << tmp_data << "'"
//               << " with start bit " << start_bit << " length " << bit_length;

//    size_t bitmask {1};
//    bitmask <<= start_bit+bit_length-1;

//    bool bit_set {false};
//    size_t value {0};

//    for (unsigned cnt=0; cnt < bit_length; ++cnt)
//    {
//        value <<= 1;
//        bit_set = tmp_data & bitmask;
//        value |= bit_set;

//        if (debug)
//            loginf << "parsing fixed bits item '" << name << "' with bit " << cnt
//                   << " bitmask " << bitmask << " set " << bit_set << " value " << value;

//        bitmask >>= 1;
//    }

//    if (debug)
//        loginf << "parsing fixed bits item '" << name << "' with start bit " << start_bit
//               << " length " << bit_length << " value " << value;

//    target = value;

//    return 0;
//}

bool variableHasValue (const nlohmann::json& data, const std::string& variable_name,
                       const nlohmann::json& variable_value)
{
    const nlohmann::json* val_ptr = &data;
    std::vector <std::string> sub_keys = split(variable_name, '.');
    for (const std::string& sub_key : sub_keys)
    {
        if (val_ptr->find (sub_key) != val_ptr->end())
        {
            if (sub_key == sub_keys.back()) // last found
            {
                val_ptr = &val_ptr->at(sub_key);
                break;
            }

            if (val_ptr->at(sub_key).is_object()) // not last, step in
                val_ptr = &val_ptr->at(sub_key);
            else // not last key, and not object
            {
                val_ptr = nullptr;
                break;
            }
        }
        else // not found
        {
            val_ptr = nullptr;
            break;
        }
    }

    if (val_ptr == nullptr || *val_ptr == nullptr) // not found
        return false;
    else
        return *val_ptr == variable_value;
}

}