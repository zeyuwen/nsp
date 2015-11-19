#ifndef RNA_H
#define RNA_H

#include "Model.h"

namespace jian {

class RNA :public Model {
public:
    RNA();
    RNA(const Model &model);
    RNA(RNA *rna) {
        len = rna->len;
        chains = rna->chains;
    }
    RNA(const RNA &rna) {
        len = rna.len;
        chains = rna.chains;
    }
    RNA &operator =(const Model &model);
    RNA &operator =(const RNA &rna);
    RNA(char *);
    RNA(std::string);
    RNA *copy();
    void read_pdb(std::string);
    void read_cif(std::string);
    void push(Chain *);
    void push(Chain &);
    Chain &operator [](int);
    void updateChains(string);

    void setLen();
    void setResNum();

    int getLen();
    int totalAtoms();
    double getDist(int, int);
    string getSeq();
    string getChain();
    
    /* IO function */
    void print();
    void printAsDNA();
    void write(string);
    
    /* assemble function */
    void move(double, double, double);
    void rotate(Matr_ *m);
    void format();
    void addP();
    void mutate(string);
    void rotateByX(double);
    void rotateByZ(double);

    /* attributes */
//    string name;
    int len;
};

class RNAs {
public:
    RNAs();
    ~RNAs();

    int getLen();
    void resize(int);
    RNA *at(int);
    RNA *operator [](int);
    void push(RNA *);
    
    vector<RNA *> RNAList;
};

} /// namespace jian

#endif // RNA_H