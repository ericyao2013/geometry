// Boost.Geometry - gis-projections (based on PROJ4)

// Copyright (c) 2008-2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017, 2018.
// Modifications copyright (c) 2017-2018, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This file is converted from PROJ4, http://trac.osgeo.org/proj
// PROJ4 is originally written by Gerald Evenden (then of the USGS)
// PROJ4 is maintained by Frank Warmerdam
// PROJ4 is converted to Boost.Geometry by Barend Gehrels

// Last updated version of proj: 5.0.0

// Original copyright notice:

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef BOOST_GEOMETRY_PROJECTIONS_LSAT_HPP
#define BOOST_GEOMETRY_PROJECTIONS_LSAT_HPP

#include <boost/geometry/util/math.hpp>

#include <boost/geometry/srs/projections/impl/base_static.hpp>
#include <boost/geometry/srs/projections/impl/base_dynamic.hpp>
#include <boost/geometry/srs/projections/impl/projects.hpp>
#include <boost/geometry/srs/projections/impl/factory_entry.hpp>
#include <boost/geometry/srs/projections/impl/aasincos.hpp>

namespace boost { namespace geometry
{

namespace srs { namespace par4
{
    struct lsat {}; // Space oblique for LANDSAT

}} //namespace srs::par4

namespace projections
{
    #ifndef DOXYGEN_NO_DETAIL
    namespace detail { namespace lsat
    {
            static const double TOL = 1e-7;

            template <typename T>
            struct par_lsat
            {
                T a2, a4, b, c1, c3;
                T q, t, u, w, p22, sa, ca, xj, rlm, rlm2;
            };

            /* based upon Snyder and Linck, USGS-NMD */
            template <typename T>
            inline void
            seraz0(T lam, T const& mult, par_lsat<T>& proj_parm)
            {
                T sdsq, h, s, fc, sd, sq, d__1 = 0;

                lam *= geometry::math::d2r<T>();
                sd = sin(lam);
                sdsq = sd * sd;
                s = proj_parm.p22 * proj_parm.sa * cos(lam) * sqrt((1. + proj_parm.t * sdsq)
                    / ((1. + proj_parm.w * sdsq) * (1. + proj_parm.q * sdsq)));

                d__1 = 1. + proj_parm.q * sdsq;
                h = sqrt((1. + proj_parm.q * sdsq) / (1. + proj_parm.w * sdsq)) * ((1. + proj_parm.w * sdsq)
                    / (d__1 * d__1) - proj_parm.p22 * proj_parm.ca);

                sq = sqrt(proj_parm.xj * proj_parm.xj + s * s);
                fc = mult * (h * proj_parm.xj - s * s) / sq;
                proj_parm.b += fc;
                proj_parm.a2 += fc * cos(lam + lam);
                proj_parm.a4 += fc * cos(lam * 4.);
                fc = mult * s * (h + proj_parm.xj) / sq;
                proj_parm.c1 += fc * cos(lam);
                proj_parm.c3 += fc * cos(lam * 3.);
            }

