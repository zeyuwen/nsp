#include "Convert.h"

namespace jian {

void Convert::operator ()(Residue &res, std::string name) {
    if (res.name == name) return;

    /// Sort atoms of res
    pdb::Format format;
    format.sort(res);

    /// Get information
    MatrixXf res_coord; 
    std::vector<int> res_nail; 
    std::vector<int> res_direct;
    std::tie(res_coord, res_nail, res_direct) = get_res_coord(res);

    MatrixXf base_coord; 
    std::vector<int> base_nail; 
    std::vector<int> base_direct;
    std::tie(base_coord, base_nail, base_direct) = read_base(name);
 
    /// Superpose axis
    geometry::superpose(base_coord, base_nail, res_coord, res_nail);

    /// Calculate base direction
    RowVector3f direct1 = geometry::normal_vector(
        base_coord.row(base_direct[0]),
        base_coord.row(base_direct[1]),
        base_coord.row(base_direct[2])
    );
    RowVector3f direct2 = geometry::normal_vector(
        res_coord.row(res_direct[0]),
        res_coord.row(res_direct[1]),
        res_coord.row(res_direct[2])
    );


    /// Calculate rotate angle
    auto rotate_angle = geometry::dihedral(
        base_coord.row(base_nail[0]) + direct1,
        base_coord.row(base_nail[0]),
        base_coord.row(base_nail[1]),
        base_coord.row(base_nail[1]) + direct2
    );

    /// Rotate base
    VectorXf beg = base_coord.row(base_nail[0]);
    VectorXf end = base_coord.row(base_nail[1]);
    geometry::rotate(base_coord, beg, end, rotate_angle);

    /// Set residue coordiantes size
    std::set<std::string> dna_name_list{"DA", "DT", "DG", "DC"};
    std::set<std::string> rna_name_list{"A", "U", "G", "C"};

    int old_phos_size, old_sugar_size, old_base_size;
    std::tie(old_phos_size, old_sugar_size, old_base_size) = get_res_size(res);

    int new_phos_size, new_sugar_size, new_base_size;
    new_phos_size = old_phos_size;
    new_sugar_size = (dna_name_list.count(name) ? 8 : 9);
    BOOST_ASSERT(new_sugar_size == old_sugar_size && "jian::Convert error! Please check the residue to be converted.");
    new_base_size = std::map<std::string, int>{{"A", 10}, {"U", 8}, {"G", 11}, {"C", 8}, {"DA", 10}, {"DT", 9}, {"DG", 11}, {"DC", 8}}[name];

    MatrixXf coord(new_phos_size + new_sugar_size + new_base_size, 3);

    /// Copy phosphate and sugar coordinates
    if (rna_name_list.count(res.name) && dna_name_list.count(name)) {
        coord.topRows(new_phos_size + new_sugar_size - 1) = res_coord.topRows(old_phos_size + old_sugar_size - 2);
    } else if (dna_name_list.count(res.name) && rna_name_list.count(name)) {
        coord.topRows(new_phos_size + new_sugar_size - 2) = res_coord.topRows(old_phos_size + old_sugar_size - 1);
        coord.row(new_phos_size + new_sugar_size - 2) = get_o2_coord(res);
    } else {
        coord.topRows(new_phos_size + new_sugar_size) = res_coord.topRows(old_phos_size + old_sugar_size);
    }

    /// Copy base coordinates
    coord.bottomRows(base_coord.rows()) = base_coord;

    /// Make residue
    res = make_residue(name, coord);
}

std::tuple<MatrixXf, std::vector<int>, std::vector<int>> Convert::get_res_coord(const Residue &res) {
    MatrixXf res_coord(res.atoms.size(), 3);
    std::vector<int> res_nail(2);
    std::vector<int> res_direct(3);
    for (int i = 0; i < res.atoms.size(); i++) {
        const Atom &atom = res.atoms[i];
        res_coord(i, 0) = atom.x;
        res_coord(i, 1) = atom.y;
        res_coord(i, 2) = atom.z;
        if (atom.name == "C1*") {
            res_nail[0] = i;
        } else if (std::set<std::string>{"A", "G", "DA", "DG"}.count(res.name) && atom.name == "N9") {
            res_nail[1] = i;
        } else if (std::set<std::string>{"U", "C", "DT", "DC"}.count(res.name) && atom.name == "N1") {
            res_nail[1] = i;
        } else if (atom.name == "C2") {
            res_direct[0] = i;
        } else if (atom.name == "C4") {
            res_direct[1] = i;
        } else if (atom.name == "C6") {
            res_direct[2] = i;
        }
    }
    if (std::set<std::string>{"U", "C", "DT", "DC"}.count(res.name)) {
        std::swap(res_direct[1], res_direct[2]);
    }
    return std::make_tuple(res_coord, res_nail, res_direct);
}

void Convert::set_res_coord(Residue &res, const MatrixXf &res_coord) {
    for (int i = 0; i < res.atoms.size(); i++) {
        res.atoms[i].x = res_coord(i, 0);
        res.atoms[i].y = res_coord(i, 1);
        res.atoms[i].z = res_coord(i, 2);
    }
}

std::tuple<MatrixXf, std::vector<int>, std::vector<int>> Convert::read_base(std::string name) {
    std::string file_name = _lib;
    if (std::set<std::string>{"A", "U", "G", "C"}.count(name)) {
        file_name += "/RNA/base/" + name + ".pdb";
    } else if (std::set<std::string>{"DA", "DT", "DG", "DC"}.count(name)) {
        file_name += "/DNA/base/" + name + ".pdb";
    }
    return get_res_coord(Model(file_name).chains[0].residues[0]);
}

std::tuple<int, int, int> Convert::get_res_size(const Residue &res) {
    std::set<std::string> phos_atoms{"P", "O1P", "O2P"};
    int phos_size = 0, sugar_size = 0, base_size = 0;
    for (auto &&atom: res.atoms) {
        if (phos_atoms.count(atom.name)) {
            phos_size++;
        } else if (atom.name.size() == 3) {
            sugar_size++;
        } else {
            base_size++;
        }
    }
    return make_tuple(phos_size, sugar_size, base_size);
}

RowVector3f Convert::get_o2_coord(const Residue &res) {
    RowVector3f c1, c2, c3, o2;
    for (auto &&atom: res.atoms) {
        if (atom.name == "C1*") {
            c1 = atom.pos<RowVector3f>();
        } else if (atom.name == "C2*") {
            c2 = atom.pos<RowVector3f>();
        } else if (atom.name == "C3*") {
            c3 = atom.pos<RowVector3f>();
        }
    }
    o2 = (c3 - c2).cross(c1 - c2) + (c3 + c1 - 2 * c2) + c2;
    return o2;
}

} /// namespace jian
