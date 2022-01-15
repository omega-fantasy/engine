#ifndef DB_H
#define DB_H

#include "util.h"

class CompressedFile {
    public:
    CompressedFile(const std::string filename, bool write): write_mode(write) {
        std::ios_base::openmode mode = write ? std::ios::out : std::ios::in;
        file = std::fstream(filename, mode | std::ios::binary);
        if (!write) {
            read_block();
        }
    }

    ~CompressedFile() {
        if (write_mode && buffer_pos > 0) {
            write_block();
        }
    }

    void write(const char* s, int n) {
        for (int i = 0; i < n; i++) {
            buffer[buffer_pos++] = s[i];
            if (buffer_pos >= BLOCK_SIZE) {
                write_block();
            }
        }
    }

    void read(char* s, int n) {
        for (int i = 0; i < n; i++) {
            s[i] = buffer[buffer_pos++];
            if (buffer_pos >= BLOCK_SIZE) {
                read_block();
            }
        }
    }

    void write_block() {
        compress(buffer, BLOCK_SIZE, comp_buffer, comp_block_size);
        file.write((char*)(&comp_block_size), sizeof(comp_block_size));
        file.write(comp_buffer, comp_block_size);
        buffer_pos = 0;
    }

    void read_block() {
        file.read((char*)(&comp_block_size), sizeof(comp_block_size));
        file.read(comp_buffer, comp_block_size);
        decompress(comp_buffer, comp_block_size, buffer, 2 * BLOCK_SIZE);
        buffer_pos = 0;
    }

    int comp_block_size = 0;
    bool write_mode;
    static constexpr int BLOCK_SIZE = 32000;
    std::fstream file;
    int buffer_pos = 0;
    char buffer[2 * BLOCK_SIZE];
    char comp_buffer[2 * BLOCK_SIZE];
};

class TableBase {
    public:
        void write(CompressedFile& file) {
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
        
        void read(CompressedFile& file) {
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
        void write(CompressedFile& file) {
            int namesize = name.size();
            file.write((char*)(&namesize), sizeof(namesize)); 
            file.write(name.c_str(), name.size());
            file.write((char*)(&w), sizeof(w)); 
            file.write((char*)(&h), sizeof(h)); 
            file.write((char*)(&elem_size), sizeof(elem_size)); 
            file.write(mem, w * h * elem_size);
        }
        
        void read(CompressedFile& file) {
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
            CompressedFile file(filename, true);
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
            CompressedFile file(filename, false);
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
