#ifndef DB_H
#define DB_H

#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

/*
struct StringBase{};
template<int N> struct String : StringBase {
    String() {}
    String(const char* c) { strcpy(mem, c); }
    String(const std::string& c) { strcpy(mem, c.c_str()); }
    String(const String<N>& c) { memcpy(mem, c.mem, N); }
    String(const String<N>&&) = delete;
        
    String<N>& operator=(const String<N>&&) = delete;
    String<N>& operator=(const String<N>& c) { memcpy(mem, c.mem, N); return *this;}
    
    String<N>& operator=(const std::string& c) { strcpy(mem, c.c_str()); return *this;}
    String<N>& operator=(const char* c) { strcpy(mem, c); return *this;}
    std::string toStdString() { return std::string(mem); }
    char mem[N] = {0};
};
using String8 = String<8>;
using String16 = String<16>;
using String32 = String<32>;
*/

class TableBase {
    public:
        void write(std::ofstream& file) {
            int namesize = name.size();
            file.write((char*)(&namesize), sizeof(namesize)); 
            file.write(name.c_str(), name.size());
            int nRows = keyToIndex.size();
            file.write((char*)(&nRows), sizeof(nRows)); 
            for (auto& k : keyToIndex) {
                file.write((char*)(&k.first), sizeof(k.first)); 
                file.write((char*)(&k.second), sizeof(k.second)); 
            }
            int nDeleted = deletedIndices.size();
            file.write((char*)(&nDeleted), sizeof(nDeleted)); 
            for (auto& k : deletedIndices) {
                file.write((char*)(&k), sizeof(k)); 
            }
            file.write((char*)(&elem_size), sizeof(elem_size)); 
            file.write((char*)(mem.data()), nRows * elem_size);
        }
        
        void read(std::ifstream& file) {
            int nRows = -1;
            file.read((char*)(&nRows), sizeof(nRows));
            for (int i = 0; i < nRows; i++) {
                int key = 0;
                int value = 0;
                file.read((char*)(&key), sizeof(key)); 
                file.read((char*)(&value), sizeof(value)); 
                keyToIndex[key] = value;
            }
            int nDeleted = 0;
            file.read((char*)(&nDeleted), sizeof(nDeleted)); 
            for (int i = 0; i < nDeleted; i++) {
                int key = 0;
                file.read((char*)(&key), sizeof(key)); 
                deletedIndices.push_back(key);
            }
            file.read((char*)(&elem_size), sizeof(elem_size)); 
            mem.resize(elem_size * nRows);
            file.read((char*)(mem.data()), nRows * elem_size);
        }
    
    protected:
        std::vector<char> mem;
        std::map<int, int> keyToIndex;
        std::vector<int> deletedIndices;
        std::string name;
        int elem_size;
};

template <typename T>
class Table : public TableBase {
    public:
        class Iterator {
            public:
                Iterator(std::map<int, int>::iterator i, Table<T>& t): it(i), table(t) {}
                Iterator& operator++() { ++it; return *this; }
                bool operator!=(const Iterator & other) const { return it != other.it; }
                T& operator*() { return table.get(it->first); }
                int key() { return it->first; }
            private:
                std::map<int, int>::iterator it;
                Table<T>& table;
        };

        Iterator begin() { return Iterator(keyToIndex.begin(), *this); } 
        Iterator end() { return Iterator(keyToIndex.end(), *this); }
        
        Table(const std::string& table_name) {
            name = table_name;
            elem_size = sizeof(T);
        }

        template <typename ...Ts>
        T& add(int key, Ts const&... values) {
            int idx = -1;
            if (deletedIndices.empty()) {
                idx = keyToIndex.size() * elem_size;
                mem.resize(idx + elem_size);
            } else {
                idx = *(deletedIndices.end()-1);
                deletedIndices.pop_back();
            }    
            keyToIndex[key] = idx;
            return *(new ((char*)mem.data() + idx) T(values...));
        }

        T& get(int key) {
            return *(T*)((char*)mem.data() + keyToIndex[key]);
        }

        void erase(int key) {
            deletedIndices.push_back(keyToIndex[key]);
            keyToIndex.erase(keyToIndex.find(key));
        }

        bool exists(int key) { return keyToIndex.find(key) != keyToIndex.end(); }
};

class MatrixBase {
    public:
        void write(std::ofstream& file) {
            int namesize = name.size();
            file.write((char*)(&namesize), sizeof(namesize)); 
            file.write(name.c_str(), name.size());
            file.write((char*)(&w), sizeof(w)); 
            file.write((char*)(&h), sizeof(h)); 
            file.write((char*)(&elem_size), sizeof(elem_size)); 
            file.write(mem, w * h * elem_size);
        }
        
