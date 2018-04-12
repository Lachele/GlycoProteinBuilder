#include "../includes/resolve_overlaps.h"

typedef std::vector<GlycosylationSite*> GlycosylationSitePointerVector;

using namespace gmml;

/* New algorithm idea:
 * For each site that has overlap:
 1) Try to resolve all protein overlaps, ignoring glycan-glycan. Delete glycan from any site with unresolvable protein overlaps, report to user.
 2) Resolve glycan overlaps. Either
    a) Loop through and check every site each cycle, and move those with overlaps

    c) Create a tree structure for sites that overlap with each other.
        Could use assembly index/id of bead atom? Can work on individual trees and report nice info to users.


Note: Chi1 is 180,-60,60 +/- 30 degrees. Want a function that keeps the values within these states.
Note: Need to save best structure.
*/


void resolve_overlaps::protein_first_monte_carlo(MolecularModeling::Assembly &glycoprotein, GlycosylationSiteVector &glycosites)
{
    /* Algorithm plan:
     *  Resolve all protein overlap first, reject sites that cannot be resolved. Record cycles to resolve?
     *  Calculate protein overlaps, for each site with overlaps
     *        Randomly change all chi1 and chi2 values
     *  Once finished;
     *  Calculate glycan overlaps, for each site with overlaps
     *        Change chi1 and chi2, scaled by degree of overlap
     *        Reject changes causing protein overlaps
     */
    double strict_tolerance = 0.1, loose_tolerance = 1.0; // Aim for <0.1 when resolving, but keep any less than 1 when culling.
    int max_cycles = 10;
    GlycosylationSitePointerVector sites_with_protein_overlaps = DetermineSitesWithOverlap(glycosites, strict_tolerance, "with", "protein");
    Monte_Carlo_Torsions(sites_with_protein_overlaps, glycosites, glycoprotein, max_cycles, strict_tolerance, "protein");
    SetBestChi1Chi2(sites_with_protein_overlaps, glycoprotein, "protein");     //Remove sites that could not be resolved.
    GlycosylationSitePointerVector remaining_sites = DeleteSitesWithOverlaps(glycoprotein, glycosites, loose_tolerance, "protein");
    Monte_Carlo_Torsions(remaining_sites, glycosites, glycoprotein, max_cycles, strict_tolerance, "total");
    SetBestChi1Chi2(remaining_sites, glycoprotein);
    GlycosylationSitePointerVector resolved_sites = DeleteSitesWithOverlaps(glycoprotein, glycosites, loose_tolerance, "total");
    //PrintOverlaps(glycosites);
}

void resolve_overlaps::dumb_monte_carlo(Assembly &glycoprotein, GlycosylationSiteVector &glycosites)
{
    /* Algorithm:
     * Determine which sites have overlaps greater than tolerance. Stop if zero sites.
     * For each site with overlaps:
     *        Randomly change all chi1 and chi2 values
     */
    double tolerance = 0.1;
    int cycle = 0, max_cycles = 5;
    GlycosylationSitePointerVector sites_with_overlaps = DetermineSitesWithOverlap(glycosites, tolerance);
    bool stop = false;

    while ( (cycle < max_cycles) && (stop == false) )
    {
        ++cycle;
        std::cout << "Cycle " << cycle << " of " << max_cycles << std::endl;
        for(GlycosylationSitePointerVector::iterator it1 = sites_with_overlaps.begin(); it1 != sites_with_overlaps.end(); ++it1)
        {
            GlycosylationSite *current_glycosite = (*it1);

            current_glycosite->SetChi1Value(RandomAngle_360range(), glycoprotein);

           // current_glycosite->SetChi2Value(RandomAngle_360range(), glycoprotein);
            //double percent_overlap = ((current_glycosite->GetTotalOverlap() / (current_glycosite->GetAttachedGlycan()->GetAllAtomsOfAssembly().size()) ) + 0.01);
            //  new_dihedral_value = RandomAngle_PlusMinusX(current_glycosite->GetChi1Value(), (180 * percent_overlap) ); // scaled to degree of overlap
            //        sites_with_overlaps.erase(std::remove(sites_with_overlaps.begin(), sites_with_overlaps.end(), *it1), sites_with_overlaps.end());
        }
        write_pdb_file(glycoprotein, cycle, "./outputs/summary", 0.0);
        //std::cout << "Updating list of sites with overlaps." << std::endl;
        sites_with_overlaps = DetermineSitesWithOverlap(glycosites, tolerance); // Moved glycans may clash with other glycans. Need to check.
        if (sites_with_overlaps.size() == 0)
        {
            std::cout << "Stopping with all overlaps resolved.\n";
            stop = true;
        }
    }
    write_pdb_file(glycoprotein, (cycle + 1), "./outputs/summary", 0.0);
}


