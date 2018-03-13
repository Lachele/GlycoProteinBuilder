#include "../includes/resolve_overlaps.h"
#include "../includes/glycosylationsite.h"


// genetic algorithm 

# include <cstdlib>
# include <iostream>
# include <iomanip>
# include <fstream>
# include <iomanip>
# include <cmath>
# include <ctime>
# include <cstring>
# include <ostream>

#include <string>
#include <vector> 
#include <stdio.h>
#include <stdlib.h>
#include <sstream> 
#include <math.h>
#include <ctime>
#include <cstring>
#include <time.h>


//#include "resolve_overlaps.h"
//#include "glycosylationsite.h"

using namespace std;

# define POPSIZE 100
# define MAXGENS 1000
# define NVARS 156
# define PXOVER 0.8
# define PMUTATION 0.15

struct genotype
{
  double gene[NVARS];
  double fitness;
  double upper[NVARS];
  double lower[NVARS];
  double rfitness;
  double cfitness;
};

struct genotype population[POPSIZE+1];
struct genotype newpopulation[POPSIZE+1]; 

time_t t;
time_t timer; 
time_t timezero=time(0);


int i4_uniform_ab ( int a, int b, int &seed )

//****************************************************************************80
//
//  Purpose:
//
//    I4_UNIFORM_AB returns a scaled pseudorandom I4 between A and B.
//
//  Discussion:
//
//    The pseudorandom number should be uniformly distributed
//    between A and B.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    02 October 2012
//
//  Author:
//
//    John Burkardt
//
//  Reference:
//
//    Paul Bratley, Bennett Fox, Linus Schrage,
//    A Guide to Simulation,
//    Second Edition,
//    Springer, 1987,
//    ISBN: 0387964673,
//    LC: QA76.9.C65.B73.
//
//    Bennett Fox,
//    Algorithm 647:
//    Implementation and Relative Efficiency of Quasirandom
//    Sequence Generators,
//    ACM Transactions on Mathematical Software,
//    Volume 12, Number 4, December 1986, pages 362-376.
//
//    Pierre L'Ecuyer,
//    Random Number Generation,
//    in Handbook of Simulation,
//    edited by Jerry Banks,
//    Wiley, 1998,
//    ISBN: 0471134031,
//    LC: T57.62.H37.
//
//    Peter Lewis, Allen Goodman, James Miller,
//    A Pseudo-Random Number Generator for the System/360,
//    IBM Systems Journal,
//    Volume 8, Number 2, 1969, pages 136-143.
//
//  Parameters:
//
//    Input, int A, B, the limits of the interval.
//
//    Input/output, int &SEED, the "seed" value, which should NOT be 0.
//    On output, SEED has been updated.
//
//    Output, int I4_UNIFORM, a number between A and B.
//
{
  int c;
  const int i4_huge = 2147483647;
  int k;
  float r;
  int value;

  if ( seed == 0 )
  {
    cout << "\n";
    cout << "I4_UNIFORM_AB - Fatal error!\n";
    cout << "  Input value of SEED = 0.\n";
    exit ( 1 );
  }
//
//  Guarantee A <= B.
//
  if ( b < a )
  {
    c = a;
    a = b;
    b = c;
  }

  k = seed / 127773;

  seed = 16807 * ( seed - k * 127773 ) - k * 2836;

  if ( seed < 0 )
  {
    seed = seed + i4_huge;
  }

  r = ( float ) ( seed ) * 4.656612875E-10;
//
//  Scale R to lie between A-0.5 and B+0.5.
//
  r = ( 1.0 - r ) * ( ( float ) a - 0.5 ) 
    +         r   * ( ( float ) b + 0.5 );
//
//  Use rounding to convert R to an integer between A and B.
//
  value = round ( r );
//
//  Guarantee A <= VALUE <= B.
//
  if ( value < a )
  {
    value = a;
  }
  if ( b < value )
  {
    value = b;
  }

  return value;
}
//****************************************************************************80

void initialize ( std::string filename, int &seed )

//****************************************************************************80
// 
//  Purpose:
//
//    INITIALIZE initializes the genes within the variables bounds. 
//
//  Discussion:
//
//    It also initializes (to zero) all fitness values for each
//    member of the population. It reads upper and lower bounds 
//    of each variable from the input file `gadata.txt'. It 
//    randomly generates values between these bounds for each 
//    gene of each genotype in the population. The format of 
//    the input file `gadata.txt' is 
//
//      var1_lower_bound var1_upper bound
//      var2_lower_bound var2_upper bound ...

