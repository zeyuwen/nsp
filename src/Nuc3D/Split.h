#ifndef JIAN_NUC3D_SPLIT_H
#define JIAN_NUC3D_SPLIT_H

#include "../Pdb.h"
#include "../Nuc2D.h"

namespace jian {

namespace nuc3d {

class Split {
public:
    Split(Par par);

    void operator ()();
    void splitMol(nuc2d::loop *);

    std::string lib;
    std::string ss;
    std::string family = "other";
    std::string name;
    std::string type = "RNA";
    std::string seq;
    Model mol;
    int _helix_num = 0;
    int _loop_num = 0;
};

} /// namespace nuc3d

} /// namespace jian


#endif




