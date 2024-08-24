/**
 * @file tests/distribution_test.cpp
 * @author Ryan Curtin
 * @author Yannis Mentekidis
 * @author Rohan Raj
 *
 * Tests for the classes:
 *  * DiscreteDistribution
 *  * GaussianDistribution
 *  * GammaDistribution
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#include <mlpack/core.hpp>

#include "catch.hpp"
#include "serialization.hpp"
#include "test_catch_tools.hpp"

using namespace mlpack;

/*********************************/
/** Discrete Distribution Tests **/
/*********************************/

/**
 * Make sure we initialize correctly.
 */
TEMPLATE_TEST_CASE("DiscreteDistributionConstructorTest", "[DistributionTest]",
    (std::pair<double, double>),
    (std::pair<double, size_t>),
    (std::pair<float, float>),
    (std::pair<float, size_t>),
    (std::pair<float, unsigned short>))
{
  typedef typename TestType::first_type ElemType;
  typedef typename TestType::second_type ObsElemType;
  typedef typename arma::Mat<ElemType> MatType;
  typedef typename arma::Mat<ObsElemType> ObsMatType;

  DiscreteDistribution<MatType, ObsMatType> d(5);

  REQUIRE(d.Probabilities().n_elem == 5);
  REQUIRE(d.Probability("0") == Approx(0.2).epsilon(1e-7));
  REQUIRE(d.Probability("1") == Approx(0.2).epsilon(1e-7));
  REQUIRE(d.Probability("2") == Approx(0.2).epsilon(1e-7));
  REQUIRE(d.Probability("3") == Approx(0.2).epsilon(1e-7));
  REQUIRE(d.Probability("4") == Approx(0.2).epsilon(1e-7));
}

/**
 * Make sure we get the probabilities of observations right.
 */
TEMPLATE_TEST_CASE("DiscreteDistributionProbabilityTest", "[DistributionTest]",
    (std::pair<double, double>),
    (std::pair<double, size_t>),
    (std::pair<float, float>),
    (std::pair<float, size_t>),
    (std::pair<float, unsigned short>))
{
  typedef typename TestType::first_type ElemType;
  typedef typename TestType::second_type ObsElemType;
  typedef typename arma::Mat<ElemType> MatType;
  typedef typename arma::Mat<ObsElemType> ObsMatType;

  DiscreteDistribution<MatType, ObsMatType> d(5);

  d.Probabilities() = "0.2 0.4 0.1 0.1 0.2";

  REQUIRE(d.Probability("0") == Approx(0.2).epsilon(1e-7));
  REQUIRE(d.Probability("1") == Approx(0.4).epsilon(1e-7));
  REQUIRE(d.Probability("2") == Approx(0.1).epsilon(1e-7));
  REQUIRE(d.Probability("3") == Approx(0.1).epsilon(1e-7));
  REQUIRE(d.Probability("4") == Approx(0.2).epsilon(1e-7));
}

/**
 * Make sure we get random observations correct.
 */
TEMPLATE_TEST_CASE("DiscreteDistributionRandomTest", "[DistributionTest]",
    (std::pair<double, double>),
    (std::pair<double, size_t>),
    (std::pair<float, float>),
    (std::pair<float, size_t>),
    (std::pair<float, unsigned short>))
{
  typedef typename TestType::first_type ElemType;
  typedef typename TestType::second_type ObsElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;
  typedef typename arma::Mat<ObsElemType> ObsMatType;

  DiscreteDistribution<MatType, ObsMatType> d(arma::Col<size_t>("3"));

  d.Probabilities() = "0.3 0.6 0.1";

  VecType actualProb(3);

  actualProb.zeros();

  for (size_t i = 0; i < 50000; ++i)
    actualProb((size_t) (d.Random()[0] + 0.5))++;

  // Normalize.
  actualProb /= accu(actualProb);

  // 8% tolerance, because this can be a noisy process.
  REQUIRE(actualProb(0) == Approx(0.3).epsilon(0.08));
  REQUIRE(actualProb(1) == Approx(0.6).epsilon(0.08));
  REQUIRE(actualProb(2) == Approx(0.1).epsilon(0.08));
}

/**
 * Make sure we can estimate from observations correctly.
 */
TEMPLATE_TEST_CASE("DiscreteDistributionTrainTest", "[DistributionTest]",
    (std::pair<double, double>),
    (std::pair<double, size_t>),
    (std::pair<float, float>),
    (std::pair<float, size_t>),
    (std::pair<float, unsigned short>))
{
  typedef typename TestType::first_type ElemType;
  typedef typename TestType::second_type ObsElemType;
  typedef typename arma::Mat<ElemType> MatType;
  typedef typename arma::Mat<ObsElemType> ObsMatType;

  DiscreteDistribution<MatType, ObsMatType> d(4);

  ObsMatType obs("0 0 1 1 2 2 2 3");

  d.Train(obs);

  REQUIRE(d.Probability("0") == Approx(0.25).epsilon(1e-7));
  REQUIRE(d.Probability("1") == Approx(0.25).epsilon(1e-7));
  REQUIRE(d.Probability("2") == Approx(0.375).epsilon(1e-7));
  REQUIRE(d.Probability("3") == Approx(0.125).epsilon(1e-7));
}

/**
 * Estimate from observations with probabilities.
 */
TEMPLATE_TEST_CASE("DiscreteDistributionTrainProbTest", "[DistributionTest]",
    (std::pair<double, double>),
    (std::pair<double, size_t>),
    (std::pair<float, float>),
    (std::pair<float, size_t>),
    (std::pair<float, unsigned short>))
{
  typedef typename TestType::first_type ElemType;
  typedef typename TestType::second_type ObsElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;
  typedef typename arma::Mat<ObsElemType> ObsMatType;

  DiscreteDistribution<MatType, ObsMatType> d(3);

  ObsMatType obs("0 0 1 2");
  VecType prob("0.25 0.25 0.5 1.0");

  d.Train(obs, prob);

  REQUIRE(d.Probability("0") == Approx(0.25).epsilon(1e-7));
  REQUIRE(d.Probability("1") == Approx(0.25).epsilon(1e-7));
  REQUIRE(d.Probability("2") == Approx(0.5).epsilon(1e-7));
}

/**
 * Achieve multidimensional probability distribution.
 */
TEMPLATE_TEST_CASE("MultiDiscreteDistributionTrainProbTest",
    "[DistributionTest]",
    (std::pair<double, double>),
    (std::pair<double, size_t>),
    (std::pair<float, float>),
    (std::pair<float, size_t>),
    (std::pair<float, unsigned short>))
{
  typedef typename TestType::first_type ElemType;
  typedef typename TestType::second_type ObsElemType;
  typedef typename arma::Mat<ElemType> MatType;
  typedef typename arma::Mat<ObsElemType> ObsMatType;

  DiscreteDistribution<MatType, ObsMatType> d("10 10 10");

  ObsMatType obs("0 1 1 1 2 2 2 2 2 2;"
                 "0 0 0 1 1 1 2 2 2 2;"
                 "0 0 0 1 1 2 2 2 2 2;");

  d.Train(obs);
  REQUIRE(d.Probability("0 0 0") == Approx(0.009).epsilon(1e-7));
  REQUIRE(d.Probability("0 1 2") == Approx(0.015).epsilon(1e-7));
  REQUIRE(d.Probability("2 1 0") == Approx(0.054).epsilon(1e-7));
}

/**
 * Make sure we initialize multidimensional probability distribution
 * correctly.
 */
TEMPLATE_TEST_CASE("MultiDiscreteDistributionConstructorTest",
    "[DistributionTest]",
    (std::pair<double, double>),
    (std::pair<double, size_t>),
    (std::pair<float, float>),
    (std::pair<float, size_t>),
    (std::pair<float, unsigned short>))
{
  typedef typename TestType::first_type ElemType;
  typedef typename TestType::second_type ObsElemType;
  typedef typename arma::Mat<ElemType> MatType;
  typedef typename arma::Mat<ObsElemType> ObsMatType;

  DiscreteDistribution<MatType, ObsMatType> d("4 4 4 4");

  REQUIRE(d.Probabilities(0).size() == 4);
  REQUIRE(d.Dimensionality() == 4);
  REQUIRE(d.Probability("0 0 0 0") == Approx(0.00390625).epsilon(1e-7));
  REQUIRE(d.Probability("0 1 2 3") == Approx(0.00390625).epsilon(1e-7));
}

