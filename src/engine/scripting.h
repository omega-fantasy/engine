#ifndef SCRIPTING_H
#define SCRIPTING_H

#include "util.h"
#include <setjmp.h>

class Script {
    public:
    enum Type { Number, List, String, Any };
    using Value = std::variant<double, std::vector<double>*, std::string>;
    using ParamList = std::vector<Value>;
    using Function = std::function<Value(ParamList&)>;
    static double d(const Value& v) { return std::get<0>(v); }
    static std::string s(const Value& v) { return std::get<2>(v); }
    static std::vector<double>* dl(Value& v) { return std::get<1>(v); }

    static std::string type_to_str(Type t) {
        switch (t) {
            case Type::Number: return "number";
            case Type::List:   return "list";
            case Type::String:  return "string";
            default: return "any";
        }
    }

    static std::string valuetype_to_str(const Value& v) {
        if (std::holds_alternative<double>(v)) { return "number"; } 
        else if (std::holds_alternative<std::string>(v)) { return "string"; }
        else if (std::holds_alternative<std::vector<double>*>(v)) { return "list"; }
        return "?";
    }

    class Context {
        public:
        Context(std::map<std::string, std::pair<Function, std::vector<Type>>>& f): functions(f) {}
        ~Context() {
            // TODO: clean up heap variables
        }

        void panic(const std::string& msg) {
            print("ERROR while parsing script in line " + std::to_string(current_line) + "!");
            print("Description: " + msg);
            longjmp(env, 1);
        }

        void check_parameters(std::string& fname, ParamList& params) {
            auto& expected_params = functions[fname].second;
            if (params.size() != expected_params.size()) {
                panic("function " + fname + " expected " + std::to_string(expected_params.size()) + " parameters but recevied " + std::to_string(params.size()));
            }
            for (int i = 0; i < (int)params.size(); i++) {
                if (expected_params[i] == Type::Any) {
                    continue;
                }
                std::string expected = type_to_str(expected_params[i]);
                std::string actual = valuetype_to_str(params[i]);
                if (expected != actual) {
                    panic("while evaluating parameter " + std::to_string(i+1) + " of function " + fname + ": expected type " + expected + " but received type " + actual);
                }
            }
        }

        void execute(const std::string& filepath) {
            std::vector<std::pair<std::string, int>> stack;
            std::ifstream stream = std::ifstream(filepath);
            std::vector<std::vector<std::string>> lines = {{"script start"}};
            int l = 1;
            for (std::string line; std::getline(stream, line);) {
                replace(line, ",", " ");
                replace(line, "(", "( ");
                replace(line, ")", " )");
                lines.push_back(split(line, ' '));
                const std::vector<std::string>& words = lines.back();
                if (words.size() > 0) {
                    if (words[0] == "for" || words[0] == "else" || words[0] == "if") {
                        stack.push_back({words[0], l});
                    } else if (words[0] == "end") {
                        auto token = stack.back();
                        stack.pop_back();
                        if (token.first == "for") {
                            loops[token.second] = l;
                        } else if (token.first == "if") {
                            branches[token.second] = {-1, l};
                        } else if (token.first == "else") {
                            auto if_token = stack.back(); // TODO: assert that if/else matches
                            stack.pop_back();
                            branches[if_token.second] = {token.second, l};
                        }
                    }
                }
                l++;
            }
            // TODO: assert that stack is empty

            for (current_line = 1; current_line < (int)lines.size(); current_line++) {
                d(evaluate(lines[current_line], 0));
            }
        }

        Value evaluate(const std::vector<std::string>& words, int idx, int* nested_idx = 0) {
            if (words.empty() || words[0] == "" || words[0] == "#") { // comment
                return 0.0;
            }
            if (idx == 0) {
                if (words[0] == "let" && words.size() >= 3) { // assignment
                    variables[words[1]] = evaluate(words, 2);
                    return 0.0;
                } else if (words[0] == "for") { // loop
                    if (variables.find(words[1]) == variables.end()) {
                        variables[words[1]] = d(evaluate(words, 2));
                    } else {
                        variables[words[1]] = d(variables[words[1]]) + 1;
                    }
                    if (d(variables[words[1]]) > d(evaluate(words, 3))) {
                        current_line = loops[current_line];
                        variables.erase(variables.find(words[1]));
                    }
                    return 0.0;
                } else if (words[0] == "if" && words.size() >= 3) {
                    if (d(evaluate(words, 1)) == 0) {
                        if (branches[current_line].first > 0) {
                            current_line = branches[current_line].first;
                        } else {
                            current_line = branches[current_line].second;
                        }
                    }
                    return 0.0;
                } else if (words[0] == "else") {
                    for (auto& pair : branches) {
                        if (pair.second.first == current_line) {
                            current_line = pair.second.second;
                            return 0.0;
                        }
                    }
                    panic("could not match else in line " + std::to_string(current_line));
                } else if (words[0] == "end") {
                    for (auto& pair : loops) {
                        if (pair.second == current_line) {
                            current_line = pair.first - 1;
                            return 0.0;
                        }
                    }
                    return 0.0;
                }
            }

            const std::string& current_word = words[idx];
            if (current_word == ")") {
                return ")";
            } else if (current_word.back() == '(') { // function call
                std::string fname = current_word.substr(0, current_word.size()-1);
                auto find = functions.find(fname);
                if (find != functions.end()) {
                    ParamList params;
                    while (++idx < (int)words.size()) {
                        Value v = evaluate(words, idx, &idx);
                        if (std::holds_alternative<std::string>(v) && s(v) == ")") {
                            if (nested_idx) {
                                *nested_idx = idx;
                            }
                            break;
                        }
                        params.push_back(v);
                    }
                    check_parameters(fname, params);
                    return find->second.first(params);
                } else {
                    panic("unknown function " + fname);
                }
            }
            if (current_word.size() > 2 && current_word[0] == '"' &&  current_word.back() == '"') { // string literal
                return current_word.substr(1, current_word.size()-2);
            }
            double d = to_double(current_word);
            if (!std::isnan(d)) { // number literal
                return d;
            }
            if (variables.find(current_word) != variables.end()) { // variable
                return variables[current_word];
            }
    
            panic("undeclared identifier " + current_word);
            return -1.0;
        }
        
