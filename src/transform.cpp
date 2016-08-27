#include <string>
#include "nsp.hpp"
#include <jian/nuc3d/transform.hpp>

namespace jian {

REGISTER_NSP_COMPONENT(transform) {
    auto && m = mol_read_to<Model>(par.get("s"));
    std::cout << transform(m, par["seq"][0], par["type"][0]) << std::endl;
}

} // namespace jian

