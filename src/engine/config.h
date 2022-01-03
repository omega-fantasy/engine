#ifndef CONF_H
#define CONF_H

#include "util.h"

class ConfigParser {
    public:
    class Node {
        using Type = std::variant<std::string, std::map<std::string, Node>, std::vector<Node>>;
        public:
        auto str() { return std::get<0>(val); }
        auto begin() { return std::get<2>(val).begin(); }
        auto end() { return std::get<2>(val).end(); }
        Node& operator[](const std::string& key) { return std::get<1>(val)[key];}
        double d() { return std::stod(std::get<0>(val)); }
        int i() { return std::stoi(std::get<0>(val)); }
        Color c() { return Color((unsigned)std::stoul(std::get<0>(val), nullptr, 0)); }
        std::map<std::string, Node> map() { return std::get<1>(val); }
        operator std::string() const { return std::get<0>(val); }
        operator std::vector<Node>() const { return std::get<2>(val); }
        bool contains(const std::string& s) { return std::get<1>(val).find(s) != std::get<1>(val).end(); }

        Node() { assert(false); }
        Node(std::ifstream& stream) {
            std::string word;
            stream >> word;
            if (word == "{") {
                //std::cout << "Parsing map: " << std::endl;
                val = std::map<std::string, Node>();
                while (1) {
                    std::string key;
                    stream >> key;
                    if (key == "}") break;
                    stream >> word; // seperator
                    //std::cout << "Parsing key: " << key << std::endl;
                    std::get<1>(val).emplace(key, stream);
                }
                //std::cout << "End map" << std::endl;
            } else if (word == "[") {
                //std::cout << "Parsing list: " << std::endl;
                val = std::vector<Node>();
                auto& vec = std::get<2>(val);
                while (1) {
                    vec.emplace_back(stream);
                    if (std::get_if<0>(&vec.back().val) && vec.back().str() == "]") {
                        vec.pop_back();
                        break;
                    }
                }
                //std::cout << "End list" << std::endl;
            } else {
                //std::cout << "Parsing element: " << word << std::endl;
                val = std::string();
                std::replace(word.begin(), word.end(), '$', ' ');
                std::get<0>(val) = word;
            }
        }

        Type val;
    };

    void clear() { files.clear(); }
    Node& get(const std::string& file) { return files[file]; }
        
    void add_folder(const std::string& folder) {
        for (auto& filepath : filelist(folder, ".config")) {
            std::ifstream f(filepath);
            files.emplace(filename(filepath), f);
        }
    }

    std::map<std::string, Node> files; 
};

#endif
