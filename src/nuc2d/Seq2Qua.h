#ifndef JIAN_NUC2D_SEQ2QUA_H
#define JIAN_NUC2D_SEQ2QUA_H

#include <util/util.h>

namespace jian {
namespace nuc2d {

namespace seq2qua {
struct MyHash {
    std::size_t operator ()(const std::vector<int> &s) const {
        long hash = 5381;  
        for(int i = 0; i < s.size(); i++)  {  
            hash = ((hash << 5) + hash) + s[i];  
        }  
        return hash;
    }
};
}

class Seq2Qua {
public:
    typedef std::tuple<int, int, int, int> Pair;
    typedef std::vector<Pair> Pairs;
    typedef std::vector<std::pair<Pairs, double>> PairLists;

    void operator ()(std::string seq);
    double get(const std::vector<int> &sequence);
    double score(int, int, int, int);
    PairLists backtrack(const std::vector<int> &sequence, double cutoff);

    std::unordered_map<std::vector<int>, double, seq2qua::MyHash> _scores;

    std::string _seq;
    std::map<char, int> _convert{{'A', 0}, {'U', 1}, {'G', 2}, {'C', 3}};
    int _min_hairpin_size = 4;
    double _cutoff = 1;
};

} /// namespace nuc2d
} /// namespace jian

#endif