//  Parameters:
//
//    Input, string FILENAME, the name of the input file.
//
//    Input/output, int &SEED, a seed for the random number generator.
//
{
  int i;
  ifstream input;
  int j;
  double lbound=0;
  double ubound=360;


/*
  input.open ( filename.c_str ( ) );

  if ( !input )
  {
    cout << "\n";
    cout << "INITIALIZE - Fatal error!\n";
    cout << "  Cannot open the input file!\n";
    exit ( 1 );
  }
// 
//  Initialize variables within the bounds 
//
  for ( i = 0; i < NVARS; i++ )
  {
//    input >> lbound >> ubound;
*/


    for ( j = 0; j < POPSIZE; j++ ){
        for (int i=0; i<NVARS; i++){

      population[j].fitness = 0;
      population[j].rfitness = 0;
      population[j].cfitness = 0;
      population[j].lower[i] = lbound;
      population[j].upper[i]= ubound;
      population[j].gene[i] =(rand() % 360) + 1 - 180; 

//r8_uniform_ab ( lbound, ubound, seed);
   
      }
    }

//  }

//  input.close ( );

  return;
}
//****************************************************************************80

void keep_the_best ( )

//****************************************************************************80
// 
//  Purpose:
//
//    KEEP_THE_BEST keeps track of the best member of the population. 
//
//  Discussion:
//
//    Note that the last entry in the array Population holds a 
//    copy of the best individual.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    29 December 2007
//
//  Author:
//
//    Original version by Dennis Cormier and Sita Raghavan.
//    This C++ version by John Burkardt.
//
//  Local parameters:
//
//    Local, int CUR_BEST, the index of the best individual.
//
{
  int cur_best;
  int mem;
  int i;

  cur_best = -100000000;

  for ( mem = 0; mem < POPSIZE; mem++ )
  {
    if ( population[mem].fitness >= cur_best)
    {
      cur_best = mem;
      population[POPSIZE].fitness = population[mem].fitness;
    }
  }
// 
//  Once the best member in the population is found, copy the genes.
//
  for ( i = 0; i < NVARS; i++ )
  {
    population[POPSIZE].gene[i] = population[cur_best].gene[i];
  
// cout << population[POPSIZE].gene[i] << endl;

}

  return;
}
//****************************************************************************80

void mutate ( int &seed )

//****************************************************************************80
// 
//  Purpose:
//
//    MUTATE performs a random uniform mutation. 
//
//  Discussion:
//
//    A variable selected for mutation is replaced by a random value 
//    between the lower and upper bounds of this variable.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    28 April 2014
//
//  Author:
//
//    Original version by Dennis Cormier and Sita Raghavan.
//    This C++ version by John Burkardt.
//
//  Parameters:
//
//    Input/output, int &SEED, a seed for the random number generator.
//
{
  const double a = 0.0;
  const double b = 1.0;
  int i;
  int j;
  double lbound;
  double ubound;
  double x;

  for ( i = 0; i < POPSIZE; i++ )
  {
    for ( j = 0; j < NVARS; j++ )
    {
      x = rand(); 

//r8_uniform_ab ( a, b, seed );
      if ( x < PMUTATION )
      {
        lbound = population[i].lower[j];
        ubound = population[i].upper[j];  
        population[i].gene[j] =  (rand() % 360) + 1 - 180;

//r8_uniform_ab ( lbound, ubound, seed );
      }
    }
  }

  return;
}
//****************************************************************************80

double r8_uniform_ab ( double a, double b, int &seed )

//****************************************************************************80
//
//  Purpose:
//
//    R8_UNIFORM_AB returns a scaled pseudorandom R8.
//
//  Discussion:
//
//    The pseudorandom number should be uniformly distributed
//    between A and B.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    09 April 2012
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, double A, B, the limits of the interval.
//
//    Input/output, int &SEED, the "seed" value, which should NOT be 0.
//    On output, SEED has been updated.
//
//    Output, double R8_UNIFORM_AB, a number strictly between A and B.
//
{
  int i4_huge = 2147483647;
  int k;
  double value;

  if ( seed == 0 )
  {
    cout << "\n";
    cout << "R8_UNIFORM_AB - Fatal error!\n";
    cout << "  Input value of SEED = 0.\n";
    exit ( 1 );
  }

  k = seed / 127773;

  seed = 16807 * ( seed - k * 127773 ) - k * 2836;

  if ( seed < 0 )
  {
    seed = seed + i4_huge;
  }

  value = ( double ) ( seed ) * 4.656612875E-10;

  value = a + ( b - a ) * value;

  return value;
}
//****************************************************************************80

