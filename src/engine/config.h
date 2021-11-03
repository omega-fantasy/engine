#ifndef CONF_H
#define CONF_H

#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <variant>
#include <cassert>
#include <filesystem>

class ConfigParser {
    public:
    class Node {
        using Type = std::variant<std::string, std::map<std::string, Node>, std::vector<Node>>;
        public:
        auto str() { return std::get<0>(val); }
        auto begin() { return std::get<2>(val).begin(); }
        auto end() { return std::get<2>(val).end(); }
        Node& operator[](const std::string& key) { return std::get<1>(val)[key];}
        operator std::string() const { return std::get<0>(val); }
        operator std::vector<Node>() const { return std::get<2>(val); }

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
                std::get<0>(val) = word;
            }
        }

        Type val;
    };

    void clear() { files.clear(); }
    Node& get(const std::string& file) { return files[file]; }
        
    void add_folder(const std::string& folder) {
        for (auto& fil : std::filesystem::recursive_directory_iterator(folder)) {
            std::string filename = fil.path().filename().string();
            std::string filepath = fil.path().string();
            if (filename.find(".config") != std::string::npos) {
                std::string name = filename.substr(0, filename.find("."));
                std::ifstream f(filepath);
                files.emplace(name, f);
            }
        }
    }
        

    std::map<std::string, Node> files; 
};

#endif
