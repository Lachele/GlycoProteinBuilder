#include "glycosylationsite.h"

//////////////////////////////////////////////////////////
//                       CONSTRUCTOR                    //
//////////////////////////////////////////////////////////
GlycosylationSite::GlycosylationSite()
{
    SetGlycanName("");
    SetGlycanOverlap(0.0);
    SetProteinOverlap(0.0);
}

GlycosylationSite::GlycosylationSite(std::string glycan_name)
{
    SetGlycanName(glycan_name);
    SetGlycanOverlap(0.0);
    SetProteinOverlap(0.0);
}

GlycosylationSite::GlycosylationSite(std::string glycan_name, Residue* residue, Assembly glycan)
{
    SetGlycanName(glycan_name);
    SetResidue(residue);
    SetGlycan(glycan);
    SetGlycanOverlap(0.0);
    SetProteinOverlap(0.0);
}

GlycosylationSite::~GlycosylationSite()
{

}

//////////////////////////////////////////////////////////
//                       ACCESSOR                       //
//////////////////////////////////////////////////////////

std::string GlycosylationSite::GetGlycanName()
{
    return glycan_name_;
}

Residue* GlycosylationSite::GetResidue()
{
    return residue_;
}

Assembly* GlycosylationSite::GetAttachedGlycan()
{
    return &glycan_;
}

double GlycosylationSite::GetTotalOverlap()
{
    return (glycan_overlap_ + protein_overlap_);
}

double GlycosylationSite::GetGlycanOverlap()
{
    return glycan_overlap_;
}

double GlycosylationSite::GetProteinOverlap()
{
    return protein_overlap_;
}

//////////////////////////////////////////////////////////
//                       FUNCTIONS                      //
//////////////////////////////////////////////////////////

// Only need glycoprotein so can merge assemblies and set bonding for the connecting atom
// Bond by distance wouldn't work as may have overlaps after superimposition.
void GlycosylationSite::AttachGlycan(Assembly glycan, Assembly *glycoprotein)
{
    this->SetGlycan(glycan);
    //std::cout << "Here" << std::endl;
    glycoprotein->MergeAssembly(&glycan_); // Add glycan to glycoprotein assembly, allows SetDihedral later.
    this->Prepare_Glycans_For_Superimposition_To_Particular_Residue(residue_->GetName());
    this->Superimpose_Glycan_To_Glycosite(residue_);
}