void report ( int generation )

//****************************************************************************80
// 
//  Purpose:
//
//    REPORT reports progress of the simulation. 
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    29 December 2007
//
//  Author:
//
//    Original version by Dennis Cormier and Sita Raghavan.
//    This C++ version by John Burkardt.
//
//  Local parameters:
//
//    Local, double avg, the average population fitness.
//
//    Local, best_val, the best population fitness.
//
//    Local, double square_sum, square of sum for std calc.
//
//    Local, double stddev, standard deviation of population fitness.
//
//    Local, double sum, the total population fitness.
//
//    Local, double sum_square, sum of squares for std calc.
//
{
  double avg;
  double best_val;
  int i;
  double square_sum;
  double stddev;
  double sum;
  double sum_square;

  if ( generation == 0 )
  {
    cout << "\n";
    cout << "  Generation       Best            Average       Standard       time \n";
    cout << "  number           value           fitness       deviation      calculation \n";
    cout << "\n";
  }

  sum = 0.0;
  sum_square = 0.0;

  for ( i = 0; i < POPSIZE; i++ )
  {
    sum = sum + population[i].fitness;
    sum_square = sum_square + population[i].fitness * population[i].fitness;
  }

  avg = sum / ( double ) POPSIZE;
  square_sum = avg * avg * POPSIZE;
  stddev = sqrt ( ( sum_square - square_sum ) / ( POPSIZE - 1 ) );
  best_val = population[POPSIZE].fitness;

  t=time(0)-timezero;

  ofstream geneticoutput;

  geneticoutput.open("genetic_algorithm_output.txt",ios::out|ios::app);

  cout << "  " << setw(8) << generation 
       << "  " << setw(14) << best_val 
       << "  " << setw(14) << avg 
       << "  " << setw(14) << stddev 
       << "  " << t << "\n";

  geneticoutput << "  " << setw(8) << generation
       << "  " << setw(14) << best_val
       << "  " << setw(14) << avg
       << "  " << setw(14) << stddev
       << "  " << t << "\n";

  geneticoutput.close();


  return;
}
//****************************************************************************80

void selector ( int &seed )

//****************************************************************************80
// 
//  Purpose:
//
//    SELECTOR is the selection function.
//
//  Discussion:
//
//    Standard proportional selection for maximization problems incorporating 
//    the elitist model.  This makes sure that the best member always survives.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    28 April 2014
//
//  Author:
//
//    Original version by Dennis Cormier and Sita Raghavan.
//    This C++ version by John Burkardt.
//
//  Parameters:
//
//    Input/output, int &SEED, a seed for the random number generator.
//
{
  const double a = 0.0;
  const double b = 1.0;
  int i;
  int j;
  int mem;
  double p;
  double sum;
//
//  Find the total fitness of the population.
//
  sum = 0.0;
  for ( mem = 0; mem < POPSIZE; mem++ )
  {
    sum = sum + population[mem].fitness;
  }
//
//  Calculate the relative fitness of each member.
//
  for ( mem = 0; mem < POPSIZE; mem++ )
  {
    population[mem].rfitness = population[mem].fitness / sum;
  }
// 
//  Calculate the cumulative fitness.
//
  population[0].cfitness = population[0].rfitness;
  for ( mem = 1; mem < POPSIZE; mem++ )
  {
    population[mem].cfitness = population[mem-1].cfitness +       
      population[mem].rfitness;
  }
// 
//  Select survivors using cumulative fitness. 
//
  for ( i = 0; i < POPSIZE; i++ )
  { 
    p = r8_uniform_ab ( a, b, seed );
    if ( p < population[0].cfitness )
    {
      newpopulation[i] = population[0];      
    }
    else
    {
      for ( j = 0; j < POPSIZE; j++ )
      { 
        if ( population[j].cfitness <= p && p < population[j+1].cfitness )
        {
          newpopulation[i] = population[j+1];
        }
      }
    }
  }
// 
//  Overwrite the old population with the new one.
//
  for ( i = 0; i < POPSIZE; i++ )
  {
    population[i] = newpopulation[i]; 
  }

  return;     
}
//****************************************************************************80

