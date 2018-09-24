#include "../includes/selections.h"

AtomVector selection::AtomsWithinDistanceOf(MolecularModeling::Atom *query_atom, double distance, AtomVector atoms)
{
    AtomVector atoms_within_distance;
    for(AtomVector::iterator it1 = atoms.begin(); it1 != atoms.end(); ++it1)
    {
        MolecularModeling::Atom *atom1 = (*it1);
        if (atom1->GetDistanceToAtom(query_atom) < distance )
        {
            atoms_within_distance.push_back(atom1);
        }
    }
    return atoms_within_distance;
}

// I want a generic recursive function, where I can pass in the condition(s). Lots of Repeating code here.
// This one was written before the others. Could update with previous atom being passed in, though that makes the initial call confusing...
void selection::FindAtomsConnectingResidues(Atom *current_atom, Residue *second_residue, AtomVector *connecting_atoms, bool *found_neighbor)
{
    current_atom->SetDescription("VisitedByFindAtomsConnectingResidues");
    AtomVector neighbors = current_atom->GetNode()->GetNodeNeighbors();
    for(AtomVector::iterator it1 = neighbors.begin(); it1 != neighbors.end(); ++it1)
    {
        MolecularModeling::Atom *neighbor = *it1;
        if(neighbor->GetResidue()->GetId().compare(second_residue->GetId())==0) // Should be using indices for Residues too.
        {
            *found_neighbor = true;
            connecting_atoms->push_back(current_atom);
            connecting_atoms->push_back(neighbor);
          //  std::cout << "Found the connection point: " << current_atom->GetId() << " - " << neighbor->GetId() << "\n";
        }
        // If haven't visited this atom already AND don't move onto other residues
        else if ((neighbor->GetDescription().compare("VisitedByFindAtomsConnectingResidues")!=0) && (current_atom->GetResidue()->GetId().compare(neighbor->GetResidue()->GetId())==0))
        {
            selection::FindAtomsConnectingResidues(neighbor, second_residue, connecting_atoms, found_neighbor);
        }
    }
    return;
}

// Will not ignore fused rings. Explores everything to find all cycle points. Looks for cycle point closest to start atom.
bool selection::FindCyclePoint(Atom *previous_atom, Atom *current_atom, AtomVector *atom_path, bool *found_cycle_point, Atom *&cycle_point)
{
    // Need this to explore everything. It will find same cycle point more than once, but that doesn't matter.
    current_atom->SetDescription("VisitedByFindCyclePoint");
    //std::cout << "Checking neighbors of " << current_atom->GetName() << "\n";

    atom_path->push_back(current_atom);
    AtomVector neighbors = current_atom->GetNode()->GetNodeNeighbors();
    for(AtomVector::iterator it1 = neighbors.begin(); it1 != neighbors.end(); ++it1)
    {
        MolecularModeling::Atom *neighbor = *it1;
        // If not previous atom and not from a different residue
        if ( (neighbor->GetIndex() != previous_atom->GetIndex()) && (current_atom->GetResidue()->GetId().compare(neighbor->GetResidue()->GetId())==0))
        //if ( neighbor->GetIndex() != previous_atom->GetIndex() ) // Good for testing multiple cycles
        {
           // std::cout << "Coming from previous atom " << previous_atom->GetName() << " we see bonding: " << current_atom->GetName() << "->" << neighbor->GetName() << "\n";
            if ( std::find(atom_path->begin(), atom_path->end(), neighbor) != atom_path->end() ) // If we've been at this atom before
            {
              //  std::cout << "Found a cycle point!" << std::endl;
                if(*found_cycle_point) // If there are more than one cycle points
                {
                    // Finds position of atoms in atom_path. Want earliest possible cycle point i.e. closest to start atom
                    std::ptrdiff_t new_cycle_position = std::distance(atom_path->begin(), std::find(atom_path->begin(), atom_path->end(), neighbor));
                    std::ptrdiff_t current_cycle_position = std::distance(atom_path->begin(), std::find(atom_path->begin(), atom_path->end(), cycle_point));
                    if (new_cycle_position < current_cycle_position)
                    {
                //        std::cout << "Updating cycle point to be: " << neighbor->GetId() << std::endl;
                        cycle_point = neighbor;
                    }
                }
                else
                {
                    *found_cycle_point = true;
                    cycle_point = neighbor;
             //       std::cout << "Found the cycle point to be: " << cycle_point->GetId() << "\n";
                }
            }
            if(neighbor->GetDescription().compare("VisitedByFindCyclePoint")!=0) // Don't look back!
            {
                //std::cout << "DEEPER STILL\n";
                selection::FindCyclePoint(current_atom, neighbor, atom_path, found_cycle_point, cycle_point);
            }
        }
    }
    return *found_cycle_point;
}

