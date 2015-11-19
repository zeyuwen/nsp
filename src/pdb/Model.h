#ifndef Model_H
#define Model_H

#include "Chain.h"

namespace jian {

class Model
{
public:
    Model();
    Model(MolFile &mol_file);
//    Model(PdbFile &pdb_file);
//    Model(Cif &cif);
    Model(Model *model) : name(model->name), chains(model->chains) {}
    Model(const Model &model) : name(model.name), chains(model.chains) {}
    Model &operator =(const Model &model);
    Model(vector<string>);
    Model(std::string pdbfile);
    void read(std::string);
    virtual void read_pdb(std::string);
    virtual void read_cif(std::string);

    vector<Chain>::iterator begin();
    vector<Chain>::iterator end();

    int res_nums() const {
        int res_num = 0;
        for (auto &chain: chains) {
            for (auto &residue: chain.residues) {
                res_num++;
            }
        }
        return res_num;
    }

    int atom_nums() const {
        int atom_num = 0;
        for (auto &chain: chains) {
            for (auto &residue: chain.residues) {
                for (auto &atom: residue.atoms) {
                    atom_num++;
                }
            }
        }
        return atom_num;
    }


    virtual std::string seq(std::string delimiter = "");
    template<class T>
    Model sub(const T &t) {
        Model model;
        int res_num = 0;
        for (auto &&chain: chains) {
            Chain temp_chain;
            temp_chain.name = chain.name;
            for (auto &&res: chain.residues) {
                if (std::count(std::begin(t), std::end(t), res_num)) {
                    temp_chain.residues.push_back(res);
                }
                res_num++;
            }
            if (!temp_chain.residues.empty())
                model.chains.push_back(temp_chain);
        }
        return model;
    }
    int empty();
    void push(const Chain &);
    Chain &operator [](int);
    const Chain &operator [](int) const;

    /* IO function */
    void print();
    void write(string);
    friend ostream &operator <<(ostream &, const Model &);
    
    /* attributes */
    string name = "none";
    vector<Chain> chains;

};

} /// namespace jian

#endif // Model_H