        void read(std::ifstream& file) {
            file.read((char*)(&w), sizeof(w)); 
            file.read((char*)(&h), sizeof(h)); 
            file.read((char*)(&elem_size), sizeof(elem_size));
            mem = new char[w * h * elem_size]; 
            file.read(mem, w * h * elem_size);
        }
    
    protected:
        std::string name;
        int w;
        int h;
        int elem_size;
        char* mem;
};

template <typename T>
class Matrix : public MatrixBase {
    public:
        Matrix(const std::string& matrix_name, int width, int height) {
            name = matrix_name;
            if (width && height) {
                w = width;
                h = height;
                elem_size = sizeof(T); 
                mem = new char[w * h * elem_size];
                std::memset(mem, 0, w * h * elem_size);
            }
        }
        ~Matrix() { delete[] mem; }
        T* begin() const { return (T*)mem; }
        T* end() const { return (T*)mem + w * h; }
        T& get(short x, short y) { return *((T*)mem + y * w + x); }
        int width() { return w; }
        int height() { return h; }
};

class Database {
    public:
        Database(const std::string& db_name): name(db_name) {}

        template <typename T>
        Table<T>* create_table(const std::string& table_name) {
           tables.insert(std::make_pair(table_name, new Table<T>(table_name)));
           return get_table<T>(table_name);
        }

        template <typename T>
        Table<T>* get_table(const std::string& table_name) {
           if (tables.find(table_name) != tables.end()) {
               return static_cast<Table<T>*>(tables.at(table_name)); 
           }
           return create_table<T>(table_name);
        }
        
        template <typename T>
        Matrix<T>* create_matrix(const std::string& matrix_name, int width, int height) {
           matrices.insert(std::make_pair(matrix_name, new Matrix<T>(matrix_name, width, height)));
           return get_matrix<T>(matrix_name, width, height);
        }

        template <typename T>
        Matrix<T>* get_matrix(const std::string& matrix_name, int width, int height) {
           if (matrices.find(matrix_name) != matrices.end()) {
               return static_cast<Matrix<T>*>(matrices.at(matrix_name)); 
           }
           return create_matrix<T>(matrix_name, width, height);
        }

        void write(const std::string& filename) {
            std::ofstream file(filename, std::ios::out | std::ios::binary);
            int namesize = name.size();
            file.write((char*)(&namesize), sizeof(namesize)); 
            file.write(name.c_str(), name.size());
            int numTables = tables.size();
            file.write((char*)(&numTables), sizeof(numTables)); 
            for (auto& t : tables) {
                t.second->write(file);
            } 
            int numMatrices = matrices.size();
            file.write((char*)(&numMatrices), sizeof(numMatrices)); 
            for (auto& t : matrices) {
                t.second->write(file);
            } 
        }
        
        void read(const std::string& filename) {
            for (auto item : tables) delete item.second;
            for (auto item : matrices) delete item.second;
            tables.clear();
            matrices.clear();

            std::ifstream file(filename, std::ios::out | std::ios::binary);
            int namesize = -1;
            file.read((char*)(&namesize), sizeof(namesize));
            name.resize(namesize); 
            file.read(&name[0], name.size());
            int numTables = -1;
            file.read((char*)(&numTables), sizeof(numTables)); 
            for (int i = 0; i < numTables; i++) {
                std::string table_name;
                file.read((char*)(&namesize), sizeof(namesize)); 
                table_name.resize(namesize);
                file.read(&table_name[0], table_name.size());
                create_table<char>(table_name)->read(file);
            }
            int numMatrices = -1;
            file.read((char*)(&numMatrices), sizeof(numMatrices)); 
            for (int i = 0; i < numMatrices; i++) {
                std::string matrix_name;
                file.read((char*)(&namesize), sizeof(namesize)); 
                matrix_name.resize(namesize);
                file.read(&matrix_name[0], matrix_name.size());
                create_matrix<char>(matrix_name, 0, 0)->read(file);
            }
        }

    private:
        std::map<std::string, TableBase*> tables;
        std::map<std::string, MatrixBase*> matrices;
        std::string name;
};

#endif

/*
struct MyClass {
    MyClass(int a, int b): x(a), y(b) {}
    int x = 5;
    int y = 3;
    char c[32] = "xyz";
};

int main () {
    auto db = Database("foobar");
    auto t = db.create_table<MyClass>("default");
    int num = 1000000;
    for (int i = 0; i < num; i++)    
        t->create(i, 1, 2);
    for (auto& val : *t) {
        val.x = 5;
    }
    auto m = db.create_matrix<MyClass>("default2", 100, 100);
    for (auto& val : *m) {
        val.x = 13;
    }
    db.write("foobar");
    
    auto db2 = Database("foobar2");
    db2.read("foobar2");
    return 0;
}
*/