/**
 * Achieve multidimensional probability distribution.
 */
TEMPLATE_TEST_CASE("MultiDiscreteDistributionTrainTest", "[DistributionTest]",
    (std::pair<double, double>),
    (std::pair<double, size_t>),
    (std::pair<float, float>),
    (std::pair<float, size_t>),
    (std::pair<float, unsigned short>))
{
  typedef typename TestType::first_type ElemType;
  typedef typename TestType::second_type ObsElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;
  typedef typename arma::Mat<ObsElemType> ObsMatType;

  std::vector<VecType> pro;
  pro.push_back(VecType("0.1, 0.3, 0.6"));
  pro.push_back(VecType("0.3, 0.3, 0.3"));
  pro.push_back(VecType("0.25, 0.25, 0.5"));

  DiscreteDistribution<MatType, ObsMatType> d(pro);

  REQUIRE(d.Probability("0 0 0") == Approx(0.0083333).epsilon(1e-5));
  REQUIRE(d.Probability("0 1 2") == Approx(0.0166666).epsilon(1e-5));
  REQUIRE(d.Probability("2 1 0") == Approx(0.05).epsilon(1e-7));
}

/**
 * Estimate multidimensional probability distribution from observations with
 * probabilities.
 */
TEMPLATE_TEST_CASE("MultiDiscreteDistributionTrainProTest",
    "[DistributionTest]",
    (std::pair<double, double>),
    (std::pair<double, size_t>),
    (std::pair<float, float>),
    (std::pair<float, size_t>),
    (std::pair<float, unsigned short>))
{
  typedef typename TestType::first_type ElemType;
  typedef typename TestType::second_type ObsElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;
  typedef typename arma::Mat<ObsElemType> ObsMatType;

  DiscreteDistribution<MatType, ObsMatType> d("5 5 5");

  ObsMatType obs("0 0 1 1 2;"
                 "0 1 1 2 2;"
                 "0 1 1 2 2");

  VecType prob("0.25 0.25 0.25 0.25 1");

  d.Train(obs, prob);

  REQUIRE(d.Probability("0 0 0") == Approx(0.00390625).epsilon(1e-7));
  REQUIRE(d.Probability("1 0 1") == Approx(0.0078125).epsilon(1e-7));
  REQUIRE(d.Probability("2 1 0") == Approx(0.015625).epsilon(1e-7));
}

/**
 * Test the LogProbability() function, for multiple points in the multivariate
 * Discrete case.
 */
TEMPLATE_TEST_CASE("DiscreteLogProbabilityTest", "[DistributionTest]",
    (std::pair<double, double>),
    (std::pair<double, size_t>),
    (std::pair<float, float>),
    (std::pair<float, size_t>),
    (std::pair<float, unsigned short>))
{
  typedef typename TestType::first_type ElemType;
  typedef typename TestType::second_type ObsElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;
  typedef typename arma::Mat<ObsElemType> ObsMatType;

  // Same case as before.
  DiscreteDistribution<MatType, ObsMatType> d("5 5");

  ObsMatType obs("0 2;"
                 "1 2;");

  VecType logProb;

  d.LogProbability(obs, logProb);

  REQUIRE(logProb.n_elem == 2);

  REQUIRE(logProb(0) == Approx(-3.2188758248682).epsilon(1e-5));
  REQUIRE(logProb(1) == Approx(-3.2188758248682).epsilon(1e-5));
}

/**
 * Test the Probability() function, for multiple points in the multivariate
 * Discrete case.
 */
TEMPLATE_TEST_CASE("DiscreteProbabilityTest", "[DistributionTest]",
    (std::pair<double, double>),
    (std::pair<double, size_t>),
    (std::pair<float, float>),
    (std::pair<float, size_t>),
    (std::pair<float, unsigned short>))
{
  typedef typename TestType::first_type ElemType;
  typedef typename TestType::second_type ObsElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;
  typedef typename arma::Mat<ObsElemType> ObsMatType;

  // Same case as before.
  DiscreteDistribution<MatType, ObsMatType> d("5 5");

  ObsMatType obs("0 2;"
                 "1 2;");

  VecType prob;

  d.Probability(obs, prob);

  REQUIRE(prob.n_elem == 2);

  REQUIRE(prob(0) == Approx(0.0400000000000).epsilon(1e-5));
  REQUIRE(prob(1) == Approx(0.0400000000000).epsilon(1e-5));
}

/*********************************/
/** Gaussian Distribution Tests **/
/*********************************/

/**
 * Make sure Gaussian distributions are initialized correctly.
 */
TEMPLATE_TEST_CASE("GaussianDistributionEmptyConstructor", "[DistributionTest]",
    float, double)
{
  typedef typename arma::Mat<TestType> MatType;

  GaussianDistribution<MatType> d;

  REQUIRE(d.Mean().n_elem == 0);
  REQUIRE(d.Covariance().n_elem == 0);
}

/**
 * Make sure Gaussian distributions are initialized to the correct
 * dimensionality.
 */
TEMPLATE_TEST_CASE("GaussianDistributionDimensionalityConstructor",
                   "[DistributionTest]", float, double)
{
  typedef typename arma::Mat<TestType> MatType;

  GaussianDistribution<MatType> d(4);

  REQUIRE(d.Mean().n_elem == 4);
  REQUIRE(d.Covariance().n_rows == 4);
  REQUIRE(d.Covariance().n_cols == 4);
}

/**
 * Make sure Gaussian distributions are initialized correctly when we give a
 * mean and covariance.
 */
TEMPLATE_TEST_CASE("GaussianDistributionDistributionConstructor",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  VecType mean(3);
  MatType covariance(3, 3);

  mean.randu();
  covariance.randu();
  covariance *= covariance.t();
  covariance += arma::eye<MatType>(3, 3);

  GaussianDistribution<MatType> d(mean, covariance);

  for (size_t i = 0; i < 3; ++i)
    REQUIRE(d.Mean()[i] == Approx(mean[i]).epsilon(1e-7));

  for (size_t i = 0; i < 3; ++i)
    for (size_t j = 0; j < 3; ++j)
      REQUIRE(d.Covariance()(i, j) == Approx(covariance(i, j)).epsilon(1e-7));
}

/**
 * Make sure the probability of observations is correct.
 */
TEMPLATE_TEST_CASE("GaussianDistributionProbabilityTest", "[DistributionTest]",
    float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  VecType mean("5 6 3 3 2");
  MatType cov("6 1 1 1 2;"
              "1 7 1 0 0;"
              "1 1 4 1 1;"
              "1 0 1 7 0;"
              "2 0 1 0 6");

  GaussianDistribution<MatType> d(mean, cov);

  REQUIRE(d.LogProbability("0 1 2 3 4") ==
      Approx(-13.432076798791542).epsilon(1e-7));
  REQUIRE(d.LogProbability("3 2 3 7 8") ==
      Approx(-15.814880322345738).epsilon(1e-7));
  REQUIRE(d.LogProbability("2 2 0 8 1") ==
      Approx(-13.754462857772776).epsilon(1e-7));
  REQUIRE(d.LogProbability("2 1 5 0 1") ==
      Approx(-13.283283233107898).epsilon(1e-7));
  REQUIRE(d.LogProbability("3 0 5 1 0") ==
      Approx(-13.800326511545279).epsilon(1e-7));
  REQUIRE(d.LogProbability("4 0 6 1 0") ==
      Approx(-14.900192463287908).epsilon(1e-7));
}

/**
 * Test GaussianDistribution::Probability() in the univariate case.
 */