void timestamp ( )

//****************************************************************************80
//
//  Purpose:
//
//    TIMESTAMP prints the current YMDHMS date as a time stamp.
//
//  Example:
//
//    May 31 2001 09:45:54 AM
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    04 October 2003
//
//  Author:
//
//    John Burkardt
//
{
# define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct tm *tm;
  size_t len;
  time_t now;

  now = time ( NULL );
  tm = localtime ( &now );

  len = strftime ( time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm );

  cout << time_buffer << "\n";

  return;
# undef TIME_SIZE
}
//****************************************************************************80

void Xover ( int one, int two, int &seed )

//****************************************************************************80
// 
//  Purpose:
//
//    XOVER performs crossover of the two selected parents. 
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    28 April 2014
//
//  Author:
//
//    Original version by Dennis Cormier and Sita Raghavan.
//    This C++ version by John Burkardt.
//
//  Local parameters:
//
//    Local, int point, the crossover point.
//
//  Parameters:
//
//    Input, int ONE, TWO, the indices of the two parents.
//
//    Input/output, int &SEED, a seed for the random number generator.
//
{
  int i;
  int point;
  double t;
// 
//  Select the crossover point.
//
  point = i4_uniform_ab ( 0, NVARS - 1, seed );
//
//  Swap genes in positions 0 through POINT-1.
//
  for ( i = 0; i < point; i++ )
  {
    t                       = population[one].gene[i];
    population[one].gene[i] = population[two].gene[i];
    population[two].gene[i] = t;
  }

  return;
}

typedef std::vector<GlycosylationSite*> GlycosylationSitePointerVector;

using namespace gmml;

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

void write_pdb_file(Assembly *glycoprotein, int cycle, std::string summary_filename, double score)
{
    std::string pdb_filename = "outputs/pose_" + std::to_string(cycle) + ".pdb";
    PdbFileSpace::PdbFile *outputPdbFile = glycoprotein->BuildPdbFileStructureFromAssembly(-1,0);
    outputPdbFile->Write(pdb_filename);
    std::ofstream summary;   // write a file that describes the best conformations found
    summary.open(summary_filename, std::ios::out | std::ios::app);
    summary << score << "\t" << "pose_" << cycle << ".pdb\n";
    summary.close();
}

void PrintOverlaps(GlycosylationSiteVector *glycosites)
{
    for (GlycosylationSiteVector::iterator current_glycosite = glycosites->begin(); current_glycosite != glycosites->end(); ++current_glycosite)
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

void SetBestProteinChi1Chi2(GlycosylationSitePointerVector &glycosites, Assembly *glycoprotein)
{
    for (GlycosylationSitePointerVector::iterator it = glycosites.begin(); it != glycosites.end(); ++it)
    {
        GlycosylationSite *current_glycosite = (*it);
        current_glycosite->SetChi1Value(current_glycosite->GetBestProteinOverlapRecord().GetChi1(), glycoprotein);
        current_glycosite->SetChi2Value(current_glycosite->GetBestProteinOverlapRecord().GetChi2(), glycoprotein);
    }
}

GlycosylationSitePointerVector DetermineSitesWithOverlap(GlycosylationSiteVector *glycosites, double tolerance, std::string type = "total")
{
    GlycosylationSitePointerVector sites_with_overlaps;
    double overlap = 0.0;
    std::cout << "      Site        |  Total | Protein | Glycan " << std::endl;
    for (GlycosylationSiteVector::iterator current_glycosite = glycosites->begin(); current_glycosite != glycosites->end(); ++current_glycosite)
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
        if ( overlap > tolerance)
        {
            current_glycosite->Print_bead_overlaps();
            sites_with_overlaps.push_back(&(*current_glycosite));
        }
    }
    return sites_with_overlaps;
}

