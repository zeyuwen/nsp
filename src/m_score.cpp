#include "nsp.hpp"
#include <jian/geom.hpp>
#include <jian/pdb.hpp>
#include <jian/scoring/ScoreAa.hpp>
#include <jian/scoring/Score.hpp>
#include <jian/utils/file.hpp>
#include <jian/nuc3d/Format.hpp>

namespace jian {
	namespace {
		Format formater;

		void read_chain(Chain &chain, std::string filename) {
			chain_read_model(chain, filename);
			chain = formater(chain);
		}

		void sum_counts(std::string filename, int rows, int cols) {
			Veci v;
			int i, j, d;
			std::ifstream ifile;

			v = Veci::Zero(cols);
			FOPEN(ifile, filename);
			for (i = 0; i < rows; i++) {
				for (j = 0; j < cols; j++) {
					ifile >> d;
					v[j] += d;
				}
			}
			FCLOSE(ifile);

			for (i = 0; i < cols; i++) {
				std::cout << v[i] << ' ';
			}
			std::cout << std::endl;
		}

		void score_res(ScoreBase * scoring, std::string filename, std::string score_type = "pairing") {
			Chain chain;
			int i, j, l;

			chain_read_model(chain, filename);
			l = chain.size();
			//std::cout << l << std::endl;
			for (i = 0; i < l; i++) {
				for (j = 0; j < l; j++) {
					if (i == j) std::cout << 0 << "\t";
					else {
						scoring->en_bp(chain[i], chain[j]);
						if (score_type == "pairing") std::cout << scoring->m_en_pairing << "\t";
						if (score_type == "stacking") std::cout << scoring->m_en_stacking << "\t";
						else if (score_type == "wc") std::cout << scoring->m_en_wc << "\t";
						else if (score_type == "nwc") std::cout << scoring->m_en_nwc << "\t";
						else throw "error!";
					}
				}
				std::cout << std::endl;
			}
		}

		void score_s(ScoreBase * scoring, std::string filename) {
			Chain chain;
			chain_read_model(chain, filename);
			scoring->run(chain);
			std::cout <<
				"Score of " << filename << ": " <<
				//scoring->m_score_dih << "(dih) " <<
				//scoring->m_score_dist << "(dist) " <<
				scoring->m_score << "(total)" <<
				std::endl;
		}

		void score_l(ScoreBase * scoring, std::string filename) {
			EACH_SPLIT_LINE(filename, " ",
				score_s(scoring, F[0]);
			);
		}

		void train_s(ScoreBase * scoring, std::string filename) {
			Chain chain;

			std::cout << "Train " << filename << " ..." << std::endl;
			chain_read_model(chain, filename);
			scoring->train(chain);
		}

		void train_l(ScoreBase * scoring, std::string filename) {
			EACH_SPLIT_LINE(filename, " ",
				train_s(scoring, F[0]);
			);
		}

		double en_crash(const Residue &r1, const Residue &r2) {
			int i, j;
			double d, e;

			e = 0;
			for (i = 0; i < 3; i++) {
				for (j = 0; j < 3; j++) {
					d = geom::distance(r1[i], r2[j]);
					if (i == 0 || j == 0) {
						if (d < 5) {
							e += square(d - 4);
						}
					}
					else if ((i == 1 || j == 1) && d < 5) {
						e += square(d - 5);
					}
					else if (d < 3.5) {
						e += square(d - 3.5);
					}
				}
			}
			return e;
		}

		REGISTER_NSP_COMPONENT(score) {
			//std::ofstream stream;
			std::ofstream ofile;
			std::ostream stream(std::cout.rdbuf());
			std::string method;
			std::string score_type = "pairing";

			par.set(score_type, "score_type");

			CG *m_cg;
			//std::streambuf *buf = stream.rdbuf();

			if (par.has("out", "o") && par.get("out", "o").size() > 0) {
				FOPEN(ofile, par.get("out", "o"));
				stream.rdbuf(ofile.rdbuf());
			}

			method = "aa";
			par.set(method, "cg");

			m_cg = CG::fac_t::create(method);

			if (par.has("crash")) {
				double e, d;
				Chain chain;
				int i, j, l;

				chain_read_model(chain, par.get("s"));
				chain = m_cg->to_cg(chain);
				l = chain.size();
				e = 0;
				for (i = 0; i < l; i++) {
					for (j = 0; j < l; j++) {
						d = (i == j ? 0 : en_crash(chain[i], chain[j]));
						e += d;
						stream << d << "\t";
					}
					stream << std::endl;
				}
				std::cout << e << std::endl;

			}
			else if (par.has("sum_counts")) {
				Par::pars_t & pars = par["sum_counts"];
				std::string filename = pars[0];
				int rows = std::stoi(pars[1]);
				int cols = std::stoi(pars[2]);
				sum_counts(filename, rows, cols);
			}
			else {
				ScoreBase *scoring = ScoreBase::fac_t::create(method);
				scoring->init();

				if (par.has("print_freqs")) {
					scoring->print_freqs(stream);
				}
				else if (par.has("print_counts")) {
					scoring->print_counts(stream);
				}
				else if (par.has("train")) {
					if (par.has("s)")) {
						train_s(scoring, par.get("s"));
					}
					else if (par.has("l")) {
						train_l(scoring, par.get("l", "list"));
					}
					scoring->print_counts(stream);
				}
				else {
					if (par.has("s")) {
						if (par.has("res")) {
							score_res(scoring, par.get("s"), score_type);
						}
						else {
							score_s(scoring, par.get("s"));
						}
					}
					else if (par.has("l")) {
						score_l(scoring, par.get("l", "list"));
					}
				}
				delete scoring;
			}
			FCLOSE(ofile);
			delete m_cg;
		}
	}
} // namespace jian
