TEMPLATE_TEST_CASE("GaussianUnivariateProbabilityTest", "[DistributionTest]",
    float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  const ElemType tol = (std::is_same<ElemType, float>::value) ? 1e-4 : 1e-7;

  GaussianDistribution<MatType> g(VecType("0.0"), MatType("1.0"));

  // Simple case.
  REQUIRE(g.Probability(VecType("0.0")) ==
      Approx(0.398942280401433).epsilon(tol));
  REQUIRE(g.Probability(VecType("1.0")) ==
      Approx(0.241970724519143).epsilon(tol));
  REQUIRE(g.Probability(VecType("-1.0")) ==
      Approx(0.241970724519143).epsilon(tol));

  // A few more cases...
  MatType covariance;

  covariance = 2.0;
  g.Covariance(std::move(covariance));
  REQUIRE(g.Probability(VecType("0.0")) ==
      Approx(0.282094791773878).epsilon(tol));
  REQUIRE(g.Probability(VecType("1.0")) ==
      Approx(0.219695644733861).epsilon(tol));
  REQUIRE(g.Probability(VecType("-1.0")) ==
      Approx(0.219695644733861).epsilon(tol));

  g.Mean().fill(1.0);
  covariance = 1.0;
  g.Covariance(std::move(covariance));
  REQUIRE(g.Probability(VecType("1.0")) ==
      Approx(0.398942280401433).epsilon(tol));

  covariance = 2.0;
  g.Covariance(std::move(covariance));
  REQUIRE(g.Probability(VecType("-1.0")) ==
      Approx(0.103776874355149).epsilon(tol));
}

/**
 * Test GaussianDistribution::Probability() in the multivariate case.
 */
TEMPLATE_TEST_CASE("GaussianMultivariateProbabilityTest", "[DistributionTest]",
    float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  const ElemType tol = (std::is_same<ElemType, float>::value) ? 1e-4 : 1e-7;

  // Simple case.
  VecType mean = "0 0";
  MatType cov = "1 0; 0 1";
  VecType x = "0 0";

  GaussianDistribution<MatType> g(mean, cov);

  REQUIRE(g.Probability(x) == Approx(0.159154943091895).epsilon(tol));

  MatType covariance;
  covariance = "2 0; 0 2";
  g.Covariance(std::move(covariance));

  REQUIRE(g.Probability(x) == Approx(0.0795774715459477).epsilon(tol));

  x = "1 1";

  REQUIRE(g.Probability(x) == Approx(0.0482661763150270).epsilon(tol));
  REQUIRE(g.Probability(-x) == Approx(0.0482661763150270).epsilon(tol));

  g.Mean() = "1 1";
  REQUIRE(g.Probability(x) == Approx(0.0795774715459477).epsilon(tol));
  g.Mean() *= -1;
  REQUIRE(g.Probability(-x) == Approx(0.0795774715459477).epsilon(tol));

  g.Mean() = "1 1";
  covariance = "2 1.5; 1.5 4";
  g.Covariance(std::move(covariance));

  REQUIRE(g.Probability(x) == Approx(0.066372199406187285).epsilon(tol));
  g.Mean() *= -1;
  REQUIRE(g.Probability(-x) == Approx(0.066372199406187285).epsilon(tol));

  g.Mean() = "1 1";
  x = "-1 4";

  REQUIRE(g.Probability(x) == Approx(0.00072147262356379415).epsilon(tol));
  REQUIRE(g.Probability(-x) == Approx(0.00085851785428674523).epsilon(tol));

  // Higher-dimensional case.
  x = "0 1 2 3 4";
  g.Mean() = "5 6 3 3 2";

  covariance = "6 1 1 1 2;"
               "1 7 1 0 0;"
               "1 1 4 1 1;"
               "1 0 1 7 0;"
               "2 0 1 0 6";
  g.Covariance(std::move(covariance));

  REQUIRE(g.Probability(x) == Approx(1.4673143531128877e-06).epsilon(tol));
  REQUIRE(g.Probability(-x) == Approx(7.7404143494891786e-09).epsilon(
      0.01 * tol).margin(0.01 * tol));

  g.Mean() *= -1;
  REQUIRE(g.Probability(-x) == Approx(1.4673143531128877e-06).epsilon(tol));
  REQUIRE(g.Probability(x) == Approx(7.7404143494891786e-09).epsilon(
      0.01 * tol).margin(0.01 * tol));
}

/**
 * Test the phi() function, for multiple points in the multivariate Gaussian
 * case.
 */
TEMPLATE_TEST_CASE("GaussianMultipointMultivariateProbabilityTest",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  // Same case as before.
  VecType mean = "5 6 3 3 2";
  MatType cov("6 1 1 1 2;"
              "1 7 1 0 0;"
              "1 1 4 1 1;"
              "1 0 1 7 0;"
              "2 0 1 0 6");

  MatType points = "0 3 2 2 3 4;"
                   "1 2 2 1 0 0;"
                   "2 3 0 5 5 6;"
                   "3 7 8 0 1 1;"
                   "4 8 1 1 0 0;";

  VecType phis;
  GaussianDistribution<MatType> g(mean, cov);
  g.LogProbability(points, phis);

  REQUIRE(phis.n_elem == 6);

  REQUIRE(phis(0) == Approx(-13.432076798791542).epsilon(1e-7));
  REQUIRE(phis(1) == Approx(-15.814880322345738).epsilon(1e-7));
  REQUIRE(phis(2) == Approx(-13.754462857772776).epsilon(1e-7));
  REQUIRE(phis(3) == Approx(-13.283283233107898).epsilon(1e-7));
  REQUIRE(phis(4) == Approx(-13.800326511545279).epsilon(1e-7));
  REQUIRE(phis(5) == Approx(-14.900192463287908).epsilon(1e-7));
}

/**
 * Make sure random observations follow the probability distribution correctly.
 */
TEMPLATE_TEST_CASE("GaussianDistributionRandomTest", "[DistributionTest]",
    float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  const ElemType tol = (std::is_same<ElemType, float>::value) ? 0.3 : 0.125;

  VecType mean("1.0 2.25");
  MatType cov("0.85 0.60;"
              "0.60 1.45");

  GaussianDistribution<MatType> d(mean, cov);

  MatType obs(2, 7500);

  for (size_t i = 0; i < 7500; ++i)
    obs.col(i) = d.Random();

  // Now make sure that reflects the actual distribution.
  VecType obsMean = arma::mean(obs, 1);
  MatType obsCov = ColumnCovariance(obs);

  // 12.5% tolerance because this can be noisy.  (30% for floats.)
  REQUIRE(obsMean[0] == Approx(mean[0]).epsilon(tol));
  REQUIRE(obsMean[1] == Approx(mean[1]).epsilon(tol));

  REQUIRE(obsCov(0, 0) == Approx(cov(0, 0)).epsilon(tol));
  REQUIRE(obsCov(0, 1) == Approx(cov(0, 1)).epsilon(tol));
  REQUIRE(obsCov(1, 0) == Approx(cov(1, 0)).epsilon(tol));
  REQUIRE(obsCov(1, 1) == Approx(cov(1, 1)).epsilon(tol));
}

/**
 * Make sure that we can properly estimate from given observations.
 */
TEMPLATE_TEST_CASE("GaussianDistributionTrainTest", "[DistributionTest]", float,
    double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  const ElemType tol = (std::is_same<ElemType, float>::value) ? 1e-3 : 1e-5;

  VecType mean("1.0 3.0 0.0 2.5");
  MatType cov("3.0 0.0 1.0 4.0;"
              "0.0 2.4 0.5 0.1;"
              "1.0 0.5 6.3 0.0;"
              "4.0 0.1 0.0 9.1");

  // Now generate the observations.
  MatType observations(4, 10000);

  MatType transChol = trans(chol(cov));
  for (size_t i = 0; i < 10000; ++i)
    observations.col(i) = transChol * arma::randn<VecType>(4) + mean;

  // Now estimate.
  GaussianDistribution<MatType> d;

  // Find actual mean and covariance of data.
  VecType actualMean = arma::mean(observations, 1);
  MatType actualCov = ColumnCovariance(observations);

  d.Train(observations);

  // Check that everything is estimated right.
  for (size_t i = 0; i < 4; ++i)
    REQUIRE(d.Mean()[i] - actualMean[i] == Approx(0.0).margin(tol));

  for (size_t i = 0; i < 4; ++i)
    for (size_t j = 0; j < 4; ++j)
    {
      REQUIRE(d.Covariance()(i, j) - actualCov(i, j) ==
          Approx(0.0).margin(tol));
    }
}

/**
 * This test verifies the fitting of GaussianDistribution works properly when
 * probabilities for each sample is given.
 */