/* New algorithm idea:
 * For each site that has overlap:
 1) Try to resolve all protein overlaps, ignoring glycan-glycan. Delete glycan from any site with unresolvable protein overlaps, report to user.
 2) Resolve glycan overlaps. Either
    a) Loop through and check every site each cycle, and move those with overlaps

    c) Create a tree structure for sites that overlap with each other.
        Could use assembly index/id of bead atom? Can work on individual trees and report nice info to users.


Note: Chi1 is 180,-60,60 +/- 30 degrees. Want a function that keeps the values within these states.
Note: Need to save best structure.
Note: Remember to delete the SUP atoms at some stage.
*/

void resolve_overlaps::monte_carlo(Assembly *glycoprotein, GlycosylationSiteVector *glycosites)
{
    /* Algorithm:
     *  Resolve all protein overlap first, reject sites that cannot be resolved. Record cycles to resolve?
     *  Calculate protein overlaps, for each site with overlaps
     *        Randomly change all chi1 and chi2 values
     *  Once finished;
     *  Calculate glycan overlaps, for each site with overlaps
     *        Change chi1 and chi2, scaled by degree of overlap
     *        Reject changes causing protein overlaps
     *
     */
    double tolerance = 0.1;
    double new_dihedral_angle = 0.0;
    int cycle = 0, max_cycles = 5;

    while (cycle < max_cycles)
    {
        ++cycle;
        for(GlycosylationSiteVector::iterator current_glycosite = glycosites->begin(); current_glycosite != glycosites->end(); ++current_glycosite)
        {
            current_glycosite->Calculate_protein_bead_overlaps();
            new_dihedral_angle = GetNewAngleScaledToDegreeOfOverlap(current_glycosite->GetChi1Value(), current_glycosite->GetProteinOverlap(), current_glycosite->GetAttachedGlycan()->GetAllAtomsOfAssembly().size());
            current_glycosite->SetChi1Value(new_dihedral_angle, glycoprotein);
            new_dihedral_angle = GetNewAngleScaledToDegreeOfOverlap(current_glycosite->GetChi2Value(), current_glycosite->GetProteinOverlap(), current_glycosite->GetAttachedGlycan()->GetAllAtomsOfAssembly().size());
            current_glycosite->SetChi2Value(new_dihedral_angle, glycoprotein);
            current_glycosite->Calculate_protein_bead_overlaps();

        }
    }

//    GlycosylationSitePointerVector sites_with_protein_overlaps = DetermineSitesWithOverlap(glycosites, tolerance, "protein");
//    bool stop = false;

//    while ( (cycle < max_cycles) && (stop == false) )
//    {
//        ++cycle;
//        std::cout << "Cycle " << cycle << " of " << max_cycles << std::endl;
//        for(GlycosylationSitePointerVector::iterator it1 = sites_with_protein_overlaps.begin(); it1 != sites_with_protein_overlaps.end(); ++it1)
//        {
//            GlycosylationSite *current_glycosite = (*it1);
//            current_glycosite->Calculate_protein_bead_overlaps();

//            new_dihedral_angle = GetNewAngleScaledToDegreeOfOverlap(current_glycosite->GetChi1Value(), current_glycosite->GetProteinOverlap(), current_glycosite->GetAttachedGlycan()->GetAllAtomsOfAssembly().size());
//           // std::cout << "Changing chi1 from " << current_glycosite->GetChi1Value() << " to " << new_dihedral_angle << " based on protein overlap of " << current_glycosite->GetProteinOverlap() << std::endl;
//            current_glycosite->SetChi1Value(new_dihedral_angle, glycoprotein);
//          //  std::cout << "New value: " << current_glycosite->GetChi1Value() << std::endl;
//            new_dihedral_angle = GetNewAngleScaledToDegreeOfOverlap(current_glycosite->GetChi2Value(), current_glycosite->GetProteinOverlap(), current_glycosite->GetAttachedGlycan()->GetAllAtomsOfAssembly().size());
//         //   std::cout << "Changing chi2 from " << current_glycosite->GetChi2Value() << " to " << new_dihedral_angle << " based on protein overlap of " << current_glycosite->GetProteinOverlap() << std::endl;
//            current_glycosite->SetChi2Value(new_dihedral_angle, glycoprotein);
//      //      std::cout << "New value: " << current_glycosite->GetChi2Value() << std::endl;

////            current_glycosite->Calculate_protein_bead_overlaps();
////            current_glycosite->Print_bead_overlaps();
////            current_glycosite->SetChi1Value(-73.4055522836, glycoprotein);
////            current_glycosite->SetChi2Value(-132.8354561298, glycoprotein);
////            current_glycosite->Calculate_protein_bead_overlaps();
////            current_glycosite->Print_bead_overlaps();
////            current_glycosite->SetChi1Value(-73.4055522836, glycoprotein);
////            current_glycosite->SetChi2Value(-132.8354561298, glycoprotein);
////            current_glycosite->Calculate_protein_bead_overlaps();
////            current_glycosite->Print_bead_overlaps();
////            current_glycosite->SetChi1Value(0.00, glycoprotein);
////            current_glycosite->SetChi2Value(0.00, glycoprotein);
////            current_glycosite->Calculate_protein_bead_overlaps();
////            current_glycosite->Print_bead_overlaps();
////            current_glycosite->SetChi1Value(-73.4055522836, glycoprotein);
////            current_glycosite->SetChi2Value(-132.8354561298, glycoprotein);
//               current_glycosite->Calculate_protein_bead_overlaps();
//         //   current_glycosite->Print_bead_overlaps();

//            //double percent_overlap = ((current_glycosite->GetTotalOverlap() / (current_glycosite->GetAttachedGlycan()->GetAllAtomsOfAssembly().size()) ) + 0.01);
//            //  new_dihedral_value = RandomAngle_PlusMinusX(current_glycosite->GetChi1Value(), (180 * percent_overlap) ); // scaled to degree of overlap
//            //        sites_with_overlaps.erase(std::remove(sites_with_overlaps.begin(), sites_with_overlaps.end(), *it1), sites_with_overlaps.end());
//        }
//        //std::cout << "Updating list of sites with overlaps." << std::endl;
//       // sites_with_protein_overlaps = DetermineSitesWithOverlap(glycosites, tolerance, "protein"); // Moved glycans may clash with other glycans. Need to check.
//        if (sites_with_protein_overlaps.size() == 0)
//        {
//            std::cout << "No more protein overlaps" << std::endl;
//            stop = true;
//        }
//    }
    //PrintOverlaps(glycosites);
  //  write_pdb_file(glycoprotein, 1, "./outputs/summary", 0.0);
    // Remove sites that cannot be resolved.
 //   std::cout << "Setting best chi1 and chi2 found so far" << std::endl;
  //  SetBestProteinChi1Chi2(sites_with_protein_overlaps, glycoprotein);
 //   std::cout << "Could not resolve protein overlaps for these sites: " << std::endl;
   // sites_with_protein_overlaps = DetermineSitesWithOverlap(glycosites, tolerance, "protein");
    //PrintOverlaps(sites_with_protein_overlaps);
}

