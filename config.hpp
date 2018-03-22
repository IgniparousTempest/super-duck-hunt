#ifndef DUCKHUNT_SCORES_HPP
#define DUCKHUNT_SCORES_HPP

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

struct Config
{
    int highScore;
    bool useRemakeTextures;

    void load(const std::string &filename) {
        try {
            using boost::property_tree::ptree;
            using boost::property_tree::json_parser::read_json;
            ptree pt;

            read_json(filename, pt);

            highScore = pt.get("highScore", 0);
            useRemakeTextures = pt.get("useRemakeTextures", true);
        }
        catch (const boost::property_tree::json_parser_error& e1) {
            highScore = 0;
            useRemakeTextures = true;
        }
    }

    void save(const std::string &filename) {
        using boost::property_tree::ptree;
        using boost::property_tree::json_parser::write_json;
        ptree pt;

        pt.put("highScore", highScore);
        pt.put("useRemakeTextures", useRemakeTextures);

        // Write the property tree to the XML file.
        write_json(filename, pt);
    }
};

#endif //DUCKHUNT_SCORES_HPP