// random number generator; allows full range rotation
double RandomAngle_360range()
{
    return (rand() % 360) + 1 - 180;
}

// random number generator; specify a maximum step size relative to a start point
double RandomAngle_PlusMinusX(double start_point, int max_step_size)
{
    return start_point + (rand() % (max_step_size * 2) + 1) - max_step_size;
}

double GetNewAngleScaledToDegreeOfOverlap(double current_angle, double overlap, int number_of_atoms)
{
    int max_step_size = 1 + std::round( 180 * ( overlap / number_of_atoms ) ); // Always allow at least 1 degrees of movement
    return RandomAngle_PlusMinusX(current_angle, max_step_size);
}

void write_pdb_file(Assembly &glycoprotein, int cycle, std::string summary_filename, double score)
{
    std::string pdb_filename = "outputs/pose_" + std::to_string(cycle) + ".pdb";
    PdbFileSpace::PdbFile *outputPdbFile = glycoprotein.BuildPdbFileStructureFromAssembly(-1,0);
    outputPdbFile->Write(pdb_filename);
    std::ofstream summary;   // write a file that describes the best conformations found
    summary.open(summary_filename, std::ios::out | std::ios::app);
    summary << score << "\t" << "pose_" << cycle << ".pdb\n";
    summary.close();
}

void PrintOverlaps(GlycosylationSiteVector &glycosites)
{
    for (GlycosylationSiteVector::iterator current_glycosite = glycosites.begin(); current_glycosite != glycosites.end(); ++current_glycosite)
    {
        current_glycosite->Print_bead_overlaps();
    }
}

void PrintOverlaps(GlycosylationSitePointerVector &glycosites)
{
    for (GlycosylationSitePointerVector::iterator it = glycosites.begin(); it != glycosites.end(); ++it)
    {
        GlycosylationSite *current_glycosite = (*it);
        current_glycosite->Print_bead_overlaps();
    }
}

void SetBestChi1Chi2(GlycosylationSitePointerVector &glycosites, Assembly &glycoprotein, std::string type)
{

    if (type.compare("total")==0)
    {
        for (GlycosylationSitePointerVector::iterator it = glycosites.begin(); it != glycosites.end(); ++it)
        {
            GlycosylationSite *current_glycosite = (*it);
            Overlap_record initial_settings(current_glycosite->GetOverlap(), current_glycosite->GetChi1Value(), current_glycosite->GetChi2Value());
         //   std::cout << current_glycosite->GetResidueNumber() << ": " << current_glycosite->GetBestOverlapRecord().GetOverlap() << std::endl;

            current_glycosite->Calculate_bead_overlaps();
            if (current_glycosite->GetOverlap() > initial_settings.GetOverlap() )
            {

            }
        }
    }
    if (type.compare("protein")==0)
    {
        for (GlycosylationSitePointerVector::iterator it = glycosites.begin(); it != glycosites.end(); ++it)
        {
            GlycosylationSite *current_glycosite = (*it);
            current_glycosite->SetChi1Value(current_glycosite->GetBestProteinOverlapRecord().GetChi1(), glycoprotein);
            current_glycosite->SetChi2Value(current_glycosite->GetBestProteinOverlapRecord().GetChi2(), glycoprotein);
        }
    }
}