void resolve_overlaps::dumb_monte_carlo(Assembly *glycoprotein, GlycosylationSiteVector *glycosites)
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

//
//  Each GENOTYPE is a member of the population, with
//  gene: a string of variables,
//  fitness: the fitness
//  upper: the variable upper bounds,
//  lower: the variable lower bounds,
//  rfitness: the relative fitness,
//  cfitness: the cumulative fitness.
//

void resolve_overlaps::example_for_Gordon(Assembly *glycoprotein, GlycosylationSiteVector *glycosites)
{
    double site_total_overlap = 0.0, site_glycan_overlap = 0.0, site_protein_overlap = 0.0, new_dihedral_value = 0.0, total_system_overlap = 0.0;
    std::cout << "      Site        |  Total | Protein | Glycan " << std::endl;

   /*
    for (GlycosylationSiteVector::iterator current_glycosite = glycosites->begin(); current_glycosite != glycosites->end(); ++current_glycosite)
    {
        site_total_overlap = current_glycosite->Calculate_bead_overlaps(); // Must repeat after rotating chi1, chi2.
        site_glycan_overlap = current_glycosite->GetGlycanOverlap(); // If you wish to have this level of detail
        site_protein_overlap = current_glycosite->GetProteinOverlap(); // If you wish to have this level of detail
        current_glycosite->Print_bead_overlaps();
        new_dihedral_value = RandomAngle_PlusMinusX(current_glycosite->GetChi1Value(), 6);
        //std::cout << "Changing chi1 from " << current_glycosite->GetChi1Value() << " to " << new_dihedral_value << std::endl;
        current_glycosite->SetChi1Value(new_dihedral_value, glycoprotein);
        new_dihedral_value = RandomAngle_PlusMinusX(current_glycosite->GetChi2Value(), 6);
        current_glycosite->SetChi2Value(new_dihedral_value, glycoprotein);
        current_glycosite->Calculate_and_print_bead_overlaps();
        total_system_overlap += site_total_overlap;
    }
   */

// main taken away

// int main ( )

// this part goes in Oliver's function

//
//  Parameters:
//
//    MAXGENS is the maximum number of generations.
//
//    NVARS is the number of problem variables.
//
//    PMUTATION is the probability of mutation.
//
//    POPSIZE is the population size. 
//
//    PXOVER is the probability of crossover.                          
//
//{
  std::string filename = "simple_ga_input.txt";
  int generation;
  int i;
  int seed;

  timestamp ( );
//  cout << "\n";
//  cout << "SIMPLE_GA:\n";
//  cout << "  C++ version\n";
//  cout << "  A simple example of a genetic algorithm.\n";

  if ( NVARS < 2 )
  {
    cout << "\n";
    cout << "  The crossover modification will not be available,\n";
    cout << "  since it requires 2 <= NVARS.\n";
  }

  seed = 123456789;

  initialize ( filename, seed );

  evaluate (glycoprotein, glycosites);

  keep_the_best ( );

  for ( generation = 0; generation < MAXGENS; generation++ )
  {
    selector ( seed );
    crossover ( seed );
    mutate ( seed );
    report ( generation );
    evaluate (glycoprotein, glycosites);
    elitist ( );
  }


  ofstream geneticoutput;

  geneticoutput.open("genetic_algorithm_output.txt",ios::out|ios::app);


  cout << "\n";
  cout << "  Best member after " << MAXGENS << " generations:\n";
  cout << "\n";

  geneticoutput << endl;  

  geneticoutput << "  Best member after " << MAXGENS << " generations:\n";



  for ( i = 0; i < NVARS; i++ )
  {
    cout << "  var(" << i << ") = " << population[POPSIZE-1].gene[i] << "\n";

    geneticoutput <<  "  var(" << i << ") = " << population[POPSIZE-1].gene[i] << "\n";

  }

  cout << "\n";
  cout << "  Best fitness = " << population[POPSIZE-1].fitness << "\n";

  geneticoutput << "  Best fitness = " << population[POPSIZE-1].fitness << "\n";



//
//  Terminate.
//
  cout << "\n";
  cout << "SIMPLE_GA:\n";
  cout << "  Normal end of execution.\n";
  cout << "\n";
  timestamp ( );

  geneticoutput << "\n  Normal end of execution.\n";

  geneticoutput.close();


//****************************************************************************80

    write_pdb_file(glycoprotein, 1, "./outputs/summary", total_system_overlap);
}


