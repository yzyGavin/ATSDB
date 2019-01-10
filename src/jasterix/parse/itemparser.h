#ifndef ITEMPARSING_H
#define ITEMPARSING_H

#include <cstddef>
#include <sstream>

#include "json.hpp"
#include "jasterix_logger.h"

#include <string>
#include <cassert>
#include <exception>
#include <bitset>

namespace jASTERIX
{

class ItemParser
{
public:
    ItemParser (const nlohmann::json& item_definition);
    virtual ~ItemParser() {}

    static ItemParser* createItemParser (const nlohmann::json& item_definition);

    // always return number of parsed bytes
    virtual size_t parseItem (const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                              nlohmann::json& target, nlohmann::json& parent, bool debug) = 0;
    std::string name() const;
    std::string type() const;

protected:
    const nlohmann::json& item_definition_;
    std::string name_;
    std::string type_;
};

bool variableHasValue (const nlohmann::json& data, const std::string& variable_name,
                       const nlohmann::json& variable_value);

inline unsigned char reverseBits(unsigned char b)
{
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;

   return b;
}

inline std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return std::move(elems);
}

}

#endif // ITEMPARSING_H