TEMPLATE_TEST_CASE("GaussianDistributionTrainWithProbabilitiesTest",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  const ElemType tol = (std::is_same<ElemType, float>::value) ? 0.25 : 0.1;

  VecType mean = ("5.0");
  VecType cov = ("2.0");

  GaussianDistribution<MatType> dist(mean, cov);
  size_t N = 15000;
  size_t d = 1;

  MatType rdata(d, N);
  for (size_t i = 0; i < N; ++i)
    rdata.col(i) = dist.Random();

  VecType probabilities(N);
  for (size_t i = 0; i < N; ++i)
    probabilities(i) = Random();

  // Fit distribution with probabilities and data.
  GaussianDistribution<MatType> guDist;
  guDist.Train(rdata, probabilities);

  // Fit distribution only with data.
  GaussianDistribution<MatType> guDist2;
  guDist2.Train(rdata);

  REQUIRE(guDist.Mean()[0] == Approx(guDist2.Mean()[0]).epsilon(tol));
  REQUIRE(guDist.Covariance()[0] ==
      Approx(guDist2.Covariance()[0]).epsilon(tol));

  REQUIRE(guDist.Mean()[0] == Approx(mean[0]).epsilon(tol));
  REQUIRE(guDist.Covariance()[0] == Approx(cov[0]).epsilon(tol));
}

/**
 * This test ensures that the same result is obtained when trained with
 * probabilities all set to 1 and with no probabilities at all.
 */
TEMPLATE_TEST_CASE("GaussianDistributionWithProbabilties1Test",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  const ElemType tol1 = (std::is_same<ElemType, float>::value) ? 1e-10 : 1e-17;
  const ElemType tol2 = (std::is_same<ElemType, float>::value) ? 1e-2 : 1e-4;

  VecType mean = ("5.0");
  VecType cov  = ("4.0");

  GaussianDistribution<MatType> dist(mean, cov);
  size_t N = 50000;
  size_t d = 1;

  MatType rdata(d, N);
  for (size_t i = 0; i < N; ++i)
      rdata.col(i) = Random();

  VecType probabilities(N, arma::fill::ones);

  // Fit the distribution with only data.
  GaussianDistribution<MatType> guDist;
  guDist.Train(rdata);

  // Fit the distribution with data and each probability as 1.
  GaussianDistribution<MatType> guDist2;
  guDist2.Train(rdata, probabilities);

  REQUIRE(guDist.Mean()[0] == Approx(guDist2.Mean()[0]).epsilon(tol1));
  REQUIRE(guDist.Covariance()[0] ==
      Approx(guDist2.Covariance()[0]).epsilon(tol2));
}

/**
 * This test draws points from two different normal distributions, sets the
 * probabilities for points from the first distribution to something small and
 * the probabilities for the second to something large.
 *
 * We expect that the distribution we recover after training to be the same as
 * the second normal distribution (the one with high probabilities).
 */
TEMPLATE_TEST_CASE("GaussianDistributionTrainWithTwoDistProbabilitiesTest",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  VecType mean1 = ("5.0");
  VecType cov1 = ("4.0");

  VecType mean2 = ("3.0");
  VecType cov2 = ("1.0");

  // Create two GaussianDistributions with different parameters.
  GaussianDistribution<MatType> dist1(mean1, cov1);
  GaussianDistribution<MatType> dist2(mean2, cov2);

  size_t N = 50000;
  size_t d = 1;

  MatType rdata(d, N);
  VecType probabilities(N);

  // Fill even numbered columns with random points from dist1 and odd numbered
  // columns with random points from dist2.
  for (size_t j = 0; j < N; ++j)
  {
    if (j % 2 == 0)
      rdata.col(j) = dist1.Random();
    else
      rdata.col(j) = dist2.Random();
  }

  // Assign high probabilities to points drawn from dist1 and low probabilities
  // to numbers drawn from dist2.
  for (size_t i = 0 ; i < N ; ++i)
  {
    if (i % 2 == 0)
      probabilities(i) = Random(0.98, 1);
    else
      probabilities(i) = Random(0, 0.02);
  }

  GaussianDistribution<MatType> guDist;
  guDist.Train(rdata, probabilities);

  REQUIRE(guDist.Mean()[0] == Approx(mean1[0]).epsilon(0.05));
  REQUIRE(guDist.Covariance()[0] == Approx(cov1[0]).epsilon(0.05));
}

/******************************/
/** Gamma Distribution Tests **/
/******************************/
/**
 * Make sure that using an object to fit one reference set and then asking
 * to fit another works properly.
 */
TEMPLATE_TEST_CASE("GammaDistributionTrainTest", "[DistributionTest]", float,
    double)
{
  typedef TestType ElemType;
  typedef typename arma::Mat<ElemType> MatType;

  // Create a gamma distribution random generator.
  ElemType alphaReal = 5.3;
  ElemType betaReal = 1.5;
  std::gamma_distribution<ElemType> dist(alphaReal, betaReal);

  // Create a N x d gamma distribution data and fit the results.
  size_t N = 200;
  size_t d = 2;
  MatType rdata(d, N);

  // Random generation of gamma-like points.
  for (size_t j = 0; j < d; ++j)
    for (size_t i = 0; i < N; ++i)
      rdata(j, i) = dist(RandGen());

  // Create GammaDistribution object and call Train() on reference set.
  GammaDistribution<MatType> gDist;
  gDist.Train(rdata);

  // Training must estimate d pairs of alpha and beta parameters.
  REQUIRE(gDist.Dimensionality() == d);
  REQUIRE(gDist.Dimensionality() == d);

  // Create a N' x d' gamma distribution, fit results without new object.
  size_t N2 = 350;
  size_t d2 = 4;
  MatType rdata2(d2, N2);

  // Random generation of gamma-like points.
  for (size_t j = 0; j < d2; ++j)
    for (size_t i = 0; i < N2; ++i)
      rdata2(j, i) = dist(RandGen());

  // Fit results using old object.
  gDist.Train(rdata2);

  // Training must estimate d' pairs of alpha and beta parameters.
  REQUIRE(gDist.Dimensionality() == d2);
  REQUIRE(gDist.Dimensionality() == d2);
}

/**
 * This test verifies that the fitting procedure for GammaDistribution works
 * properly when probabilities for each sample is given.
 */
TEMPLATE_TEST_CASE("GammaDistributionTrainWithProbabilitiesTest",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  const ElemType tol = (std::is_same<ElemType, float>::value) ? 0.03 : 0.015;

  ElemType alphaReal = 5.4;
  ElemType betaReal = 6.7;

  // Create a gamma distribution random generator.
  std::gamma_distribution<ElemType> dist(alphaReal, betaReal);

  size_t N = 50000;
  size_t d = 2;
  MatType rdata(d, N);

  for (size_t j = 0; j < d; ++j)
    for (size_t i = 0; i < N; ++i)
      rdata(j, i) = dist(RandGen());

  // Fill the probabilities randomly.
  VecType probabilities(N, arma::fill::randu);

  // Fit results with probabilities and data.
  GammaDistribution<MatType> gDist;
  gDist.Train(rdata, probabilities);

  // Fit results with only data.
  GammaDistribution<MatType> gDist2;
  gDist2.Train(rdata);

  REQUIRE(gDist2.Alpha(0) == Approx(gDist.Alpha(0)).epsilon(tol));
  REQUIRE(gDist2.Beta(0) == Approx(gDist.Beta(0)).epsilon(tol));

  REQUIRE(gDist2.Alpha(1) == Approx(gDist.Alpha(1)).epsilon(tol));
  REQUIRE(gDist2.Beta(1) == Approx(gDist.Beta(1)).epsilon(tol));

  REQUIRE(alphaReal == Approx(gDist.Alpha(0)).epsilon(2 * tol));
  REQUIRE(betaReal == Approx(gDist.Beta(0)).epsilon(2 * tol));

  REQUIRE(alphaReal == Approx(gDist.Alpha(1)).epsilon(2 * tol));
  REQUIRE(betaReal == Approx(gDist.Beta(1)).epsilon(2 * tol));
}

/**
 * This test ensures that the same result is obtained when trained with
 * probabilities all set to 1 and with no probabilities at all.
 */