void crossover ( int &seed )

//****************************************************************************80
// 
//  Purpose:
//
//    CROSSOVER selects two parents for the single point crossover.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    28 April 2014
//
//  Author:
//
//    Original version by Dennis Cormier and Sita Raghavan.
//    This C++ version by John Burkardt.
//
//  Local parameters:
//
//    Local, int FIRST, is a count of the number of members chosen.
//
//  Parameters:
//
//    Input/output, int &SEED, a seed for the random number generator.
//
{
  const double a = 0.0;
  const double b = 1.0;
  int mem;
  int one;
  int first = 0;
  double x;

  for ( mem = 0; mem < POPSIZE; ++mem )
  {
    x = r8_uniform_ab ( a, b, seed );

    if ( x < PXOVER )
    {
      ++first;

      if ( first % 2 == 0 )
      {
        Xover ( one, mem, seed );
      }
      else
      {
        one = mem;
      }

    }
  }
  return;
}
//****************************************************************************80

void elitist ( )

//****************************************************************************80
// 
//  Purpose:
//
//    ELITIST stores the best member of the previous generation.
//
//  Discussion:
//
//    The best member of the previous generation is stored as 
//    the last in the array. If the best member of the current 
//    generation is worse then the best member of the previous 
//    generation, the latter one would replace the worst member 
//    of the current population.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    29 December 2007
//
//  Author:
//
//    Original version by Dennis Cormier and Sita Raghavan.
//    This C++ version by John Burkardt.
//
//  Local parameters:
//
//    Local, double BEST, the best fitness value.
//
//    Local, double WORST, the worst fitness value.
//
{
  int i;
  double best;
  int best_mem;
  double worst;
  int worst_mem;

  best = population[0].fitness;
  worst = population[0].fitness;

  for ( i = 0; i < POPSIZE - 1; ++i )
  {
    if ( population[i+1].fitness < population[i].fitness )
    {

      if ( best <= population[i].fitness )
      {
        best = population[i].fitness;
        best_mem = i;
      }

      if ( population[i+1].fitness <= worst )
      {
        worst = population[i+1].fitness;
        worst_mem = i + 1;
      }

    }
    else
    {

      if ( population[i].fitness <= worst )
      {
        worst = population[i].fitness;
        worst_mem = i;
      }

      if ( best <= population[i+1].fitness )
      {
        best = population[i+1].fitness;
        best_mem = i + 1;
      }

    }

  }
// 
//  If the best individual from the new population is better than 
//  the best individual from the previous population, then 
//  copy the best from the new population; else replace the 
//  worst individual from the current population with the 
//  best one from the previous generation                     
//
  if ( population[POPSIZE].fitness <= best )
  {
    for ( i = 0; i < NVARS; i++ )
    {
      population[POPSIZE].gene[i] = population[best_mem].gene[i];
    }
    population[POPSIZE].fitness = population[best_mem].fitness;
  }
  else
  {
    for ( i = 0; i < NVARS; i++ )
    {
      population[worst_mem].gene[i] = population[POPSIZE].gene[i];
    }
    population[worst_mem].fitness = population[POPSIZE].fitness;
  } 

  return;
}
//****************************************************************************80