void GlycosylationSite::Prepare_Glycans_For_Superimposition_To_Particular_Residue(std::string amino_acid_name)
{
    //Dear future self, the order that you add the atoms to the residue matters for superimposition ie N, CA, CB , not CB, CA, N.

    Residue* reducing_Residue = glycan_.GetAllResiduesOfAssembly().at(1); // I assume I assumed something stupid here.
    //std::cout << "Reducing residue is " << reducing_Residue->GetName() << std::endl;
    AtomVector reducing_Atoms = reducing_Residue->GetAtoms();

    Atom* atomC5;
    Atom* atomO5;
    Atom* atomC1;

    // Want: residue.FindAtom(string Name);
    for(AtomVector::iterator it = reducing_Atoms.begin(); it != reducing_Atoms.end(); it++)
    {
       Atom* atom = *it;
       if(atom->GetName().compare("C5")==0)
           atomC5 = atom;
       if(atom->GetName().compare("O5")==0)
           atomO5 = atom;
       if(atom->GetName().compare("C1")==0)
           atomC1 = atom;
    }
    // Delete aglycon atoms from glycan.
    Residue * aglycon = glycan_.GetAllResiduesOfAssembly().at(0);
    AtomVector aglycon_Atoms = aglycon->GetAtoms();
    for(AtomVector::iterator it = aglycon_Atoms.begin(); it != aglycon_Atoms.end(); it++)
    {
       Atom* atom = *it;
       //std::cout << "Removing " << atom->GetName() << std::endl;
       aglycon->RemoveAtom(atom);
    }

    //Ok so going to set it so that the new "superimposition residue" is the old aglycon residue i.e. .at(0)
    // This avoids having to delete the algycon residue object from assembly and adding the super residue to assembly.

   // Assembly* assembly = new Assembly();
   // Residue* super_residue = new Residue();
    //Residue* alt_residue = new Residue();
   // super_residue->SetAssembly(&glycan_);
   // glycan_.AddResidue(super_residue);
    Residue* super_residue = aglycon; // "renaming" so the below reads better.
    super_residue->SetName("SUP");
    super_residue->SetId("SUP_?_1_?_?_1");

    if (amino_acid_name.compare("ASN")==0)
    {
        //super_residue->SetName("NLN");
        //super_residue->SetId("NLN_?_1_?_?_1");

        Atom *atomND2 = new Atom(super_residue, "ND2", (gmml::get_cartesian_point_from_internal_coords(atomC5, atomO5, atomC1, 109.3, 180, 1.53)));
        Atom *atomCG = new Atom(super_residue, "CG", (gmml::get_cartesian_point_from_internal_coords(atomO5, atomC1, atomND2, 109.3, 261, 1.325)));
        Atom *atomOD1 = new Atom(super_residue, "OD1", (gmml::get_cartesian_point_from_internal_coords(atomC1, atomND2, atomCG, 126, 0, 1.22)));
   //     Atom *atomCB = new Atom(super_residue, "CB", (gmml::get_cartesian_point_from_internal_coords(atomC1, atomND2, atomCG, 114, 177.3, 1.53)));
   //     Atom *atomCA = new Atom(super_residue, "CA", (gmml::get_cartesian_point_from_internal_coords(atomND2, atomCG, atomCB, 111, 177.6, 1.53)));
   //     Atom *atomN = new Atom(super_residue, "N", (gmml::get_cartesian_point_from_internal_coords(atomCG, atomCB, atomCA, 111, 191.6, 1.453)));

        super_residue->AddAtom(atomCG);
        super_residue->AddAtom(atomOD1);
        super_residue->AddAtom(atomND2);
        superimposition_atoms_ = super_residue->GetAtoms();
    /*
        alt_residue->SetName("NLN");
        alt_residue->SetId("NLN_?_1_?_?_1");
        alt_residue->SetAssembly(&alternate_sidechain_);
        alternate_sidechain_.AddResidue(alt_residue);
        alt_residue->AddAtom(atomN);
        alt_residue->AddAtom(atomCA);
        alt_residue->AddAtom(atomCB);
        alt_residue->AddAtom(atomCG);
        alt_residue->AddAtom(atomOD1);
        alt_residue->AddAtom(atomND2);
        alternate_sidechain_.BuildStructureByDistance();
        */

       // outputPdbFile = alternate_sidechain_.BuildPdbFileStructureFromAssembly();
       // outputPdbFile->Write("/home/oliver/Programs/Cplusplus/GlycoproteinBuilder/glycoproteinBuilder/outputs/NLN_AlignedToGlycan.pdb");
    }
    else if (amino_acid_name.compare("THR")==0 || amino_acid_name.compare("SER")==0)
    {
        //super_residue->SetName("OLS");
        //super_residue->SetId("OLS_?_1_?_?_1");

        Atom *atomOG1 = new Atom(super_residue, "OG", (gmml::get_cartesian_point_from_internal_coords(atomC5, atomO5, atomC1, 112, 68, 1.46)));
        Atom *atomCB = new Atom(super_residue, "CB", (gmml::get_cartesian_point_from_internal_coords(atomO5, atomC1, atomOG1, 109.3, 75, 1.53)));
        Atom *atomCA = new Atom(super_residue, "CA", (gmml::get_cartesian_point_from_internal_coords(atomC1, atomOG1, atomCB, 109.3, 125, 1.53)));
       // Atom *atomN = new Atom(super_residue, "N", (gmml::get_cartesian_point_from_internal_coords(atomOG1, atomCB, atomCA, 109.3, 180, 1.53)));
       // Atom *atomCG2 = new Atom(super_residue, "CG2", (gmml::get_cartesian_point_from_internal_coords(atomC1, atomOG1, atomCB, 109.3, -60, 1.53)));

        super_residue->AddAtom(atomCA);
        super_residue->AddAtom(atomCB);
        super_residue->AddAtom(atomOG1);
        superimposition_atoms_ = super_residue->GetAtoms();

        /*
        alt_residue->SetName("OLS");
        alt_residue->SetId("OLS_?_1_?_?_1");
        alt_residue->SetAssembly(&alternate_sidechain_);
        alternate_sidechain_.AddResidue(alt_residue);
        alt_residue->AddAtom(atomN);
        alt_residue->AddAtom(atomCA);
        alt_residue->AddAtom(atomCB);
        alt_residue->AddAtom(atomOG1);
        */

       /* if (amino_acid_name.compare("SER")==0)
        {
            alternate_sidechain_.BuildStructureByDistance();
        }
        else
        {
            alt_residue->SetName("OLT"); //Thr = Ser + CG2
            alt_residue->SetId("OLT_?_1_?_?_1");
            alt_residue->AddAtom(atomCG2);
            atomOG1->SetName("OG1"); // It's OG in Ser.
            alternate_sidechain_.BuildStructureByDistance();
        }
        */
        if (amino_acid_name.compare("THR")==0)
        {
            atomOG1->SetName("OG1"); // It's OG in Ser.
        }
    }

    else if (amino_acid_name.compare("TYR")==0)
    {
      //  super_residue->SetName("OLY");
      //  super_residue->SetId("OLY_?_1_?_?_1");

        Atom *atomOH = new Atom(super_residue, "OH", (gmml::get_cartesian_point_from_internal_coords(atomC5, atomO5, atomC1, 112, 68, 1.46)));
        Atom *atomCZ = new Atom(super_residue, "CZ", (gmml::get_cartesian_point_from_internal_coords(atomO5, atomC1, atomOH, 117, 60, 1.35)));
        Atom *atomCE1 = new Atom(super_residue, "CE1", (gmml::get_cartesian_point_from_internal_coords(atomC1, atomOH, atomCZ, 120, 180, 1.37)));
      /*  Atom *atomCD1 = new Atom(super_residue, "CD1", (gmml::get_cartesian_point_from_internal_coords(atomOH, atomCZ, atomCE1, 120, 180, 1.37)));
        Atom *atomCE2 = new Atom(super_residue, "CE2", (gmml::get_cartesian_point_from_internal_coords(atomC1, atomOH, atomCZ, 120, 0, 1.37)));
        Atom *atomCD2 = new Atom(super_residue, "CD2", (gmml::get_cartesian_point_from_internal_coords(atomOH, atomCZ, atomCE2, 120, 180, 1.37)));
        Atom *atomCG = new Atom(super_residue, "CG", (gmml::get_cartesian_point_from_internal_coords(atomCZ, atomCE2, atomCD2, 120, 0, 1.37)));
        Atom *atomCB = new Atom(super_residue, "CB", (gmml::get_cartesian_point_from_internal_coords(atomCE2, atomCD2, atomCG, 122, 180, 1.51)));
        Atom *atomCA = new Atom(super_residue, "CA", (gmml::get_cartesian_point_from_internal_coords(atomCD2, atomCG, atomCB, 111, -107, 1.55)));
        Atom *atomN = new Atom(super_residue, "N", (gmml::get_cartesian_point_from_internal_coords(atomCG, atomCB, atomCA, 114, -170, 1.44)));
         */
        super_residue->AddAtom(atomCE1);
        super_residue->AddAtom(atomCZ);
        super_residue->AddAtom(atomOH);
        superimposition_atoms_ = super_residue->GetAtoms();
        /*
        alt_residue->SetName("OLY");
        alt_residue->SetId("OLY_?_1_?_?_1");
        alt_residue->SetAssembly(&alternate_sidechain_);
        alternate_sidechain_.AddResidue(alt_residue);
        alt_residue->AddAtom(atomN);
        alt_residue->AddAtom(atomCA);
        alt_residue->AddAtom(atomCB);
        alt_residue->AddAtom(atomOH);
        alt_residue->AddAtom(atomCZ);
        alt_residue->AddAtom(atomCE1);
        alt_residue->AddAtom(atomCD1);
        alt_residue->AddAtom(atomCE2);
        alt_residue->AddAtom(atomCD2);
        alt_residue->AddAtom(atomCG);
        alternate_sidechain_.BuildStructureByDistance();
        */
    }
    return;
}

