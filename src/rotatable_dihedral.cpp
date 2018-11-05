#include "../includes/rotatable_dihedral.h"
#include <random>
#include "../includes/pcg_random.hpp"

// Seed with a real random value, if available
static pcg_extras::seed_seq_from<std::random_device> seed_source;
// Make a random number engine
static pcg32 rng(seed_source);

//////////////////////////////////////////////////////////
//                       CONSTRUCTOR                    //
//////////////////////////////////////////////////////////

// Of the next three forms, I'll probably use only one and delete the others later
Rotatable_dihedral::Rotatable_dihedral(Atom *atom1, Atom *atom2, Atom *atom3, Atom *atom4)
{
    AtomVector atoms {atom1, atom2, atom3, atom4};
    this->Initialize(atoms);
}

Rotatable_dihedral::Rotatable_dihedral(AtomVector atoms)
{
    this->Initialize(atoms);
}

Rotatable_dihedral::Rotatable_dihedral(AtomVector atoms, AtomVector atoms_that_move)
{
    this->SetAtoms(atoms);
    this->SetAtomsThatMove(atoms_that_move);
}

//////////////////////////////////////////////////////////
//                       ACCESSOR                       //
//////////////////////////////////////////////////////////

double Rotatable_dihedral::CalculateDihedralAngle() const
{
    GeometryTopology::Coordinate* a1 = atom1_->GetCoordinate();
    GeometryTopology::Coordinate* a2 = atom2_->GetCoordinate();
    GeometryTopology::Coordinate* a3 = atom3_->GetCoordinate();
    GeometryTopology::Coordinate* a4 = atom4_->GetCoordinate();

    GeometryTopology::Coordinate b1 = a2;
    b1.operator -(*a1);
    GeometryTopology::Coordinate b2 = a3;
    b2.operator -(*a2);
    GeometryTopology::Coordinate b3 = a4;
    b3.operator -(*a3);
    GeometryTopology::Coordinate b4 = b2;
    b4.operator *(-1);

    GeometryTopology::Coordinate b2xb3 = b2;
    b2xb3.CrossProduct(b3);

    GeometryTopology::Coordinate b1_m_b2n = b1;
    b1_m_b2n.operator *(b2.length());

    GeometryTopology::Coordinate b1xb2 = b1;
    b1xb2.CrossProduct(b2);

    double current_dihedral_angle = atan2(b1_m_b2n.DotProduct(b2xb3), b1xb2.DotProduct(b2xb3));

    return current_dihedral_angle;
}

AtomVector Rotatable_dihedral::GetAtoms() const
{
    AtomVector atoms = {atom1_, atom2_, atom3_, atom4_};
    return atoms;
}

AtomVector Rotatable_dihedral::GetAtomsThatMove()
{
    return atoms_that_move_;
}

double Rotatable_dihedral::GetPreviousDihedralAngle()
{
    return previous_dihedral_angle_;
}

//////////////////////////////////////////////////////////
//                       MUTATOR                        //
//////////////////////////////////////////////////////////

void Rotatable_dihedral::DetermineAtomsThatMove()
{
    AtomVector atoms_that_move;
    atoms_that_move.push_back(atom2_);
    atom3_->FindConnectedAtoms(atoms_that_move);
    this->SetAtomsThatMove(atoms_that_move);
}


void Rotatable_dihedral::SetPreviousDihedralAngle()
{
    this->SetDihedralAngle(this->GetPreviousDihedralAngle());
}


double Rotatable_dihedral::RandomizeDihedralAngle()
{
    return Rotatable_dihedral::RandomizeDihedralAngleWithinRange(0.0, 360.0);
    //return (rand() % 360) + 1 - 180; // Can get same one everytime for testing
}

double Rotatable_dihedral::RandomizeDihedralAngleWithinRange(double min, double max)
{
//    std::random_device rd1; // obtain a random number from hardware
//    std::mt19937 eng1(rd1()); // seed the generator
    std::uniform_real_distribution<> angle_distribution(min, max); // define the range

//    double random_angle = angle_distribution(eng1);
    double random_angle = angle_distribution(rng);
    std::cout << "Random angle is: " << random_angle << "\n";

    /*******************************************/
    /*               IMPORTANT                 */
    /*******************************************/

    this->SetDihedralAngle(random_angle); // THIS IS IMPORTANT!!! THIS SHOULD BE SEPARATED?!?! The two other functions call this one. Seems fragile.

    /*******************************************/
    /*               IMPORTANT                 */
    /*******************************************/

    return random_angle;
    //return rand() % (max + 1 - min) + min; // Can get same one everytime for testing
}

