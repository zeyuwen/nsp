#include "../pdb.hpp"
#include "../geom.hpp"
#include "DistAnal.hpp"

namespace jian {

#define FREE_COORDS do {\
	for (unsigned int i = 0; i < len; i++) {\
		delete m_coords[i];\
	}\
} while (0)

#define PRINT_MAT3(a) do { \
	unsigned int i, j, k; \
	for (i = 0; i < 85; i++) {\
		for (j = 0; j < 85; j++) {\
			for (k = 0; k < bins; k++) {\
				std::cout << a[(i * 85 + j) * bins + k] << ' ';\
			}\
			std::cout << std::endl;\
		}\
	}\
} while (0)


	DistAnal & DistAnal::init(double interval, int cutoff) {
		this->interval = interval;
		this->cutoff = cutoff;
		bins = int(ceil(cutoff / interval));
		m_freqs.resize(85 * 85 * bins);
		m_counts.resize(85 * 85 * bins);
		return *this;
	}

	DistAnal::~DistAnal() {
		FREE_COORDS;
	}

	int DistAnal::res_type(std::string name) {
		return (name == "A" ? 1 : (name == "U" ? 2 : (name == "G" ? 3 : (name == "C" ? 4 : (name == "T" ? 2 : -1)))));
	}

	void DistAnal::read_mol(const Chain &chain) {
		len = chain.size();
		num.resize(len);
		type.resize(len);
		size_nt.resize(len);
		FREE_COORDS;
		m_coords.resize(len);

		Point p;
		int m = 0, n = 0;
		for (auto && res : chain) {
			m++;
			size_nt[n] = res.size();
			type[n] = res_type(res.name);
			m_coords[n] = new Point[size_nt[n]];
			int k = 0;
			for (auto && atom : res) {
				for (int l = 0; l < 3; l++) m_coords[n][k][l] = atom[l];
				m_coords[n][k][3] = (res[0].name == "P" ? 0 : 3);
				m_coords[n][k][3] += (type[n] == 1 ? k : (type[n] == 2 ? 22 + k : (type[n] == 3 ? 42 + k : 65 + k)));
				if (atom.name == "O5*" && n != 0) {
					double dist = geom::distance(m_coords[n][k], p);
					if (dist > 4) m += 10000;
				}
				else if (atom.name == "O3*") {
					for (int l = 0; l < 3; l++) p[l] = m_coords[n][k][l];
				}
				k++;
			}
			num[n] = m;
			n++;
		}
	}

	DistAnal & DistAnal::train(const Chain & c) {
		unsigned int i, j, k, l, type1, type2;
		double temp;

		read_mol(c);
		for (i = 0; i < len; i++) {
			for (j = i + 1; j < len; j++) {
				for (k = 0; k < size_nt[i]; k++) {
					for (l = 0; l < size_nt[j]; l++) {
						type1 = static_cast<unsigned int>(m_coords[i][k][3]);
						type2 = static_cast<unsigned int>(m_coords[j][l][3]);
						if (num[j] - num[i] == 1 && !in_base(type1) && !in_base(type2)) continue;
						temp = geom::distance(m_coords[i][k], m_coords[j][l]);
						if (temp >= cutoff) continue;
						m_counts[static_cast<unsigned int>(m_coords[i][k][3] * 85 + m_coords[j][l][3]) * bins +
							     static_cast<unsigned int>(temp / interval)]++;
						m_counts[static_cast<unsigned int>(m_coords[j][l][3] * 85 + m_coords[i][k][3]) * bins +
							     static_cast<unsigned int>(temp / interval)]++;
					}
				}
			}
		}
		return *this;
	}

	bool DistAnal::in_base(unsigned int type) {
		return (type > 11 && type < 22) ||
			   (type > 33 && type < 42) ||
			   (type > 54 && type < 62) ||
			   (type > 74 && type < 85);
	}

	DistAnal & DistAnal::run(const Chain &chain) {
		unsigned int i, j, k, l, type1, type2;
		double a, b, temp;

		read_mol(chain);
		score = 0;
		for (i = 0; i < len; i++) {
			for (j = i + 1; j < len; j++) {
				for (k = 0; k < size_nt[i]; k++) {
					for (l = 0; l < size_nt[j]; l++) {
						type1 = m_coords[i][k][3];
						type2 = m_coords[j][l][3];
						if (num[j] - num[i] == 1 && !in_base(type1) && !in_base(type2)) continue;
						temp = geom::distance(m_coords[i][k], m_coords[j][l]);
						if (temp >= cutoff) continue;
						a = m_freqs[int(m_coords[i][k][3] * 85 + m_coords[j][l][3]) * bins + int(temp / interval)];
						b = m_freqs[int(m_coords[j][l][3] * 85 + m_coords[i][k][3]) * bins + int(temp / interval)];
						score += (a == 0 ? penalty : (-log(a) * ((in_base(type1) && in_base(type2)) ? 2.5 : 1)));
						score += (b == 0 ? penalty : (-log(b) * ((in_base(type1) && in_base(type2)) ? 2.5 : 1)));
					}
				}
			}
		}
		score = score / (len * (len - 1));
		return *this;
	}

	void DistAnal::read_freqs(std::string filename) {
		unsigned int i, j, k;
		std::ifstream ifile;

		FOPEN(ifile, filename);
		for (i = 0; i < 85; i++) {
			for (j = 0; j < 85; j++) {
				for (k = 0; k < bins; k++) {
					ifile >> m_freqs[(i * 85 + j) * bins + k];
				}
			}
		}
		FCLOSE(ifile);
	}

	void DistAnal::print_freqs() const {
		PRINT_MAT3(m_freqs);
	}

	void DistAnal::print_counts() const {
		PRINT_MAT3(m_counts);
	}


} // namespace jian