void GlycosylationSite::Superimpose_Glycan_To_Glycosite(Residue *glycosite_residue)
{
    // Get the 3 target atoms from protein residue.
    std::cout << "Superimposing to " << glycosite_residue->GetName() << std::endl;
    AtomVector protein_atoms = glycosite_residue->GetAtoms();
    AtomVector target_atoms;
    //AtomVector super_atoms = superimposition_atoms_.GetAllAtomsOfAssembly();
  //  Assembly* assemblyTarget = new Assembly();
   // Residue* residueTarget = new Residue();
    Atom* last_protein_atom;
    for(AtomVector::iterator it1 = protein_atoms.begin(); it1 != protein_atoms.end(); ++it1)
    {
        Atom *protein_atom = (*it1);
        for(AtomVector::iterator it2 = superimposition_atoms_.begin(); it2 != superimposition_atoms_.end(); ++it2)
        {
            Atom *super_atom = (*it2);
            if (protein_atom->GetName() == super_atom->GetName())
            {
                target_atoms.push_back(protein_atom);
                //std::cout << "Here1" << std::endl;
                if (super_atom == superimposition_atoms_.back()) // This is just to set the bonding correctly.
                {
                  //  std::cout << "Here2" << std::endl;
                    last_protein_atom = protein_atom;
                }
            }
        }
    }
   // residueTarget->SetAssembly(assemblyTarget);
   // assemblyTarget->AddResidue(residueTarget);
  //  assemblyTarget->BuildStructureByDistance();

    //std::cout << "Here now" << std::endl;
   //superimposition_atoms_.Print();
   // std::cout << "How now" << std::endl;
   // glycan_.Print();

    AtomVector glycan_atoms = glycan_.GetAllAtomsOfAssembly();

    gmml::Superimpose(superimposition_atoms_, target_atoms, glycan_atoms);

    Residue* reducing_Residue = glycan_.GetAllResiduesOfAssembly().at(1); // I assume I assumed something stupid here.
    //std::cout << "Reducing residue is " << reducing_Residue->GetName() << std::endl;
    AtomVector reducing_Atoms = reducing_Residue->GetAtoms();
    Atom* atomC1;
    for(AtomVector::iterator it = reducing_Atoms.begin(); it != reducing_Atoms.end(); it++)
    {
       Atom* atom = *it;
       if(atom->GetName().compare("C1")==0)
           atomC1 = atom;
    }

    last_protein_atom->GetNode()->AddNodeNeighbor(atomC1);
    atomC1->GetNode()->AddNodeNeighbor(last_protein_atom);

    //AtomVector newSideChainAtoms = alternate_sidechain_.GetAllAtomsOfAssembly();
    //glycosite_residue->ReplaceAtomCoordinates(&newSideChainAtoms);
}