bool selection::FindPathBetweenTwoAtoms(Atom *current_atom, Atom *target_atom, AtomVector *atom_path, bool *found)
{
    //atom_path->push_back(current_atom);
    current_atom->SetDescription("VistedByFindPathBetweenTwoAtoms");
    AtomVector neighbors = current_atom->GetNode()->GetNodeNeighbors();
    for(AtomVector::iterator it1 = neighbors.begin(); it1 != neighbors.end(); ++it1)
    {
        MolecularModeling::Atom *neighbor = *it1;
        if(neighbor->GetIndex() == target_atom->GetIndex())
        {
            *found = true;
            atom_path->push_back(neighbor);
        }
        // If not found && not previously visited atom && ( if neighbor residue is current residue || target_atom residue)
        if ( (*found == false) && (neighbor->GetDescription().compare("VistedByFindPathBetweenTwoAtoms")!=0) && ((current_atom->GetResidue()->GetId().compare(neighbor->GetResidue()->GetId())==0) || (target_atom->GetResidue()->GetId().compare(neighbor->GetResidue()->GetId())==0)) )
        {
          //  std::cout << "STEEEPER\n";
            selection::FindPathBetweenTwoAtoms(neighbor, target_atom, atom_path, found);
        }
    }
    if(*found)
    {
        atom_path->push_back(current_atom);
    }
    return *found;
}

void selection::ClearAtomDescriptions(Residue *residue)
{
    AtomVector atoms = residue->GetAtoms();
    for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); ++it)
    {
        Atom *atom = *it;
        atom->SetDescription("");
    }
    return;
}

// Below doesn't handle non-protein non-cyclics and would fail to find inner rotatable bonds here:
/*
   __    __
__/  \__/  \__
  \__/  \__/

  If this was within one residue
*/
AtomVector selection::FindRotationPoints(Atom *atom)
{
    AtomVector rotation_points;
    AtomVector atom_path;
    bool found = false;
    if(atom->GetResidue()->CheckIfProtein())
    {
        Atom *caAtom = atom->GetResidue()->GetAtom("CA");
        // Find any cycle points. Note: starting from connection point to other residue
        Atom *cycle_point;
        found = false;
        atom_path.clear();
        if(selection::FindCyclePoint(atom, atom, &atom_path, &found, cycle_point))
        {
            rotation_points.push_back(cycle_point);
            found = false;
            atom_path.clear();
            //Want residue.clearAtomDescriptions
            selection::ClearAtomDescriptions(atom->GetResidue());
            selection::FindCyclePoint(caAtom, caAtom, &atom_path, &found, cycle_point);
            rotation_points.push_back(cycle_point);
        }
        // Want this at end of vector
        rotation_points.push_back(caAtom);

    }
    else // Non protein. Must have cycles for code to work!
    {
        Atom *cycle_point;
        atom_path.clear();
        found = false;
        //Find path to first cycle atom, i.e. anomeric carbon
        if(selection::FindCyclePoint(atom, atom, &atom_path, &found, cycle_point))
        {
            rotation_points.push_back(cycle_point);
        }
    }
    return rotation_points;
}

// Find a neighbor of a cycle point to define the dihedral. Must not be in path used to come to this cycle_point.
// Oh gawd this code is horrible.
// The logic for selecting the atom to define a dihedral is messy.
// Comparing strings is messy when I really care about the number of e.g. C2 Vs O5, but need to factor in a C2 vs O comparision
// Ok so: If cycle_point name is "CA", then set "N" atom always. Otherwise highest numbered or random
Atom* selection::FindCyclePointNeighbor(const AtomVector atom_path, Atom *cycle_point)
{
    Atom *selected_neighbor;
    if (cycle_point->GetName().compare("CA")==0)
    {
        selected_neighbor = cycle_point->GetResidue()->GetAtom("N");
    }
    else
    {
        AtomVector neighbors = cycle_point->GetNode()->GetNodeNeighbors();
        // Ok must first get a list of neighbors that weren't in the connection path
        AtomVector good_neighbors; // Couldn't think of a better name. Everybody needs these.
        for(AtomVector::iterator it1 = neighbors.begin(); it1 != neighbors.end(); ++it1)
        {
            Atom *neighbor = *it1;
            if ( ! (std::find(atom_path.begin(), atom_path.end(), neighbor) != atom_path.end()) ) // If we've NOT been at this atom on way to cycle point
            {
                good_neighbors.push_back(neighbor);
            }
        }
        selected_neighbor = good_neighbors.at(0); // Set to any to start. If there are not good_neighbors then deserve to crash and burn
        for(AtomVector::iterator it1 = good_neighbors.begin(); it1 != good_neighbors.end(); ++it1)
        {
            Atom *neighbor = *it1;
            if(selected_neighbor->GetName().size() >= 2)
            {
                if(neighbor->GetName().size() >= 2) // This is the only time I want to compare and select the larger number
                {
                    if(neighbor->GetName().at(1) > selected_neighbor->GetName().at(1))
                    {
                        selected_neighbor = neighbor;
                    }
                } // Otherwise any neighbor is ok. Yes I'm comparing char's, but that is fine unless C9 Vs C10, but in that case I don't care again.
            }
        }
    }
    return selected_neighbor;
}