            // template class, using CRTP to implement forward/inverse
            template <typename CalculationType, typename Parameters>
            struct base_lsat_ellipsoid : public base_t_fi<base_lsat_ellipsoid<CalculationType, Parameters>,
                     CalculationType, Parameters>
            {

                typedef CalculationType geographic_type;
                typedef CalculationType cartesian_type;

                par_lsat<CalculationType> m_proj_parm;

                inline base_lsat_ellipsoid(const Parameters& par)
                    : base_t_fi<base_lsat_ellipsoid<CalculationType, Parameters>,
                     CalculationType, Parameters>(*this, par) {}

                // FORWARD(e_forward)  ellipsoid
                // Project coordinates from geographic (lon, lat) to cartesian (x, y)
                inline void fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y) const
                {
                    static const CalculationType FORTPI = detail::FORTPI<CalculationType>();
                    static const CalculationType HALFPI = detail::HALFPI<CalculationType>();
                    static const CalculationType PI_HALFPI = detail::PI_HALFPI<CalculationType>();
                    static const CalculationType TWOPI_HALFPI = detail::TWOPI_HALFPI<CalculationType>();

                    int l, nn;
                    CalculationType lamt = 0.0, xlam, sdsq, c, d, s, lamdp = 0.0, phidp, lampp, tanph;
                    CalculationType lamtp, cl, sd, sp, sav, tanphi;

                    if (lp_lat > HALFPI)
                        lp_lat = HALFPI;
                    else if (lp_lat < -HALFPI)
                        lp_lat = -HALFPI;

                    if (lp_lat >= 0. )
                        lampp = HALFPI;
                    else
                        lampp = PI_HALFPI;
                    tanphi = tan(lp_lat);
                    for (nn = 0;;) {
                        CalculationType fac;
                        sav = lampp;
                        lamtp = lp_lon + this->m_proj_parm.p22 * lampp;
                        cl = cos(lamtp);
                        if (fabs(cl) < TOL)
                            lamtp -= TOL;
                        if( cl < 0 )
                            fac = lampp + sin(lampp) * HALFPI;
                        else
                            fac = lampp - sin(lampp) * HALFPI;
                        for (l = 50; l; --l) {
                            lamt = lp_lon + this->m_proj_parm.p22 * sav;
                            c = cos(lamt);
                            if (fabs(c) < TOL)
                                lamt -= TOL;
                            xlam = (this->m_par.one_es * tanphi * this->m_proj_parm.sa + sin(lamt) * this->m_proj_parm.ca) / c;
                            lamdp = atan(xlam) + fac;
                            if (fabs(fabs(sav) - fabs(lamdp)) < TOL)
                                break;
                            sav = lamdp;
                        }
                        if (!l || ++nn >= 3 || (lamdp > this->m_proj_parm.rlm && lamdp < this->m_proj_parm.rlm2))
                            break;
                        if (lamdp <= this->m_proj_parm.rlm)
                            lampp = TWOPI_HALFPI;
                        else if (lamdp >= this->m_proj_parm.rlm2)
                            lampp = HALFPI;
                    }
                    if (l) {
                        sp = sin(lp_lat);
                        phidp = aasin((this->m_par.one_es * this->m_proj_parm.ca * sp - this->m_proj_parm.sa * cos(lp_lat) *
                            sin(lamt)) / sqrt(1. - this->m_par.es * sp * sp));
                        tanph = log(tan(FORTPI + .5 * phidp));
                        sd = sin(lamdp);
                        sdsq = sd * sd;
                        s = this->m_proj_parm.p22 * this->m_proj_parm.sa * cos(lamdp) * sqrt((1. + this->m_proj_parm.t * sdsq)
                             / ((1. + this->m_proj_parm.w * sdsq) * (1. + this->m_proj_parm.q * sdsq)));
                        d = sqrt(this->m_proj_parm.xj * this->m_proj_parm.xj + s * s);
                        xy_x = this->m_proj_parm.b * lamdp + this->m_proj_parm.a2 * sin(2. * lamdp) + this->m_proj_parm.a4 *
                            sin(lamdp * 4.) - tanph * s / d;
                        xy_y = this->m_proj_parm.c1 * sd + this->m_proj_parm.c3 * sin(lamdp * 3.) + tanph * this->m_proj_parm.xj / d;
                    } else
                        xy_x = xy_y = HUGE_VAL;
                }

                // INVERSE(e_inverse)  ellipsoid
                // Project coordinates from cartesian (x, y) to geographic (lon, lat)
                inline void inv(cartesian_type& xy_x, cartesian_type& xy_y, geographic_type& lp_lon, geographic_type& lp_lat) const
                {
                    static const CalculationType FORTPI = detail::FORTPI<CalculationType>();
                    static const CalculationType HALFPI = detail::HALFPI<CalculationType>();

                    int nn;
                    CalculationType lamt, sdsq, s, lamdp, phidp, sppsq, dd, sd, sl, fac, scl, sav, spp;

                    lamdp = xy_x / this->m_proj_parm.b;
                    nn = 50;
                    do {
                        sav = lamdp;
                        sd = sin(lamdp);
                        sdsq = sd * sd;
                        s = this->m_proj_parm.p22 * this->m_proj_parm.sa * cos(lamdp) * sqrt((1. + this->m_proj_parm.t * sdsq)
                             / ((1. + this->m_proj_parm.w * sdsq) * (1. + this->m_proj_parm.q * sdsq)));
                        lamdp = xy_x + xy_y * s / this->m_proj_parm.xj - this->m_proj_parm.a2 * sin(
                            2. * lamdp) - this->m_proj_parm.a4 * sin(lamdp * 4.) - s / this->m_proj_parm.xj * (
                            this->m_proj_parm.c1 * sin(lamdp) + this->m_proj_parm.c3 * sin(lamdp * 3.));
                        lamdp /= this->m_proj_parm.b;
                    } while (fabs(lamdp - sav) >= TOL && --nn);
                    sl = sin(lamdp);
                    fac = exp(sqrt(1. + s * s / this->m_proj_parm.xj / this->m_proj_parm.xj) * (xy_y -
                        this->m_proj_parm.c1 * sl - this->m_proj_parm.c3 * sin(lamdp * 3.)));
                    phidp = 2. * (atan(fac) - FORTPI);
                    dd = sl * sl;
                    if (fabs(cos(lamdp)) < TOL)
                        lamdp -= TOL;
                    spp = sin(phidp);
                    sppsq = spp * spp;
                    lamt = atan(((1. - sppsq * this->m_par.rone_es) * tan(lamdp) *
                        this->m_proj_parm.ca - spp * this->m_proj_parm.sa * sqrt((1. + this->m_proj_parm.q * dd) * (
                        1. - sppsq) - sppsq * this->m_proj_parm.u) / cos(lamdp)) / (1. - sppsq
                        * (1. + this->m_proj_parm.u)));
                    sl = lamt >= 0. ? 1. : -1.;
                    scl = cos(lamdp) >= 0. ? 1. : -1;
                    lamt -= HALFPI * (1. - scl) * sl;
                    lp_lon = lamt - this->m_proj_parm.p22 * lamdp;
                    if (fabs(this->m_proj_parm.sa) < TOL)
                        lp_lat = aasin(spp / sqrt(this->m_par.one_es * this->m_par.one_es + this->m_par.es * sppsq));
                    else
                        lp_lat = atan((tan(lamdp) * cos(lamt) - this->m_proj_parm.ca * sin(lamt)) /
                            (this->m_par.one_es * this->m_proj_parm.sa));
                }

