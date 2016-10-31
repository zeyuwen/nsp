#pragma once

#include "../matrix.hpp"
#include "../pdb/Chain.hpp"
#include "CG.hpp"

namespace jian {

	class CGpsb : public CG {
	public:
		static const std::vector<std::string> m_basic_atoms;

		CGpsb();
		virtual Residue to_cg(const Residue &r) const;
		virtual int res_size() const;
	};

} // namespace jian


