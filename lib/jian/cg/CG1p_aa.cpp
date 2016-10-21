#include <memory>
#include "CG1p.hpp"
#include "../utils/Debug.hpp"
#include "../pp.hpp"
#include "../utils/file.hpp"
#include "../pdb.hpp"
#include "../utils/Env.hpp"
#include "../geom.hpp"

namespace jian {

class C2A {
public:
    std::deque<std::shared_ptr<Eigen::MatrixXd> > _frags;
    std::deque<std::string> _names;
    std::string _path;
    int _len_frag = 4;

    C2A() {
        _path = Env::lib() + "/RNA/pars/nuc3d/C2A/";
        EACH_SPLIT_LINE((_path + "inf.txt").c_str(), " ",
            auto m = std::make_shared<Eigen::MatrixXd>(_len_frag, 3);
            _names.push_back(F[0]);
			for (int i = 0; i < _len_frag; i++) for (int j = 0; j < 3; j++) (*m)(i, j) = JN_DBL(F[i * 3 + j + 1]);
            //FOR((i, _len_frag), FOR((j, 3), (*m)(i, j) = JN_DBL(F[i*3+j+1])));
            _frags.push_back(m);
        );
    }

    template<typename T>
    auto get_residues(int i, T &&c) {
        Debug::print(_path + _names[i] + ".pdb\n");
        Chain residues;
        chain_read_model(residues, _path + _names[i] + ".pdb");
        auto s = geom::suppos(*(_frags[i]), c);
		for (auto && res : residues) {
			for (auto && atom : res) {
				geom::apply_suppos(atom, s);
			}
		}
        //EACH(res, residues, EACH(atom, res, geom::apply_suppos(atom, s)));
        return residues;
    }

    template<typename T, typename U>
    auto run(T &&coord, U &&frag) {
        Debug::print("## C2A\n");
        int len = frag[1] - frag[0] + 1; 
        Chain chain;
        for (int i = 0; i < len - _len_frag + 1; i++) {
            Eigen::MatrixXd c(_len_frag, 3); 
			for (int j = 0; j < _len_frag; j++) for (int k = 0; k < 3; k++) c(j, k) = coord(frag[0] + i + j, k);
            //FOR((j, _len_frag), FOR((k, 3), c(j, k) = coord(frag[0]+i+j, k)));
            std::deque<double> scores; 
			for (auto && j : _frags) {
				auto r = geom::suppos(*j, c);
				scores.push_back(r.rmsd);
			}
            //EACH((j, n), _frags, auto r = geom::suppos(*j, c); scores.push_back(r.rmsd));
            auto min = std::min_element(scores.begin(), scores.end()); 
            int index = std::distance(scores.begin(), min);
            auto residues = get_residues(index, c);
            //if (i == 0) FOR((j, _len_frag-1), chain.push_back(residues[j]));
			if (i == 0) for (int j = 0; j < _len_frag - 1; j++) chain.push_back(residues[j]);
            chain.push_back(residues[_len_frag-1]);
        }
        return chain;
    }

    template<typename T>
    void extract_frags(T &&model) {
        Chain dq; std::ofstream ofile(_path + "inf.txt");
		int n_res = 0;
		for (auto && chain : model) for (auto && res : chain) {
        //EACH_RES(model,
            dq.push_back(res);
            if (dq.size() >= 2 && geom::distance(dq.back()["C4*"], dq[dq.size()-2]["C4*"]) > 10) {
                dq.clear();
            } else if (dq.size() == _len_frag) {
                std::string name = model.name + "-" + JN_STR(n_res); ofile << name;
				for (int i = 0; i < _len_frag; i++) for (int j = 0; j < 3; j++) {
					auto && atom = dq[i]["C4*"];
					ofile << ' ' << atom[j];
				}
                //FOR((i, _len_frag), FOR((j, 3), auto &&atom = dq[i]["C4*"]; ofile << ' ' << atom[j]));
                ofile << '\n';
                mol_write(dq, _path + name + ".pdb");
                dq.pop_front();
            }
			n_res++;
        }
        ofile.close();
    }

};

thread_local static C2A c2a;

Chain CG1p::aa(const Mat &c, int beg, int end) {
    return c2a.run(c, std::vector<int>{beg, end});
}

} // namespace jian
