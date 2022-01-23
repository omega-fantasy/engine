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

        Node() { }
        Node(std::vector<std::string>& stream) {
            std::string word = stream.back();
            stream.pop_back();
            if (word == "{") {
                //print("Parsing map:");
                val = std::map<std::string, Node>();
                while (1) {
                    std::string key = stream.back();
                    stream.pop_back();
                    if (key == " " || key.empty()) continue;
                    if (key == "}") break;
                    word = stream.back(); // seperator
                    stream.pop_back();
                    //print("Parsing key: " + key);
                    std::get<1>(val).emplace(key, stream);
                }
                //print("End map");
            } else if (word == "[") {
                //print("Parsing list: ");
                val = std::vector<Node>();
                auto& vec = std::get<2>(val);
                while (1) {
                    if (stream.back() == " " || stream.back().empty()) {
                        stream.pop_back();
                        continue;
                    }
                    vec.emplace_back(stream);
                    if (std::get_if<0>(&vec.back().val) && vec.back().str() == "]") {
                        vec.pop_back();
                        break;
                    }
                }
                //print("End list");
            } else {
                //print("Parsing element: " + word );
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
            std::vector<std::string> tokens;
            FileHandle file = file_open(filepath);
            while (!file_isend(file)) {
                std::vector<std::string> line = split(file_readline(file), ' ');
                tokens.insert(tokens.end(), line.begin(), line.end());
            }
            file_close(file);
            std::reverse(tokens.begin(), tokens.end());
            files.emplace(filename(filepath), tokens);
        }
    }

    std::map<std::string, Node> files; 
};

#endif