void evaluate (Assembly *glycoprotein, GlycosylationSiteVector *glycosites)

//
//    EVALUATE implements the user-defined valuation function
//
//    Uses Oliver's code.
//

{
	
	double site_total_overlap = 0.0, site_glycan_overlap = 0.0, site_protein_overlap = 0.0, new_dihedral_value = 0.0, total_system_overlap = 0.0;

//    std::cout << "      Site        | Total | Protein | Glycan " << std::endl;

	int total=0;
	int member=0;
	
// for all members in the population	
	
	
  for ( member = 0; member < POPSIZE; member++ )
  
  {
	
	// combine Oliver code with c++ 
	
	total=0;
	
    for (GlycosylationSiteVector::iterator current_glycosite = glycosites->begin(); current_glycosite != glycosites->end(); ++current_glycosite)

    {
	
// NVARS = number of glycosites	
	
        current_glycosite->SetChi1Value(population[member].gene[total],glycoprotein);
		
        current_glycosite->SetChi2Value(population[member].gene[total+1],glycoprotein);
		 
		total=total+2;
		 
        site_total_overlap = current_glycosite->Calculate_bead_overlaps(); // Must repeat after rotating chi1, chi2.

//        site_glycan_overlap = current_glycosite->GetGlycanOverlap(); // If you wish to have this level of detail

//        site_protein_overlap = current_glycosite->GetProteinOverlap(); // If you wish to have this level of detail

//        current_glycosite->Print_bead_overlaps();

//        new_dihedral_value = RandomAngle_PlusMinusX(current_glycosite->GetChi1Value(), 6);

//        std::cout << "Changing chi1 from " << current_glycosite->GetChi1Value() << " to " << new_dihedral_value << std::endl;

//        current_glycosite->SetChi1Value(new_dihedral_value, glycoprotein);

//        new_dihedral_value = RandomAngle_PlusMinusX(current_glycosite->GetChi2Value(), 6);

//        current_glycosite->SetChi2Value(new_dihedral_value, glycoprotein);

//        current_glycosite->Calculate_and_print_bead_overlaps();

        total_system_overlap += site_total_overlap;

    }
	
	population[member].fitness=-total_system_overlap;
	
   }

	return;
}

	
//  int member;
//  int i;
//  double x[NVARS+1];

//  for ( member = 0; member < POPSIZE; member++ )
//  {
//    for ( i = 0; i < NVARS; i++ )
//    {
//      x[i+1] = population[member].gene[i];
//    } 
//    population[member].fitness = ( x[1] * x[1] ) - ( x[1] * x[2] ) + x[3];
//  }
//  return;
//}
//****************************************************************************