        int current_line = 1;
        std::map<std::string, Value> variables;
        std::map<std::string, std::pair<Function, std::vector<Type>>>& functions;
        std::map<int, int> loops;
        std::map<int, std::pair<int, int>> branches;
    };

    void add_function(const std::string& name, const std::vector<Type>& types, const Function& f) {
        functions[name] = {f, types};
    }

    Script() {
        add_function("add", {Type::Number, Type::Number}, [](ParamList& params){return d(params[0]) + d(params[1]);});
        add_function("sub", {Type::Number, Type::Number}, [](ParamList& params){return d(params[0]) - d(params[1]);});
        add_function("mul", {Type::Number, Type::Number}, [](ParamList& params){return d(params[0]) * d(params[1]);});
        add_function("div", {Type::Number, Type::Number}, [](ParamList& params){return d(params[0]) / d(params[1]);});
        add_function("sqrt", {Type::Number}, [](ParamList& params){return std::sqrt(d(params[0]));});
        add_function("exp", {Type::Number, Type::Number}, [](ParamList& params){return std::pow(d(params[0]), d(params[1]));});
        add_function("log", {Type::Number}, [](ParamList& params){return std::log10(d(params[0]));});
        add_function("floor", {Type::Number}, [](ParamList& params){return std::floor(d(params[0]));});
        add_function("ceil", {Type::Number}, [](ParamList& params){return std::ceil(d(params[0]));});
        add_function("and", {Type::Number, Type::Number}, [](ParamList& params){return d(params[0]) && d(params[1]) ? 1.0 : 0.0;});
        add_function("or", {Type::Number, Type::Number}, [](ParamList& params){return d(params[0]) || d(params[1]) ? 1.0 : 0.0;});
        add_function("not", {Type::Number}, [](ParamList& params){return !d(params[0]) ? 1.0 : 0.0;});
        add_function("greater", {Type::Number, Type::Number}, [](ParamList& params){ return d(params[0]) > d(params[1]) ? 1.0 : 0.0; });
        add_function("less", {Type::Number, Type::Number}, [](ParamList& params){ return d(params[0]) < d(params[1]) ? 1.0 : 0.0; });
        add_function("equal", {Type::Number, Type::Number}, [](ParamList& params){ return d(params[0]) == d(params[1]) ? 1.0 : 0.0; });
        add_function("list_new", {}, [](ParamList&){ return new std::vector<double>(); });
        add_function("list_get", {Type::List, Type::Number}, [](ParamList& params){ return (*dl(params[0]))[d(params[1])]; });
        add_function("list_add", {Type::List, Type::Number}, [](ParamList& params){ dl(params[0])->push_back(d(params[1])); return 0.0; });
        add_function("list_max", {Type::List}, [](ParamList& params){ return (double)dl(params[0])->size() - 1; });
        add_function("print", {Type::Any}, [](ParamList& params){
            if (std::holds_alternative<double>(params[0])) { print(std::to_string(d(params[0]))); }
            else if (std::holds_alternative<std::string>(params[0])) { print(s(params[0])); }
            else if (std::holds_alternative<std::vector<double>*>(params[0])) {
                std::string list = "[";
                for (auto& d : *dl(params[0])) { list += " " + std::to_string(d); }
                print(list + " ]");
            }
            return 0.0;
        });
        add_function("print_functions", {}, [&](ParamList&){
            print("All defined functions and their parameters:");
            for (auto& pair : current_context->functions) { 
                std::string s = pair.first + "(";
                for (int i = 0; i < (int)pair.second.second.size(); i++) {
                    if (i > 0) s += " ";
                    s += type_to_str(pair.second.second[i]);
                }
                print(s + ")");
            }
            return 0.0;
        });
        add_function("print_variables", {}, [&](ParamList&){
            print("All defined variables and their current value:");
            for (auto& pair : current_context->variables) {
                if (std::holds_alternative<double>(pair.second)) {
                    print(pair.first + ": " + std::to_string(d(pair.second))); 
                } else if (std::holds_alternative<std::string>(pair.second)) {
                    print(pair.first + ": " + s(pair.second));
                } else if (std::holds_alternative<std::vector<double>*>(pair.second)) {
                    std::string list = "[";
                    for (auto& d : *dl(pair.second)) { list += " " + std::to_string(d); }
                    print(pair.first + ": " + list + " ]");
                }
            }
            return 0.0;
        });
    }

    void execute(const std::string& filepath) {
        int val = setjmp(env);
        if (!val) {
            current_context = new Context(functions);
            current_context->execute(filepath);
        }
        delete current_context;
    }

    std::map<std::string, std::pair<Function, std::vector<Type>>> functions;
    Context* current_context = nullptr;
    inline static jmp_buf env;
};


#endif