TEMPLATE_TEST_CASE("GammaDistributionTrainAllProbabilities1Test",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  ElemType alphaReal = 5.4;
  ElemType betaReal = 6.7;

  // Create a gamma distribution random generator.
  std::gamma_distribution<ElemType> dist(alphaReal, betaReal);

  size_t N = 1000;
  size_t d = 2;
  MatType rdata(d, N);

  for (size_t j = 0; j < d; ++j)
    for (size_t i = 0; i < N; ++i)
      rdata(j, i) = dist(RandGen());

  // Fit results with only data.
  GammaDistribution<MatType> gDist;
  gDist.Train(rdata);

  // Fit results with data and each probability as 1.
  GammaDistribution<MatType> gDist2;
  VecType allProbabilities1(N, arma::fill::ones);
  gDist2.Train(rdata, allProbabilities1);

  REQUIRE(gDist2.Alpha(0) == Approx(gDist.Alpha(0)).epsilon(1e-7));
  REQUIRE(gDist2.Beta(0) == Approx(gDist.Beta(0)).epsilon(1e-7));

  REQUIRE(gDist2.Alpha(1) == Approx(gDist.Alpha(1)).epsilon(1e-7));
  REQUIRE(gDist2.Beta(1) == Approx(gDist.Beta(1)).epsilon(1e-7));
}

/**
 * This test draws points from two different gamma distributions, sets the
 * probabilities for the points from the first distribution to something small
 * and the probabilities for the second to something large.  It ensures that the
 * gamma distribution recovered has the same parameters as the second gamma
 * distribution with high probabilities.
 */
TEMPLATE_TEST_CASE("GammaDistributionTrainTwoDistProbabilities1Test",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  const ElemType tol = (std::is_same<ElemType, float>::value) ? 0.25 : 0.075;

  ElemType alphaReal = 5.4;
  ElemType betaReal = 6.7;

  ElemType alphaReal2 = 1.9;
  ElemType betaReal2 = 8.4;

  // Create two gamma distribution random generators.
  std::gamma_distribution<ElemType> dist(alphaReal, betaReal);
  std::gamma_distribution<ElemType> dist2(alphaReal2, betaReal2);

  size_t N = 50000;
  size_t d = 2;
  MatType rdata(d, N);
  VecType probabilities(N);

  // Draw points alternately from the two different distributions.
  for (size_t j = 0; j < d; ++j)
  {
    for (size_t i = 0; i < N; ++i)
    {
      if (i % 2 == 0)
        rdata(j, i) = dist(RandGen());
      else
        rdata(j, i) = dist2(RandGen());
    }
  }

  for (size_t i = 0; i < N; ++i)
  {
    if (i % 2 == 0)
      probabilities(i) = 0.02 * Random();
    else
      probabilities(i) = 0.98 + 0.02 * Random();
  }

  GammaDistribution<MatType> gDist;
  gDist.Train(rdata, probabilities);

  REQUIRE(alphaReal2 == Approx(gDist.Alpha(0)).epsilon(tol));
  REQUIRE(betaReal2 == Approx(gDist.Beta(0)).epsilon(tol));

  REQUIRE(alphaReal2 == Approx(gDist.Alpha(1)).epsilon(tol));
  REQUIRE(betaReal2 == Approx(gDist.Beta(1)).epsilon(tol));
}

/**
 * This test verifies that the fitting procedure for GammaDistribution works
 * properly and converges near the actual gamma parameters. We do this twice
 * with different alpha/beta parameters so we make sure we don't have some weird
 * bug that always converges to the same number.
 */
TEMPLATE_TEST_CASE("GammaDistributionFittingTest", "[DistributionTest]", float,
    double)
{
  typedef TestType ElemType;
  typedef typename arma::Mat<ElemType> MatType;

  // Offset from the actual alpha/beta. 10% is quite a relaxed tolerance since
  // the random points we generate are few (for test speed) and might be fitted
  // better by a similar distribution.
  ElemType errorTolerance = 10;

  size_t N = 5000;
  size_t d = 1; // Only 1 dimension is required for this.

  /** Iteration 1 (first parameter set) **/

  // Create a gamma-random generator and data.
  ElemType alphaReal = 5.3;
  ElemType betaReal = 1.5;
  std::gamma_distribution<ElemType> dist(alphaReal, betaReal);

  // Random generation of gamma-like points.
  MatType rdata(d, N);
  for (size_t j = 0; j < d; ++j)
    for (size_t i = 0; i < N; ++i)
      rdata(j, i) = dist(RandGen());

  // Create Gamma object and call Train() on reference set.
  GammaDistribution<MatType> gDist;
  gDist.Train(rdata);

  // Estimated parameter must be close to real.
  REQUIRE(gDist.Alpha(0) == Approx(alphaReal).epsilon(errorTolerance / 100));
  REQUIRE(gDist.Beta(0) == Approx(betaReal).epsilon(errorTolerance / 100));

  /** Iteration 2 (different parameter set) **/

  // Create a gamma-random generator and data.
  ElemType alphaReal2 = 7.2;
  ElemType betaReal2 = 0.9;
  std::gamma_distribution<ElemType> dist2(alphaReal2, betaReal2);

  // Random generation of gamma-like points.
  MatType rdata2(d, N);
  for (size_t j = 0; j < d; ++j)
    for (size_t i = 0; i < N; ++i)
      rdata2(j, i) = dist2(RandGen());

  // Create Gamma object and call Train() on reference set.
  GammaDistribution<MatType> gDist2;
  gDist2.Train(rdata2);

  // Estimated parameter must be close to real.
  REQUIRE(gDist2.Alpha(0) == Approx(alphaReal2).epsilon(errorTolerance / 100));
  REQUIRE(gDist2.Beta(0) == Approx(betaReal2).epsilon(errorTolerance / 100));
}

/**
 * Test that Train() and the constructor that takes data give the same resulting
 * distribution.
 */
TEMPLATE_TEST_CASE("GammaDistributionTrainConstructorTest",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Mat<ElemType> MatType;

  const MatType data = arma::randu<MatType>(10, 500);

  GammaDistribution<MatType> d1(data);
  GammaDistribution<MatType> d2;
  d2.Train(data);

  for (size_t i = 0; i < 10; ++i)
  {
    REQUIRE(d1.Alpha(i) == Approx(d2.Alpha(i)).epsilon(1e-7));
    REQUIRE(d1.Beta(i) == Approx(d2.Beta(i)).epsilon(1e-7));
  }
}

/**
 * Test that Train() with a dataset and Train() with dataset statistics return
 * the same results.
 */
TEMPLATE_TEST_CASE("GammaDistributionTrainStatisticsTest", "[DistributionTest]",
    float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  const MatType data = arma::randu<MatType>(1, 500);

  // Train object d1 with the data.
  GammaDistribution<MatType> d1(data);

  // Train object d2 with the data's statistics.
  GammaDistribution<MatType> d2;
  const VecType meanLogx = arma::mean(log(data), 1);
  const VecType meanx = arma::mean(data, 1);
  const VecType logMeanx = log(meanx);
  d2.Train(logMeanx, meanLogx, meanx);

  REQUIRE(d1.Alpha(0) == Approx(d2.Alpha(0)).epsilon(1e-7));
  REQUIRE(d1.Beta(0) == Approx(d2.Beta(0)).epsilon(1e-7));
}

/**
 * Tests that Random() generates points that can be reasonably well fit by the
 * distribution that generated them.
 */
TEMPLATE_TEST_CASE("GammaDistributionRandomTest", "[DistributionTest]", float,
    double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  const VecType a("2.0 2.5 3.0"), b("0.4 0.6 1.3");
  const size_t numPoints = 4000;

  // Distribution to generate points.
  GammaDistribution<MatType> d1(a, b);
  MatType data(3, numPoints); // 3-d points.

  for (size_t i = 0; i < numPoints; ++i)
    data.col(i) = d1.Random();

  // Distribution to fit points.
  GammaDistribution<MatType> d2(data);
  for (size_t i = 0; i < 3; ++i)
  {
    REQUIRE(d2.Alpha(i) == Approx(a(i)).epsilon(0.15)); // Within 15%
    REQUIRE(d2.Beta(i) == Approx(b(i)).epsilon(0.15));
  }
}

