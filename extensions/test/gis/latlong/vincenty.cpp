// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2014.
// Modifications copyright (c) 2014 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>

#include <boost/concept_check.hpp>

#include <boost/geometry/extensions/gis/geographic/strategies/vincenty.hpp>

#include <boost/geometry/core/srs.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <test_common/test_point.hpp>

#ifdef HAVE_TTMATH
#  include <boost/geometry/extensions/contrib/ttmath_stub.hpp>
#endif



template <typename P1, typename P2>
void test_vincenty(double lon1, double lat1, double lon2, double lat2,
                   double expected_km,
                   double expected_azimuth_12,
                   double expected_azimuth_21)
{
    // Set radius type, but for integer coordinates we want to have floating point radius type
    typedef typename bg::promote_floating_point
        <
            typename bg::coordinate_type<P1>::type
        >::type rtype;

    typedef bg::srs::spheroid<rtype> stype;

    // formula
    {
        bg::detail::vincenty_inverse<double> vi(lon1 * bg::math::d2r,
                                                lat1 * bg::math::d2r,
                                                lon2 * bg::math::d2r,
                                                lat2 * bg::math::d2r,
                                                stype());
        double dist = vi.distance();
        double az12 = vi.azimuth12();
        double az21 = vi.azimuth21();

        double az12_deg = az12 * bg::math::r2d;
        double az21_deg = az21 * bg::math::r2d;

        BOOST_CHECK_CLOSE(dist, 1000.0 * expected_km, 0.001);
        BOOST_CHECK_CLOSE(az12_deg, expected_azimuth_12, 0.001);
        //BOOST_CHECK_CLOSE(az21_deg, expected_azimuth_21, 0.001);
    }

    // strategy
    {
        typedef bg::strategy::distance::vincenty<stype> vincenty_type;

        BOOST_CONCEPT_ASSERT(
            (
                bg::concept::PointDistanceStrategy<vincenty_type, P1, P2>)
            );

        vincenty_type vincenty;
        typedef typename bg::strategy::distance::services::return_type<vincenty_type, P1, P2>::type return_type;

        P1 p1, p2;

        bg::assign_values(p1, lon1, lat1);
        bg::assign_values(p2, lon2, lat2);

        BOOST_CHECK_CLOSE(vincenty.apply(p1, p2), return_type(1000.0) * return_type(expected_km), 0.001);
    }
}

template <typename P1, typename P2>
void test_all()
{
    // See:
    //  - http://www.ga.gov.au/geodesy/datums/vincenty_inverse.jsp
    //  - http://www.ga.gov.au/geodesy/datums/vincenty_direct.jsp
    // Values in the comments below was calculated using the above pages
    // in some cases distances may be different, previously used values was left

    test_vincenty<P1, P2>(0, 0, 0, 50, 5540.847042, 0, 180); // N
    test_vincenty<P1, P2>(0, 0, 0, -50, 5540.847042, 180, 0); // S
    test_vincenty<P1, P2>(0, 0, 50, 0, 	5565.974540, 90, -90); // E
    test_vincenty<P1, P2>(0, 0, -50, 0, 5565.974540, -90, 90); // W
    // 7284.879297, 32deg 51' 55.87'', 237deg 24' 50.12''
    test_vincenty<P1, P2>(0, 0, 50, 50, 7284.879297, 32+51.0/60+55.87/3600, 237+24.060+50.12/3600-360); // NE

    // 1005.150875, 178deg 53' 23.85'', 359deg 53' 18.35''
    test_vincenty<P1, P2>(0, 89, 1, 80, 1005.1535769, 178+53.0/60+23.85/3600, 359+53.0/60+18.35/3600-360); // sub-polar

    test_vincenty<P1, P2>(4, 52, 4, 52, 0.0, 0, 0); // no point difference

    // 1336.027219, 183deg 41' 29.08'', 2deg 58' 5.13''
    test_vincenty<P1, P2>(4, 52, 3, 40, 1336.039890, 183+41.0/60+29.08/3600-360, 2+58.0/60+5.13/3600); // normal case
}

template <typename P>
void test_all()
{
    test_all<P, P>();
}

int test_main(int, char* [])
{

    //test_all<float[2]>();
    //test_all<double[2]>();
    test_all<bg::model::point<int, 2, bg::cs::geographic<bg::degree> > >();
    test_all<bg::model::point<float, 2, bg::cs::geographic<bg::degree> > >();
    test_all<bg::model::point<double, 2, bg::cs::geographic<bg::degree> > >();

#if defined(HAVE_TTMATH)
    test_all<bg::model::point<ttmath::Big<1,4>, 2, bg::cs::geographic<bg::degree> > >();
    test_all<bg::model::point<ttmath_big, 2, bg::cs::geographic<bg::degree> > >();
#endif


    return 0;
}