// This is a stupid way to do it. Need dihedral class but in a rush. Fix later.
void GlycosylationSite::SetDihedralAtoms(Residue* residue)
{
    AtomVector atoms = residue->GetAtoms();
    Atom *atom1, *atom2, *atom3, *atom4, *atom5;
    bool chi2 = false;
    for(AtomVector::iterator it1 = atoms.begin(); it1 != atoms.end(); ++it1)
    {
        Atom *atom = *it1;
        if ( atom->GetName().compare("N"  )==0 ) { atom1 = atom; } // All residues
        if ( atom->GetName().compare("CA" )==0 ) { atom2 = atom; } // All residues   
        if ( atom->GetName().compare("CB" )==0 ) { atom3 = atom; } // All residues
        if ( atom->GetName().compare("CG" )==0 ) { atom4 = atom; } // Asn + Tyr
        if ( atom->GetName().compare("OG1")==0 ) { atom4 = atom; } // Thr
        if ( atom->GetName().compare("OG" )==0 ) { atom4 = atom; } // Ser
        if ( atom->GetName().compare("ND2")==0 ) { atom5 = atom; chi2 = true; } // Asn
        if ( atom->GetName().compare("CD1")==0 ) { atom5 = atom; chi2 = true; } // Tyr
    }
    chi1_ = {atom1, atom2, atom3, atom4};
    if (chi2)
    {
        chi2_ = {atom2, atom3, atom4, atom5};
    }
}

//////////////////////////////////////////////////////////
//                       MUTATOR                        //
//////////////////////////////////////////////////////////

void GlycosylationSite::SetGlycanName(std::string glycan_name)
{
    glycan_name_ = glycan_name;
}

void GlycosylationSite::SetResidue(Residue* residue)
{
    residue_ = residue;
    this->SetDihedralAtoms(residue);
}

void GlycosylationSite::SetGlycan(Assembly glycan)
{
    glycan_ = glycan;
}

void GlycosylationSite::SetGlycanOverlap(double overlap)
{
    glycan_overlap_ = overlap;
}

void GlycosylationSite::SetProteinOverlap(double overlap)
{
    protein_overlap_ = overlap;
}

void GlycosylationSite::SetChi1Value(double angle, Assembly *glycoprotein)
{
    Atom *atom1 = chi1_.at(0); // horrific, fix later.
    Atom *atom2 = chi1_.at(1);
    Atom *atom3 = chi1_.at(2);
    Atom *atom4 = chi1_.at(3);
    glycoprotein->SetDihedral(atom1, atom2, atom3, atom4, angle);
}
void GlycosylationSite::SetChi2Value(double angle, Assembly *glycoprotein)
{
    Atom *atom1 = chi1_.at(0); // horrific, fix later.
    Atom *atom2 = chi1_.at(1);
    Atom *atom3 = chi1_.at(2);
    Atom *atom4 = chi1_.at(3);
    glycoprotein->SetDihedral(atom1, atom2, atom3, atom4, angle);
}
//////////////////////////////////////////////////////////
//                       DISPLAY FUNCTION               //
//////////////////////////////////////////////////////////

/*void GlycosylationSite::Print(ostream &out)
{
    std::out << "Residue ID: " << residue_->GetId() << endl;
    //out << "Glycan sequence: " << this->GetGlycanSequence() << endl;
}
*/