TEMPLATE_TEST_CASE("GammaDistributionProbabilityTest", "[DistributionTest]",
    float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  // Train two 1-dimensional distributions.
  const VecType a1("2.0"), b1("0.9"), a2("3.1"), b2("1.4");
  MatType x1("2.0"), x2("2.94");
  VecType prob1, prob2;

  // Evaluated at wolfram|alpha
  GammaDistribution<MatType> d1(a1, b1);
  d1.Probability(x1, prob1);
  REQUIRE(prob1(0) == Approx(0.267575).epsilon(1e-5));

  // Evaluated at wolfram|alpha
  GammaDistribution<MatType> d2(a2, b2);
  d2.Probability(x2, prob2);
  REQUIRE(prob2(0) == Approx(0.189043).epsilon(1e-5));

  // Check that the overload that returns the probability for 1 dimension
  // agrees.
  REQUIRE(prob2(0) == Approx(d2.Probability(2.94, 0)).epsilon(1e-7));

  // Combine into one 2-dimensional distribution.
  const VecType a3("2.0 3.1"), b3("0.9 1.4");
  MatType x3(2, 2);
  x3 = { { 2.0, 2.94 },
         { 2.0, 2.94 } };
  VecType prob3;

  // Expect that the 2-dimensional distribution returns the product of the
  // 1-dimensional distributions (evaluated at wolfram|alpha).
  GammaDistribution<MatType> d3(a3, b3);
  d3.Probability(x3, prob3);
  REQUIRE(prob3(0) == Approx(0.04408).epsilon(1e-4));
  REQUIRE(prob3(1) == Approx(0.026165).epsilon(1e-4));
}

TEMPLATE_TEST_CASE("GammaDistributionLogProbabilityTest", "[DistributionTest]",
    float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  // Train two 1-dimensional distributions.
  const VecType a1("2.0"), b1("0.9"), a2("3.1"), b2("1.4");
  MatType x1("2.0"), x2("2.94");
  VecType logprob1, logprob2;

  // Evaluated at wolfram|alpha
  GammaDistribution<MatType> d1(a1, b1);
  d1.LogProbability(x1, logprob1);
  REQUIRE(logprob1(0) == Approx(std::log(0.267575)).epsilon(1e-5));

  // Evaluated at wolfram|alpha
  GammaDistribution<MatType> d2(a2, b2);
  d2.LogProbability(x2, logprob2);
  REQUIRE(logprob2(0) == Approx(std::log(0.189043)).epsilon(1e-5));

  // Check that the overload that returns the log probability for
  // 1 dimension agrees.
  REQUIRE(logprob2(0) == Approx(d2.LogProbability(2.94, 0)).epsilon(1e-7));

  // Combine into one 2-dimensional distribution.
  const VecType a3("2.0 3.1"), b3("0.9 1.4");
  MatType x3(2, 2);
  x3 = { { 2.0, 2.94 },
         { 2.0, 2.94 } };
  VecType logprob3;

  // Expect that the 2-dimensional distribution returns the product of the
  // 1-dimensional distributions (evaluated at wolfram|alpha).
  GammaDistribution<MatType> d3(a3, b3);
  d3.LogProbability(x3, logprob3);
  REQUIRE(logprob3(0) == Approx(std::log(0.04408)).epsilon(1e-5));
  REQUIRE(logprob3(1) == Approx(std::log(0.026165)).epsilon(1e-5));
}

/**
 * Discrete Distribution serialization test.
 */
TEMPLATE_TEST_CASE("DiscreteDistributionTest", "[DistributionTest]",
    (std::pair<double, double>),
    (std::pair<double, size_t>),
    (std::pair<float, float>),
    (std::pair<float, size_t>),
    (std::pair<float, unsigned short>))
{
  typedef typename TestType::first_type ElemType;
  typedef typename TestType::second_type ObsElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;
  typedef typename arma::Col<ObsElemType> ObsVecType;
  typedef typename arma::Mat<ObsElemType> ObsMatType;

  const ElemType tol = (std::is_same<ElemType, float>::value) ? 1e-4 : 1e-8;

  // I assume that I am properly saving vectors, so, this should be
  // straightforward.
  VecType prob;
  prob.randu(12);
  std::vector<VecType> probVector = std::vector<VecType>(1, prob);
  DiscreteDistribution<MatType, ObsMatType> t(probVector);

  DiscreteDistribution<MatType, ObsMatType> xmlT, jsonT, binaryT;

  // Load and save with all serializers.
  SerializeObjectAll(t, xmlT, jsonT, binaryT);

  for (size_t i = 0; i < 12; ++i)
  {
    ObsVecType obs(1);
    obs[0] = (ObsElemType) i;
    const ElemType prob = t.Probability(obs);
    if (prob == 0.0)
    {
      REQUIRE(xmlT.Probability(obs) == Approx(0.0).margin(tol));
      REQUIRE(jsonT.Probability(obs) == Approx(0.0).margin(tol));
      REQUIRE(binaryT.Probability(obs) == Approx(0.0).margin(tol));
    }
    else
    {
      REQUIRE(prob == Approx(xmlT.Probability(obs)).epsilon(tol));
      REQUIRE(prob == Approx(jsonT.Probability(obs)).epsilon(tol));
      REQUIRE(prob == Approx(binaryT.Probability(obs)).epsilon(tol));
    }
  }
}

/**
 * Gaussian Distribution serialization test.
 */
TEST_CASE("GaussianDistributionTest", "[DistributionTest]")
{
  arma::vec mean(10);
  mean.randu();
  // Generate a covariance matrix.
  arma::mat cov;
  cov.randu(10, 10);
  cov = (cov * cov.t());

  GaussianDistribution<> g(mean, cov);
  GaussianDistribution<> xmlG, jsonG, binaryG;

  SerializeObjectAll(g, xmlG, jsonG, binaryG);

  REQUIRE(g.Dimensionality() == xmlG.Dimensionality());
  REQUIRE(g.Dimensionality() == jsonG.Dimensionality());
  REQUIRE(g.Dimensionality() == binaryG.Dimensionality());

  // First, check the means.
  CheckMatrices(g.Mean(), xmlG.Mean(), jsonG.Mean(), binaryG.Mean());

  // Now, check the covariance.
  CheckMatrices(g.Covariance(), xmlG.Covariance(), jsonG.Covariance(),
      binaryG.Covariance());

  // Lastly, run some observations through and make sure the probability is the
  // same.  This should test anything cached internally.
  arma::mat randomObs;
  randomObs.randu(10, 500);

  for (size_t i = 0; i < 500; ++i)
  {
    const double prob = g.Probability(randomObs.unsafe_col(i));

    if (prob == 0.0)
    {
      REQUIRE(xmlG.Probability(randomObs.unsafe_col(i)) ==
          Approx(0.0).margin(1e-8));
      REQUIRE(jsonG.Probability(randomObs.unsafe_col(i)) ==
          Approx(0.0).margin(1e-8));
      REQUIRE(binaryG.Probability(randomObs.unsafe_col(i)) ==
          Approx(0.0).margin(1e-8));
    }
    else
    {
      REQUIRE(prob ==
          Approx(xmlG.Probability(randomObs.unsafe_col(i))).epsilon(1e-10));
      REQUIRE(prob ==
          Approx(jsonG.Probability(randomObs.unsafe_col(i))).epsilon(1e-10));
      REQUIRE(prob ==
          Approx(binaryG.Probability(randomObs.unsafe_col(i))).epsilon(1e-10));
    }
  }
}

/**
 * Laplace Distribution serialization test.
 */
TEMPLATE_TEST_CASE("LaplaceDistributionTest", "[DistributionTest]", float,
    double)
{
  typedef TestType ElemType;
  typedef arma::Col<ElemType> VecType;
  typedef arma::Mat<ElemType> MatType;

  VecType mean(20);
  mean.randu();

  LaplaceDistribution<MatType> l(mean, 2.5);
  LaplaceDistribution<MatType> xmlL, jsonL, binaryL;

  SerializeObjectAll(l, xmlL, jsonL, binaryL);

  REQUIRE(l.Scale() == Approx(xmlL.Scale()).epsilon(1e-10));
  REQUIRE(l.Scale() == Approx(jsonL.Scale()).epsilon(1e-10));
  REQUIRE(l.Scale() == Approx(binaryL.Scale()).epsilon(1e-10));

  CheckMatrices(l.Mean(), xmlL.Mean(), jsonL.Mean(), binaryL.Mean());
}