double Rotatable_dihedral::RandomizeDihedralAngleWithinRanges(std::vector<std::pair<double,double>> ranges)
{
    // For usage, can do ranges.emplace_back(min, max);
    // Pass in a vector of pairs of ranges.
    // First select one of those ranges.
    // Then create an angle within the selected range.

    // Rando stuff from slack overflow:
//    std::random_device rd; // obtain a random number from hardware
//    std::mt19937 eng(rd()); // seed the generator
    std::uniform_int_distribution<> distr(0, (ranges.size() - 1)); // define the range

    // Select one of the ranges
    int range_selection = distr(rng);

    // create an angle within the selected range
    return Rotatable_dihedral::RandomizeDihedralAngleWithinRange(ranges.at(range_selection).first, ranges.at(range_selection).second);
}

void Rotatable_dihedral::SetMetadata(gmml::MolecularMetadata::GLYCAM::DihedralAngleDataVector metadataVector)
{
    assigned_metadata_ = metadataVector;
}

void Rotatable_dihedral::AddMetadata(gmml::MolecularMetadata::GLYCAM::DihedralAngleData metadata)
{
    assigned_metadata_.push_back(metadata);
}

void Rotatable_dihedral::SetDihedralAngleUsingMetadata(bool use_ranges)
{
    if (assigned_metadata_.empty())
    {
        std::cout << "Error in Rotatable_dihedral::SetDihedralAngleUsingMetadata; no metadata has been set.\n";
    }
    else if(assigned_metadata_.size() == 1)
    {
        for (const auto& entry : assigned_metadata_)
        {
            double lower = entry.default_angle_value_;
            double upper = entry.default_angle_value_;
            if (use_ranges)
            {
                lower = (entry.default_angle_value_ - entry.lower_deviation_) ;
                upper = (entry.default_angle_value_ + entry.upper_deviation_) ;
            }
        }
        Rotatable_dihedral::RandomizeDihedralAngleWithinRange(lower, upper);
    }
    else if(assigned_metadata_.size() >= 2)
    {
        std::vector<std::pair<double,double>> ranges;
        for (const auto& entry : assigned_metadata_)
        {
            double lower = entry.default_angle_value_;
            double upper = entry.default_angle_value_;
            if (use_ranges)
            {
                lower = (entry.default_angle_value_ - entry.lower_deviation_) ;
                upper = (entry.default_angle_value_ + entry.upper_deviation_) ;
            }
            ranges.emplace_back(lower, upper);
        }
        Rotatable_dihedral::RandomizeDihedralAngleWithinRanges(ranges);
    }
    return;
}

//////////////////////////////////////////////////////////
//                  PRIVATE FUNCTIONS                   //
//////////////////////////////////////////////////////////

void Rotatable_dihedral::Initialize(AtomVector atoms)
{
    this->SetAtoms(atoms);
    this->DetermineAtomsThatMove();
}

void Rotatable_dihedral::SetAtoms(AtomVector atoms)
{
    atom1_ = atoms.at(0);
    atom2_ = atoms.at(1);
    atom3_ = atoms.at(2);
    atom4_ = atoms.at(3);
}

void Rotatable_dihedral::SetAtomsThatMove(AtomVector atoms)
{
    atoms_that_move_ = atoms;
}

void Rotatable_dihedral::RecordPreviousDihedralAngle(double dihedral_angle)
{
    previous_dihedral_angle_ = dihedral_angle;
}

//////////////////////////////////////////////////////////
//                       DISPLAY FUNCTION               //
//////////////////////////////////////////////////////////

void Rotatable_dihedral::Print()
{
    std::cout << atom1_->GetName() << ", " << atom2_->GetName() << ", " << atom3_->GetName() << ", " << atom4_->GetName() << ": " << this->CalculateDihedralAngle() << ".\n";
//    for(AtomVector::iterator it1 = atoms_that_move_.begin(); it1 != atoms_that_move_.end(); ++it1)
//    {
//        Atom *atom = *it1;
//        std::cout << atom->GetName() << ", ";
//    }
    std::cout << std::endl;
}

//////////////////////////////////////////////////////////
//                       OPERATORS                      //
//////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const Rotatable_dihedral& rotatable_dihedral)
{
    AtomVector atoms = rotatable_dihedral.GetAtoms();
    os << atoms.at(0)->GetName() << ", " << atoms.at(1)->GetName() << ", " << atoms.at(2)->GetName() << ", " << atoms.at(3)->GetName() << ": " << rotatable_dihedral.CalculateDihedralAngle() << ".\n";
    return os;
} // operator<<
