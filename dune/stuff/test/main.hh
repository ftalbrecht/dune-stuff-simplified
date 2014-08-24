// This file is part of the dune-stuff project:
//   https://github.com/wwu-numerik/dune-stuff
// Copyright holders: Rene Milk, Felix Schindler
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

#ifndef DUNE_STUFF_TEST_MAIN_HH
#define DUNE_STUFF_TEST_MAIN_HH

#include "config.h"

#include <string>
#include <vector>
#include <map>
#include <random>
#include <fstream>
#include <sys/time.h>

#include <dune/stuff/common/disable_warnings.hh>
# include <dune/stuff/test/gtest/gtest.h>

# include <dune/common/float_cmp.hh>
# include <dune/common/fvector.hh>
# include <dune/common/fmatrix.hh>
# include <dune/common/tuples.hh>
# include <dune/common/tupleutility.hh>
# include <dune/common/parallel/mpihelper.hh>

# if HAVE_DUNE_FEM
#   include <dune/fem/misc/mpimanager.hh>
# endif
#include <dune/stuff/common/reenable_warnings.hh>

#include <dune/stuff/aliases.hh>
#include <dune/stuff/common/configuration.hh>
#include <dune/stuff/common/logging.hh>
#include <dune/stuff/common/convergence-study.hh>


static void check_for_success(const Dune::Stuff::Common::ConvergenceStudy& study,
                              const std::map< std::string, std::vector< double > >& errors_map)
{
  for (const auto& norm : study.used_norms()) {
    const auto expected_results = study.expected_results(norm);
    const auto errors_search = errors_map.find(norm);
    EXPECT_NE(errors_search, errors_map.end())
        << "          norm = " << norm;
    const auto& errors = errors_search->second;
    EXPECT_LE(errors.size(), expected_results.size())
        << "          norm = " << norm;
    for (size_t ii = 0; ii < errors.size(); ++ii)
      EXPECT_LE(errors[ii], expected_results[ii])
          << "          norm = " << norm << ", level = " << ii;
  }
} // ... check_for_success(...)


class
  DUNE_DEPRECATED_MSG("Use the expectation macros of the gtest test suite (20.08.2014)!")
      errors_are_not_as_expected
  : public Dune::Exception
{};


std::vector< double >
  DUNE_DEPRECATED_MSG("Use the expectation macros of the gtest test suite (20.08.2014)!")
                      truncate_vector(const std::vector< double >& in, const size_t size)
{
  assert(size <= in.size());
  if (size == in.size())
    return in;
  else {
    std::vector< double > ret(size);
    for (size_t ii = 0; ii < size; ++ii)
      ret[ii] = in[ii];
    return ret;
  }
} // ... truncate_vector(...)


template< template< class > class Test >
struct TestRunner
{
  struct Visitor
  {
    template< class T >
    void visit(const T&)
    {
      Test< T >().run();
    }
  };

  template< class Tuple >
  static void run()
  {
    Tuple t;
    Dune::ForEachValue< Tuple > fe(t);
    Visitor v;
    fe.apply(v);
  }
}; // struct TestRunner


template< int i >
struct Int
{
  static const int value = i;
};


//! where sleep only counts toward wall time, this wastes actual cpu time
void busywait(const int ms)
{
  // "round" up to next full 10 ms to align with native timer res
  const int milliseconds = (ms/10)*10 + 10;
  timeval start, end;
  gettimeofday(&start, NULL);
  do {
   gettimeofday(&end, NULL);
  } while (((end.tv_sec - start.tv_sec)*1e6) + ((end.tv_usec - start.tv_usec)) < milliseconds*1000);
}


typedef Dune::tuple< double
                   , float
//                   , Dune::bigunsignedint
                   ,int
                   , unsigned int
                   , unsigned long
                   , long long
                   , char
                     > BasicTypes;


int main(int argc, char** argv)
{
#ifdef DUNE_STUFF_TEST_MAIN_CATCH_EXCEPTIONS
  try {
#endif

    testing::InitGoogleTest(&argc, argv);
    DSC_CONFIG.read_options(argc, argv);
#if HAVE_DUNE_FEM
    Dune::Fem::MPIManager::initialize(argc, argv);
#else
    Dune::MPIHelper::instance(argc, argv);
#endif

    DSC::Logger().create(
#ifdef DUNE_STUFF_TEST_MAIN_ENABLE_INFO_LOGGING
                         DSC::LOG_CONSOLE | DSC::LOG_INFO | DSC::LOG_ERROR
#else
                         DSC::LOG_CONSOLE | DSC::LOG_ERROR
#endif
                                                                          , "", "", "");

    return RUN_ALL_TESTS();

#ifdef DUNE_STUFF_TEST_MAIN_CATCH_EXCEPTIONS
  } catch (Dune::Exception& e) {
    std::cerr << "\nDune reported error: " << e.what() << std::endl;
    std::abort();
  } catch (std::exception& e) {
    std::cerr << "\n" << e.what() << std::endl;
    std::abort();
  } catch (...) {
    std::cerr << "Unknown exception thrown!" << std::endl;
    std::abort();
  } // try
#endif // DUNE_STUFF_TEST_MAIN_CATCH_EXCEPTIONS
} // ... main(...)

#endif // DUNE_STUFF_TEST_MAIN_HH