/**
 * Laplace Distribution Probability Test.
 */
TEMPLATE_TEST_CASE("LaplaceDistributionProbabilityTest", "[DistributionTest]",
    float, double)
{
  typedef TestType ElemType;
  typedef arma::Col<ElemType> VecType;
  typedef arma::Mat<ElemType> MatType;

  LaplaceDistribution<MatType> l(VecType("0.0"), 1.0);

  // Simple case.
  REQUIRE(l.Probability(VecType("0.0")) ==
      Approx(0.500000000000000).epsilon(1e-7));
  REQUIRE(l.Probability(VecType("1.0")) ==
      Approx(0.183939720585721).epsilon(1e-7));

  MatType points = "0.0 1.0;";

  VecType probabilities;

  l.Probability(points, probabilities);

  REQUIRE(probabilities.n_elem == 2);

  REQUIRE(probabilities(0) == Approx(0.500000000000000).epsilon(1e-7));
  REQUIRE(probabilities(1) == Approx(0.183939720585721).epsilon(1e-7));
}

/**
 * Laplace Distribution Log Probability Test.
 */
TEMPLATE_TEST_CASE("LaplaceDistributionLogProbabilityTest",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;
  typedef arma::Col<ElemType> VecType;
  typedef arma::Mat<ElemType> MatType;

  LaplaceDistribution<MatType> l(VecType("0.0"), 1.0);

  // Simple case.
  REQUIRE(l.LogProbability(VecType("0.0")) ==
      Approx(-0.693147180559945).epsilon(1e-7));
  REQUIRE(l.LogProbability(VecType("1.0")) ==
      Approx(-1.693147180559946).epsilon(1e-7));

  MatType points = "0.0 1.0;";

  VecType logProbabilities;

  l.LogProbability(points, logProbabilities);

  REQUIRE(logProbabilities.n_elem == 2);

  REQUIRE(logProbabilities(0) ==
      Approx(-0.693147180559945).epsilon(1e-7));

  REQUIRE(logProbabilities(1) ==
      Approx(-1.693147180559946).epsilon(1e-7));
}

/**
 * Regression distribution serialization test.
 */
TEST_CASE("RegressionDistributionTest", "[DistributionTest]")
{
  // Generate some random data.
  arma::mat data;
  data.randn(15, 800);
  arma::rowvec responses;
  responses.randn(800);

  RegressionDistribution<> rd(data, responses);
  RegressionDistribution<> xmlRd, jsonRd, binaryRd;

  // Okay, now save it and load it.
  SerializeObjectAll(rd, xmlRd, jsonRd, binaryRd);

  // Check the gaussian distribution.
  CheckMatrices(rd.Err().Mean(),
                xmlRd.Err().Mean(),
                jsonRd.Err().Mean(),
                binaryRd.Err().Mean());
  CheckMatrices(rd.Err().Covariance(),
                xmlRd.Err().Covariance(),
                jsonRd.Err().Covariance(),
                binaryRd.Err().Covariance());

  // Check the regression function.
  if (rd.Rf().Lambda() == 0.0)
  {
    REQUIRE(xmlRd.Rf().Lambda() == Approx(0.0).margin(1e-8));
    REQUIRE(jsonRd.Rf().Lambda() == Approx(0.0).margin(1e-8));
    REQUIRE(binaryRd.Rf().Lambda() == Approx(0.0).margin(1e-8));
  }
  else
  {
    REQUIRE(rd.Rf().Lambda() == Approx(xmlRd.Rf().Lambda()).epsilon(1e-10));
    REQUIRE(rd.Rf().Lambda() == Approx(jsonRd.Rf().Lambda()).epsilon(1e-10));
    REQUIRE(rd.Rf().Lambda() == Approx(binaryRd.Rf().Lambda()).epsilon(1e-10));
  }

  CheckMatrices(rd.Rf().Parameters(),
                xmlRd.Rf().Parameters(),
                jsonRd.Rf().Parameters(),
                binaryRd.Rf().Parameters());
}

/*****************************************************/
/** Diagonal Covariance Gaussian Distribution Tests **/
/*****************************************************/

/**
 * Make sure Diagonal Covariance Gaussian distributions are initialized
 * correctly.
 */
TEMPLATE_TEST_CASE("DiagonalGaussianDistributionEmptyConstructor",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;

  DiagonalGaussianDistribution<arma::Mat<ElemType>> d;

  REQUIRE(d.Mean().n_elem == 0);
  REQUIRE(d.Covariance().n_elem == 0);
}

/**
 * Make sure Diagonal Covariance Gaussian distributions are initialized to
 * the correct dimensionality.
 */
TEMPLATE_TEST_CASE("DiagonalGaussianDistributionDimensionalityConstructor",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;

  DiagonalGaussianDistribution<arma::Mat<ElemType>> d(4);

  REQUIRE(d.Mean().n_elem == 4);
  REQUIRE(d.Covariance().n_elem == 4);
}

/**
 * Make sure Diagonal Covariance Gaussian distributions are initialized
 * correctly when we give a mean and covariance.
 */
TEMPLATE_TEST_CASE("DiagonalGaussianDistributionConstructor",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  VecType mean = arma::randu<VecType>(3);
  VecType covariance = arma::randu<VecType>(3);

  DiagonalGaussianDistribution<MatType> d(mean, covariance);

  // Make sure the mean and covariance is correct.
  for (size_t i = 0; i < 3; ++i)
  {
    REQUIRE(d.Mean()(i) == Approx(mean(i)).epsilon(1e-7));
    REQUIRE(d.Covariance()(i) == Approx(covariance(i)).epsilon(1e-7));
  }
}

/**
 * Make sure the probability of observations is correct.
 * The values were calculated using 'dmvnorm' in R.
 */
TEMPLATE_TEST_CASE("DiagonalGaussianDistributionProbabilityTest",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  VecType mean("2 5 3 4 1");
  VecType cov("3 1 5 3 2");

  DiagonalGaussianDistribution<MatType> d(mean, cov);

  // Observations lists randomly selected.
  REQUIRE(d.LogProbability("3 5 2 7 8") ==
    Approx(-20.861264167855161).epsilon(1e-7));
  REQUIRE(d.LogProbability("7 8 4 0 5") ==
    Approx(-22.277930834521829).epsilon(1e-7));
  REQUIRE(d.LogProbability("6 8 7 7 5") ==
    Approx(-21.111264167855161).epsilon(1e-7));
  REQUIRE(d.LogProbability("2 9 5 6 3") ==
    Approx(-16.9112641678551621).epsilon(1e-7));
  REQUIRE(d.LogProbability("5 8 2 9 7") ==
    Approx(-26.111264167855161).epsilon(1e-7));
}

/**
 * Test DiagonalGaussianDistribution::Probability() in the univariate case.
 * The values were calculated using 'dmvnorm' in R.
 */
TEMPLATE_TEST_CASE("DiagonalGaussianUnivariateProbabilityTest",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  const ElemType tol = (std::is_same<ElemType, float>::value) ? 1e-4 : 1e-7;

  DiagonalGaussianDistribution<MatType> d(VecType("0.0"), VecType("1.0"));

  // Mean: 0.0, Covariance: 1.0
  REQUIRE(d.Probability("0.0") == Approx(0.3989422804014327).epsilon(tol));
  REQUIRE(d.Probability("1.0") == Approx(0.24197072451914337).epsilon(tol));
  REQUIRE(d.Probability("-1.0") == Approx(0.24197072451914337).epsilon(tol));

  // Mean: 0.0, Covariance: 2.0
  d.Covariance("2.0");
  REQUIRE(d.Probability("0.0") == Approx(0.28209479177387814).epsilon(tol));
  REQUIRE(d.Probability("1.0") == Approx(0.21969564473386122).epsilon(tol));
  REQUIRE(d.Probability("-1.0") == Approx(0.21969564473386122).epsilon(tol));

  // Mean: 1.0, Covariance: 1.0
  d.Mean() = "1.0";
  d.Covariance("1.0");
  REQUIRE(d.Probability("0.0") == Approx(0.24197072451914337).epsilon(tol));
  REQUIRE(d.Probability("1.0") == Approx(0.3989422804014327).epsilon(tol));
  REQUIRE(d.Probability("-1.0") == Approx(0.053990966513188056).epsilon(tol));

  // Mean: 1.0, Covariance: 2.0
  d.Covariance("2.0");
  REQUIRE(d.Probability("0.0") == Approx(0.21969564473386122).epsilon(tol));
  REQUIRE(d.Probability("1.0") == Approx(0.28209479177387814).epsilon(tol));
  REQUIRE(d.Probability("-1.0") == Approx(0.10377687435514872).epsilon(tol));
}