                static inline std::string get_name()
                {
                    return "lsat_ellipsoid";
                }

            };

            // Space oblique for LANDSAT
            template <typename Parameters, typename T>
            inline void setup_lsat(Parameters& par, par_lsat<T>& proj_parm)
            {
                static T const DEG_TO_RAD = geometry::math::d2r<T>();
                static T const ONEPI = detail::ONEPI<T>();
                static T const TWOPI = detail::TWOPI<T>();

                int land, path;
                T lam, alf, esc, ess;

                land = pj_param(par.params, "ilsat").i;
                if (land <= 0 || land > 5)
                    BOOST_THROW_EXCEPTION( projection_exception(-28) );

                path = pj_param(par.params, "ipath").i;
                if (path <= 0 || path > (land <= 3 ? 251 : 233))
                    BOOST_THROW_EXCEPTION( projection_exception(-29) );

                if (land <= 3) {
                    par.lam0 = DEG_TO_RAD * 128.87 - TWOPI / 251. * path;
                    proj_parm.p22 = 103.2669323;
                    alf = DEG_TO_RAD * 99.092;
                } else {
                    par.lam0 = DEG_TO_RAD * 129.3 - TWOPI / 233. * path;
                    proj_parm.p22 = 98.8841202;
                    alf = DEG_TO_RAD * 98.2;
                }
                proj_parm.p22 /= 1440.;
                proj_parm.sa = sin(alf);
                proj_parm.ca = cos(alf);
                if (fabs(proj_parm.ca) < 1e-9)
                    proj_parm.ca = 1e-9;
                esc = par.es * proj_parm.ca * proj_parm.ca;
                ess = par.es * proj_parm.sa * proj_parm.sa;
                proj_parm.w = (1. - esc) * par.rone_es;
                proj_parm.w = proj_parm.w * proj_parm.w - 1.;
                proj_parm.q = ess * par.rone_es;
                proj_parm.t = ess * (2. - par.es) * par.rone_es * par.rone_es;
                proj_parm.u = esc * par.rone_es;
                proj_parm.xj = par.one_es * par.one_es * par.one_es;
                proj_parm.rlm = ONEPI * (1. / 248. + .5161290322580645);
                proj_parm.rlm2 = proj_parm.rlm + TWOPI;
                proj_parm.a2 = proj_parm.a4 = proj_parm.b = proj_parm.c1 = proj_parm.c3 = 0.;
                seraz0(0., 1., proj_parm);
                for (lam = 9.; lam <= 81.0001; lam += 18.)
                    seraz0(lam, 4., proj_parm);
                for (lam = 18; lam <= 72.0001; lam += 18.)
                    seraz0(lam, 2., proj_parm);
                seraz0(90., 1., proj_parm);
                proj_parm.a2 /= 30.;
                proj_parm.a4 /= 60.;
                proj_parm.b /= 30.;
                proj_parm.c1 /= 15.;
                proj_parm.c3 /= 45.;
            }

    }} // namespace detail::lsat
    #endif // doxygen

    /*!
        \brief Space oblique for LANDSAT projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Cylindrical
         - Spheroid
         - Ellipsoid
        \par Projection parameters
         - lsat (integer)
         - path (integer)
        \par Example
        \image html ex_lsat.gif
    */
    template <typename CalculationType, typename Parameters>
    struct lsat_ellipsoid : public detail::lsat::base_lsat_ellipsoid<CalculationType, Parameters>
    {
        inline lsat_ellipsoid(const Parameters& par) : detail::lsat::base_lsat_ellipsoid<CalculationType, Parameters>(par)
        {
            detail::lsat::setup_lsat(this->m_par, this->m_proj_parm);
        }
    };

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {

        // Static projection
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(srs::par4::lsat, lsat_ellipsoid, lsat_ellipsoid)

        // Factory entry(s)
        template <typename CalculationType, typename Parameters>
        class lsat_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<lsat_ellipsoid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        inline void lsat_init(detail::base_factory<CalculationType, Parameters>& factory)
        {
            factory.add_to_factory("lsat", new lsat_entry<CalculationType, Parameters>);
        }

    } // namespace detail
    #endif // doxygen

} // namespace projections

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_PROJECTIONS_LSAT_HPP