GlycosylationSitePointerVector DetermineSitesWithOverlap(GlycosylationSiteVector &glycosites, double tolerance, std::string returning, std::string type)
{
    GlycosylationSitePointerVector sites_to_return;
    double overlap = 0.0;
    std::cout << "      Site        |  Total | Protein | Glycan " << std::endl;
    for (GlycosylationSiteVector::iterator current_glycosite = glycosites.begin(); current_glycosite != glycosites.end(); ++current_glycosite)
    {
        if(type.compare("total")==0)
        {
            overlap = current_glycosite->Calculate_bead_overlaps();
        }
        else if (type.compare("protein")==0)
        {
            overlap = current_glycosite->Calculate_protein_bead_overlaps();
        }
        else if (type.compare("glycan")==0)
        {
            overlap = current_glycosite->Calculate_other_glycan_bead_overlaps();
        }

        //Figure out which sites  to return (overlapping or not) but always print those with overlaps.
        if ( overlap > tolerance)
        {
            current_glycosite->Print_bead_overlaps();
            if (returning.compare("with")==0)
            {
                sites_to_return.push_back(&(*current_glycosite));
            }
        }
        else
        {
            if (returning.compare("with")!=0) // If site does not have overlaps and they want to return sites without overlap
            {
                sites_to_return.push_back(&(*current_glycosite));
            }
        }
    }
    return sites_to_return;
}


GlycosylationSitePointerVector DeleteSitesWithOverlaps(Assembly &glycoprotein, GlycosylationSiteVector &glycosites, double tolerance, std::string type)
{
    GlycosylationSitePointerVector sites_to_return;
    double overlap = 0.0;
    for (GlycosylationSiteVector::iterator current_glycosite = glycosites.begin(); current_glycosite != glycosites.end();)
    {
        //First calculate the site's overlap value
        if(type.compare("total")==0)
        {
            overlap = current_glycosite->Calculate_bead_overlaps();
        }
        else if (type.compare("protein")==0)
        {
            overlap = current_glycosite->Calculate_protein_bead_overlaps();
        }
        else if (type.compare("glycan")==0)
        {
            overlap = current_glycosite->Calculate_other_glycan_bead_overlaps();
        }
        // Delete site from list if overlap is greater than the tolerance value
        std::cout << "Site " << current_glycosite->GetResidueNumber() << ": " << overlap << " :";
        if ( overlap > tolerance)
        {
            std::cout << "Removed\n";
            ResidueVector glycan_residues = current_glycosite->GetAttachedGlycan()->GetResidues();
            for(ResidueVector::iterator it = glycan_residues.begin(); it != glycan_residues.end(); ++it)
            {
                glycoprotein.RemoveResidue(*it);
            }
            glycosites.erase(std::remove(glycosites.begin(), glycosites.end(), *current_glycosite), glycosites.end()); // Note need #include <algorithm>
        }
        else
        {
            std::cout << "Retained\n";
            sites_to_return.push_back(&(*current_glycosite));
            ++current_glycosite; // This will get you. Erase/Remove advances current_glycosite.
        }
    }
    return sites_to_return;
}


void Monte_Carlo_Torsions(GlycosylationSitePointerVector &sites, GlycosylationSiteVector &glycosites, Assembly &assembly, int max_cycles, double tolerance, std::string overlap_type)
{
    int cycle = 0;
    bool stop = false;
    while ( (cycle < max_cycles) && (stop == false) )
    {
        ++cycle;
        std::cout << "Cycle " << cycle << " of " << max_cycles << ".\n";
        RandomizeTorsions(sites, assembly);
        std::cout << "Updating list of sites with protein overlaps.\n";
        sites = DetermineSitesWithOverlap(glycosites, tolerance, "with", overlap_type);
        if (sites.size() == 0)
        {
            std::cout << "No more protein overlaps\n";
            stop = true;
        }
    }
    return;
}

void RandomizeTorsions(GlycosylationSitePointerVector &sites, Assembly &assembly)
{
    double new_dihedral_angle = 0.0;
    for(GlycosylationSitePointerVector::iterator it1 = sites.begin(); it1 != sites.end(); ++it1)
    {
        GlycosylationSite *current_glycosite = (*it1);
        new_dihedral_angle = GetNewAngleScaledToDegreeOfOverlap(current_glycosite->GetChi1Value(), current_glycosite->GetProteinOverlap(), current_glycosite->GetAttachedGlycan()->GetAllAtomsOfAssembly().size());
        current_glycosite->SetChi1Value(new_dihedral_angle, assembly);
        new_dihedral_angle = GetNewAngleScaledToDegreeOfOverlap(current_glycosite->GetChi2Value(), current_glycosite->GetProteinOverlap(), current_glycosite->GetAttachedGlycan()->GetAllAtomsOfAssembly().size());
        current_glycosite->SetChi2Value(new_dihedral_angle, assembly);
    }
    return;
}