/**
 * Test DiagonalGaussianDistribution::Probability() in the multivariate case.
 * The values were calculated using 'dmvnorm' in R.
 */
TEMPLATE_TEST_CASE("DiagonalGaussianMultivariateProbabilityTest",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  const ElemType tol = (std::is_same<ElemType, float>::value) ? 1e-4 : 1e-7;

  VecType mean("0 0");
  VecType cov("2 2");
  VecType obs("0 0");

  DiagonalGaussianDistribution<MatType> d(mean, cov);

  REQUIRE(d.Probability(obs) == Approx(0.079577471545947673).epsilon(tol));

  obs = "1 1";
  REQUIRE(d.Probability(obs) == Approx(0.048266176315026957).epsilon(tol));

  d.Mean() = "1 3";
  REQUIRE(d.Probability(obs) == Approx(0.029274915762159581).epsilon(tol));
  REQUIRE(d.Probability(-obs) == Approx(0.00053618878559782773).epsilon(tol));

  // Higher dimensional case.
  d.Mean() = "1 3 6 2 7";
  d.Covariance("3 1 5 3 2");
  obs = "2 5 7 3 8";
  REQUIRE(d.Probability(obs) == Approx(7.2790083003378082e-05).epsilon(tol));
}

/**
 * Test the phi() function, for multiple points in the multivariate Gaussian
 * case. The values were calculated using 'dmvnorm' in R.
 */
TEMPLATE_TEST_CASE("DiagonalGaussianMultipointMultivariateProbabilityTest",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  VecType mean = "2 5 3 7 2";
  VecType cov("9 2 1 4 8");
  MatType points = "3 5 2 7 5 8;"
                   "2 6 8 3 4 6;"
                   "1 4 2 7 8 2;"
                   "6 8 4 7 9 2;"
                   "4 6 7 7 3 2";
  VecType phis;
  DiagonalGaussianDistribution<MatType> d(mean, cov);
  d.LogProbability(points, phis);

  REQUIRE(phis.n_elem == 6);

  REQUIRE(phis(0) == Approx(-12.453302051926864).epsilon(1e-7));
  REQUIRE(phis(1) == Approx(-10.147746496371308).epsilon(1e-7));
  REQUIRE(phis(2) == Approx(-13.210246496371308).epsilon(1e-7));
  REQUIRE(phis(3) == Approx(-19.724135385260197).epsilon(1e-7));
  REQUIRE(phis(4) == Approx(-21.585246496371308).epsilon(1e-7));
  REQUIRE(phis(5) == Approx(-13.647746496371308).epsilon(1e-7));
}

/**
 * Make sure random observations follow the probability distribution correctly.
 */
TEMPLATE_TEST_CASE("DiagonalGaussianDistributionRandomTest",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  const ElemType tol = (std::is_same<ElemType, float>::value) ? 0.2 : 0.1;

  VecType mean("2.5 1.25");
  VecType cov("0.50 0.25");

  DiagonalGaussianDistribution<MatType> d(mean, cov);

  MatType obs(2, 5000);
  for (size_t i = 0; i < 5000; ++i)
    obs.col(i) = d.Random();

  // Make sure that reflects the actual distribution.
  VecType obsMean = arma::mean(obs, 1);
  MatType obsCov = ColumnCovariance(obs);

  // 10% tolerance because this can be noisy.  (20% for floats.)
  REQUIRE(obsMean(0) == Approx(mean(0)).epsilon(tol));
  REQUIRE(obsMean(1) == Approx(mean(1)).epsilon(tol));

  REQUIRE(obsCov(0, 0) == Approx(cov(0)).epsilon(tol));
  REQUIRE(obsCov(1, 1) == Approx(cov(1)).epsilon(tol));
}

/**
 * Make sure that we can properly estimate from given observations.
 */
TEMPLATE_TEST_CASE("DiagonalGaussianDistributionTrainTest",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  const ElemType tol = (std::is_same<ElemType, float>::value) ? 1e-3 : 1e-5;

  VecType mean("2.5 1.5 8.2 3.1");
  VecType cov("1.2 3.1 8.3 4.3");

  // Generate the observations.
  MatType observations(4, 10000);

  for (size_t i = 0; i < 10000; ++i)
    observations.col(i) = (sqrt(cov) % arma::randn<VecType>(4)) + mean;

  DiagonalGaussianDistribution<MatType> d;

  // Calculate the actual mean and covariance of data using armadillo.
  VecType actualMean = arma::mean(observations, 1);
  MatType actualCov = ColumnCovariance(observations);

  // Estimate the parameters.
  d.Train(observations);

  // Check that the estimated parameters are right.
  for (size_t i = 0; i < 4; ++i)
  {
    REQUIRE(d.Mean()(i) - actualMean(i) == Approx(0.0).margin(tol));
    REQUIRE(d.Covariance()(i) - actualCov(i, i) == Approx(0.0).margin(tol));
  }
}

/**
 * Make sure the unbiased estimator of the weighted sample works correctly.
 * The values were calculated using 'cov.wt' in R.
 */
TEMPLATE_TEST_CASE("DiagonalGaussianUnbiasedEstimatorTest",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  const ElemType tol = (std::is_same<ElemType, float>::value) ? 1e-4 : 1e-7;

  // Generate the observations.
  MatType observations("3 5 2 7;"
                       "2 6 8 3;"
                       "1 4 2 7;"
                       "6 8 4 7");

  VecType probs("0.3 0.4 0.1 0.2");

  DiagonalGaussianDistribution<MatType> d;

  // Estimate the parameters.
  d.Train(observations, probs);

  REQUIRE(d.Mean()(0) == Approx(4.5).epsilon(tol));
  REQUIRE(d.Mean()(1) == Approx(4.4).epsilon(tol));
  REQUIRE(d.Mean()(2) == Approx(3.5).epsilon(tol));
  REQUIRE(d.Mean()(3) == Approx(6.8).epsilon(tol));

  REQUIRE(d.Covariance()(0) == Approx(3.78571428571428603).epsilon(tol));
  REQUIRE(d.Covariance()(1) == Approx(6.34285714285714253).epsilon(tol));
  REQUIRE(d.Covariance()(2) == Approx(6.64285714285714235).epsilon(tol));
  REQUIRE(d.Covariance()(3) == Approx(2.22857142857142865).epsilon(tol));
}

/**
 * Make sure that if all weights are the same, i.e. w_i / V1 = 1 / N, then
 * the weighted mean and covariance reduce to the unweighted sample mean and
 * covariance.
 */
TEMPLATE_TEST_CASE("DiagonalGaussianWeightedParametersReductionTest",
    "[DistributionTest]", float, double)
{
  typedef TestType ElemType;
  typedef typename arma::Col<ElemType> VecType;
  typedef typename arma::Mat<ElemType> MatType;

  const ElemType tol = (std::is_same<ElemType, float>::value) ? 1e-4 : 1e-7;

  VecType mean("2.5 1.5 8.2 3.1");
  VecType cov("1.2 3.1 8.3 4.3");

  // Generate the observations.
  MatType obs(4, 5);
  VecType probs("0.2 0.2 0.2 0.2 0.2");

  for (size_t i = 0; i < 5; ++i)
    obs.col(i) = (sqrt(cov) % arma::randn<VecType>(4)) + mean;

  DiagonalGaussianDistribution<MatType> d1;
  DiagonalGaussianDistribution<MatType> d2;

  // Estimate the parameters.
  d1.Train(obs);
  d2.Train(obs, probs);

  // Check if these are equal.
  for (size_t i = 0; i < 4; ++i)
  {
    REQUIRE(d1.Mean()(i) == Approx(d2.Mean()(i)).epsilon(tol));
    REQUIRE(d1.Covariance()(i) == Approx(d2.Covariance()(i)).epsilon(tol));
  }
}
