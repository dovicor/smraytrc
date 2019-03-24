// Copyright (c) Don Organ 2018, 2019
// All rights reserved.

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <deque>
#include <set>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

namespace bg = boost::geometry;

const double My_PI = 3.141592653589793; /* Yes - I know this is defined in various header files (maybe cmath, math.h, or boost/math/constants.h), but
                                    * getting it actually included was dependent on setting other various #defines - and it became
                                    * difficult to get it all to build reliably on various platforms. So I gave up and defined it here.
                                    * (Now, let's debate how many digits I should have defined this to.)
                                    */

int dvo_debug = 0;
const double BadValue = 9.999e9;
const double SmallValue = 0.000001; // for use in tolerances, etc.


template <typename T> inline const T& Max(const T&arg1, const T&arg2) { return (arg1>arg2) ? arg1 : arg2; };
template <typename T> inline const T& Min(const T&arg1, const T&arg2) { return (arg1<arg2) ? arg1 : arg2; };

const char* Indent(unsigned level, unsigned spaces_per_level=4)
{
    static const char lots_of_spaces[] = "                                                                                                 " // no comma
                                            "                                                                                                 ";
    static const char* end_of_spaces = lots_of_spaces + sizeof(lots_of_spaces)-1; // points to '\0' at end
    const char* start = end_of_spaces - (spaces_per_level * level);
    return (start < lots_of_spaces) ? lots_of_spaces : start;
}


// Started with ConvexMirror/try16.cpp - and modified for the Concave system

/* A 2 component optic system - a sun (at infinite distance and a finite
 * angular width), and a Concave mirror (no observer - as was in the ConvexMirror system).
 *
 * My conventions...
 * ..._ang = value is an angle (in degrees) - between two lines (or points, etc.)
 * ..._dir = value is a direction (in degrees) in the cartesian coordinates - i.e. 0 degrees
 *     is horizontal to the right, 90 degree is straight up, 180 is horizontal to the
 *     left and 270 is straight down. 
 * Center of the curvature of the Mirror is at (0,0)
 *
 * Standard Trig Quadrants...
 *          |
 *        2 | 1
 *      ----+----
 *        3 | 4
 *          |
 * But this is NOT my convention here.
 *
 *
 * Inputs (all angles in degrees):
 * r=radius of convex mirror whose Center of curvature is at (0,0)
 * sa=altitude of the center-point of the sun - actually the direction of a ray from the sun. (0=horizontal and to the right)
 * sw=angular width of the sun. Default=0.5
 *
 * Calculates:
 * two series of rays - one series from the 'top' of the sun, and the other from the 'bottom'
 *     of the sun. An incident ray strikes the Mirror and generates a reflected ray.
 *     The series if for various parallel rays that strike the sun at different locations.
 *
 */

double to_degrees(double radians) { return radians * (180.0 / My_PI); }
double to_radians(double degrees) { return degrees / (180.0 / My_PI); }
double NormalizeAngle(double degrees) /* Adjust to between 0 and 360 degrees -
                    * yes I find this name confusing - since 'normal' often means perpendicular to a surface,
                    * and that is NOT what it means here.
                    */
{
       while(degrees >= 360) degrees -= 360;
       while (degrees < 0) degrees += 360;
       return degrees;
}
double MinAngle(double degrees) /* Similar to NormalizeAngle() - but adjust to between 0 and 180 degrees */
{
       while(degrees >= 180) degrees -= 180;
       while (degrees < 0) degrees += 180;
       return degrees;
}

int RayStrikeConcave(double ray_dir, double normal_dir)
    // A ray reaches the surface of a circle - is the ray approaching from the inside (concave
    // side) or convex side? Concave=true, Convex=false;
{
    double ray_dir_0_360 = NormalizeAngle( ray_dir );
    double adjusted_normal_dir = normal_dir;
    while ( ray_dir_0_360 > (adjusted_normal_dir + 180) ) adjusted_normal_dir += 360;
    while ( ray_dir_0_360 < (adjusted_normal_dir - 180) ) adjusted_normal_dir -= 360;
    bool return_value = ( (ray_dir_0_360 < (adjusted_normal_dir + 90)) && (ray_dir_0_360 > (adjusted_normal_dir - 90)) ) ? 1 : 0;
//printf("\t\t\t%s(ray=%g, normal=%g): ray_dir_0_360=%g, adjusted_normal=%g, return_value=%d\n", __func__, ray_dir, normal_dir, ray_dir_0_360, adjusted_normal_dir, return_value);
    return return_value;
}

const double NearlyEqual_default1 = SmallValue;
const double NearlyEqual_default2 = SmallValue;
bool NearlyEqual(double f1, double f2, double multiply_tolerance=NearlyEqual_default1, double additive_tolerance=NearlyEqual_default2)
{
    if (f1 == f2) return true;
    double difference = f1-f2;
    double max_value, min_value;
    if (fabs(f1) > fabs(f2)) {
        max_value = fabs(f1);
        min_value = fabs(f2);
    } else {
        max_value = fabs(f2);
        min_value = fabs(f1);
    }
    if (multiply_tolerance != 0) {
        if (max_value/multiply_tolerance < min_value) {
            if (dvo_debug>=10)
            printf("Fails: NearlyEqual(%g,%g, %g,%g): fails multiply tolerance: %g/%g=%g < %g at %d of %s\n",
                    f1,f2, multiply_tolerance, additive_tolerance, max_value, multiply_tolerance, min_value, __LINE__, __FILE__ );
            return false;
        }
    }
    if (additive_tolerance != 0) {
        if ((max_value - min_value) > additive_tolerance) {
            if (dvo_debug>=10)
            printf("Fails: NearlyEqual(%g,%g, %g,%g): fails additive tolerance: %g-%g=%g > %g at %d of %s\n",
                    f1,f2, multiply_tolerance, additive_tolerance, max_value, min_value, max_value-min_value, additive_tolerance, __LINE__, __FILE__ );
            return false;
        }
    }
    return true;
}

typedef bg::model::d2::point_xy<double>  Point;
typedef bg::model::segment<Point> Segment;

static std::deque<Segment> debug_segments;

void AddDebugSegment(const Segment&seg)
{
    debug_segments.push_back( seg );
}
void AddDebugSegment(const Point& pt, double ray_dir, double length=30)
{
    Point Find2ndPoint(const Point&, double direction, double distance);
    Point pt_b = Find2ndPoint(pt, ray_dir, length );
    debug_segments.push_back( Segment(pt,pt_b) );
}


const Point& Set( Point& pt, double new_x, double new_y)
{
    pt.x( new_x );
    pt.y( new_y );
    return pt;
}

bool operator==(const Point& pt1, const Point& pt2)
{
    return ((pt1.x() == pt2.x()) && (pt1.y() == pt2.y())) ? 1 : 0;
}
bool Defined(const Point& pt)
{
    return (pt.x() != BadValue) && (pt.y() != BadValue);
}

bool NearlyEqual(const Point& p1, const Point& p2, double multiply_tolerance=SmallValue, double additive_tolerance=SmallValue)
{
    return NearlyEqual(p1.x(), p2.x(),multiply_tolerance, additive_tolerance) && NearlyEqual(p1.y(), p2.y(),multiply_tolerance, additive_tolerance);
}


double Distance(const Point& pt1, const Point& pt2)
{
    assert(pt1.x() != BadValue);
    assert(pt1.y() != BadValue);
    assert(pt2.x() != BadValue);
    assert(pt2.y() != BadValue);
    double new_distance = bg::distance(pt1,pt2);
    return new_distance;
}

double Direction(const Point& pt1, const Point& pt2) // the direction from pt1 to pt2 - in degrees.
{
    return NormalizeAngle(to_degrees( atan2( pt2.y()-pt1.y(), pt2.x()-pt1.x() )));
}

Point Find2ndPoint(const Point& pt1, double direction, double distance)
{
    double X = pt1.x() + distance * cos( to_radians( direction ) );
    double Y = pt1.y() + distance * sin( to_radians( direction ) );
    return Point(X,Y);
}

Point Closest(const Point& from_pt, double direction, const Point& pt1, const Point& pt2)
    // Assume the 3 points are co-linear. Which of pt1 or pt2 is closest to from_ptr
    // (but also in the direction indicated)?
{
    assert( from_pt.x() != BadValue);
    assert( from_pt.y() != BadValue);
    assert( pt1.x() != BadValue);
    assert( pt1.y() != BadValue);
    assert( pt2.x() != BadValue);
    assert( pt2.y() != BadValue);
    double distance1 = Distance(from_pt, pt1);
    double distance2 = Distance(from_pt, pt2);
    return (distance1 < distance2) ? pt1 : pt2 ;
}


int Intersection_2Segments(const Segment& seg1, const Segment& seg2, Point& XsectionPt, bool include_end_points=true,
                        double ne_param1=NearlyEqual_default1, double ne_param2=NearlyEqual_default2)
{
    std::vector<Point> geometry_out;
    bool bogus = bg::intersection(seg1,seg2,geometry_out);
    if ( geometry_out.size() ) {
        XsectionPt = geometry_out[0];
        if (include_end_points == false) {
            if (NearlyEqual( seg1.first.x(), XsectionPt.x(), ne_param1, ne_param2) && NearlyEqual(seg1.first.y(), XsectionPt.y(), ne_param1, ne_param2) ) return 0;
            if (NearlyEqual( seg1.first.x(), XsectionPt.x(), ne_param1, ne_param2) && NearlyEqual(seg1.second.y(), XsectionPt.y(), ne_param1, ne_param2) ) return 0;
        }
        return 1;
    }
    return 0;
}


void TerminateRay(const Segment& Ray, const Segment& LineB, Point &closest_far_point)
    /* Ray and LineB are each line segments. Ray represents a light-ray originating at Ray_Pt1 and possibly terminating at Ray_Pt2.
     * LineB represents an opaque surface that possibly crosses the path of the Ray.
     * Detemine if Ray hits LineB and, if so, if it hits it before it hits Ray_Pt2
     * In either case, closest_far_point is set to the termination point (maybe the original Ray_Pt2, or maybe a point on the LineB segment).
     */
{
//printf("%s(Ray=(%g,%g)..(%g,%g), LineB=(%g,%g)..(%g,%g),...)\n", __func__, Ray.first.x(), Ray.first.y(), Ray.second.x(), Ray.second.y(), LineB.first.x(), LineB.first.y(), LineB.second.x(), LineB.second.y() );
    closest_far_point = Ray.second; // May get changed below.
    Point intersection_pt;
    int has_intersection = Intersection_2Segments( LineB, Ray, intersection_pt, false );
    if (has_intersection) {
        assert( Ray.first.x() != BadValue);
        assert( Ray.first.y() != BadValue);
        assert( Ray.second.x() != BadValue);
        assert( Ray.second.y() != BadValue);
        assert( intersection_pt.x() != BadValue);
        assert( intersection_pt.y() != BadValue);
        double distance1 = Distance( Ray.first, Ray.second );
        double distance2 = Distance( Ray.first, intersection_pt );
//printf("\tHas Intersection: distance1=%g, distance2=%g\n", distance1, distance2);
        if (distance1 > distance2)
            closest_far_point = intersection_pt;
    }
}


int ProjectPointOntoCircle(const Point& from_pt, double direction, const Point& cir_center, double radius, Point& pt1, Point& pt2)
    /* Project a ray in direction onto the circle.
     * Returns 0, 1 or 2 - the number of times the ray intersects with the circle.
     * If return==0, then neither pt1 or pt2 is altered.
     * If return==1, then pt1= the point of intersection, and pt2 is unaltered.
     * If return==2, then pt1 and pt2 are both set.
     */ 
{
    assert( from_pt.x() != BadValue);
    assert( from_pt.y() != BadValue);
    assert( cir_center.x() != BadValue);
    assert( cir_center.y() != BadValue);
    double distance_from_center = Distance( from_pt, cir_center );
    double center_to_from_dir = Direction( cir_center, from_pt ); // direction in degrees
    double interior_angle_at_from = direction - center_to_from_dir;
    double back_edge = distance_from_center * sin( to_radians( interior_angle_at_from ) );
    double angle_shadow_pt_to_back_edge = to_degrees( asin( back_edge / radius ) );
    double direction_center_to_shadow_pt = direction - angle_shadow_pt_to_back_edge;
    double shadow_X = cir_center.x() + radius * cos( to_radians( direction_center_to_shadow_pt ) );
    double shadow_Y = cir_center.y() + radius * sin( to_radians( direction_center_to_shadow_pt ) );

    pt1.x( shadow_X );
    pt1.y( shadow_Y );

    debug_segments.push_back( Segment( from_pt, pt1 ) );
    return 1;
}

double ApparentWidth( const Point&p1, const Point&p2, double from_this_angle)
  // Given two points (via cartesian coordinates), and an observer at infinite
  // distance - how far apart do the points appear?
{
    double delta_X = p1.x() - p2.x();
    double delta_Y = p1.y() - p2.y();
    double hypotenuse_length = sqrt( delta_X*delta_X + delta_Y*delta_Y ); // length between the points
//    double hypotenuse_angle_radians = (delta_X == 0) ? 0 : atan( delta_Y / delta_X );
    double hypotenuse_angle_radians = atan2( delta_Y, delta_X );
    double   observer_angle_radians = to_radians(from_this_angle);
    double delta_radians = hypotenuse_angle_radians - observer_angle_radians;
    double apparent_height = hypotenuse_length * sin( delta_radians );
    return fabs(apparent_height);
}

double ApparentWidth_ang( const Point&p1, const Point&p2, const Point&observer)
    // Similar to ApparentWidth() - but reports in apparent angle (in degrees)
{
    double dir1 = Direction( observer, p1 );
    double dir2 = Direction( observer, p2 );
    return MinAngle( fabs(dir1 - dir2) );
}


struct BBox
{
    BBox() : min_pt(BadValue,BadValue), max_pt(BadValue,BadValue) {};

    void Update(const Point& new_pt) {
               if ((new_pt.x() > max_pt.x()) || (max_pt.x() == BadValue)) max_pt.x( new_pt.x() );
               if ((new_pt.y() > max_pt.y()) || (max_pt.y() == BadValue)) max_pt.y( new_pt.y() );
               if ((new_pt.x() < min_pt.x()) || (min_pt.x() == BadValue)) min_pt.x( new_pt.x() );
               if ((new_pt.y() < min_pt.y()) || (min_pt.y() == BadValue)) min_pt.y( new_pt.y() );
    }

    double MaxX() const { return max_pt.x(); }
    double MaxY() const { return max_pt.y(); }
    double MinX() const { return min_pt.x(); }
    double MinY() const { return min_pt.y(); }

    double MidX() const { return (max_pt.x() == BadValue) ? BadValue : (max_pt.x() + min_pt.x()) / 2; }
    double MidY() const { return (max_pt.y() == BadValue) ? BadValue : (max_pt.y() + min_pt.y()) / 2; }

    double Diagonal() const { return Defined() ? Distance( min_pt, max_pt ) : BadValue; }

    bool Defined() const { return ::Defined(min_pt) && ::Defined(max_pt); }

    Point min_pt;
    Point max_pt;
};

struct TracedRay
    // For forward-tracing one ray and its interactions with a Concave mirror surface.
    // The mirror surface is a portion of a circle (an arc).
{
    TracedRay() : m_sun_dir(BadValue), m_MirrorPt(BadValue,BadValue), m_ray_status(Unknown), m_reflect_dir(BadValue), m_StrikePts() {};

    enum RayStatus {Unknown,    // uninitialized - or not yet traced.
                    Convex,     // The incident ray reaches the Target on the Concave Mirror from the wrong side (outside - convex) of the  mirror.
                    Concave,    // The incident ray reaches the Target on the Concave Mirror from the expected side (inside - concave), but not further traced.
                    Obscured,   // Although Concave is true (above), the incident ray would have 1st crossed the mirror's arc (so would not reach the target).
                    NStrike,    // The ray strikes the concave side of the mirror, and its reflection also strikes the mirror (perhaps more than once),
                                // but eventually a reflected ray progresses beyond the mirror.
                    NStrikeOut, // Similar to NStrike - ray tracing stops before a ray escapes the mirror.
                    Unobscured  // The ray strikes the concave side of the mirror and is reflected (without again striking the mirror). Might hit a
                                // stencil or wall.
                   };

    void RayReport(FILE *fout=stdout, unsigned level=0) const;

    unsigned CountObscuredRays() const;


    double m_sun_dir; // Direction of incident ray
    Point m_MirrorPt; // The incident ray points here (but may not reach - depending on m_ray_status)
    RayStatus m_ray_status; /* Indicates status of the reflected ray - see comments for ConcaveRayCalculate(). */
    double m_reflect_dir; // Valid only if NStrike or Unobscured
    std::deque<Point> m_StrikePts; // Valid if NStrike - has only 2nd to Nth reflection points (i.e. not 1stStrikePt=m_MirrorPt)
                                    // One point *might* be valid if Unobscured.

};
static const char* Name(TracedRay::RayStatus rs)
{
    switch(rs) {
        case TracedRay::Unknown:    return "Unknown";
        case TracedRay::Convex:        return "Convex";
        case TracedRay::Concave:    return "Concave";
        case TracedRay::Obscured:    return "Obscured";
        case TracedRay::NStrike:    return "NStrike";
        case TracedRay::NStrikeOut:    return "NStrikeOut";
        case TracedRay::Unobscured:    return "Unobscured";
        default:            return "undefined";
    }
}

void TracedRay::RayReport(FILE *fout, unsigned level) const
{
    fprintf(fout, "%sIncidentRay=%g (deg), Mirror=(%g,%g) Status=%s", Indent(level), m_sun_dir, m_MirrorPt.x(), m_MirrorPt.y(), Name(m_ray_status));
    switch (m_ray_status) {
        case TracedRay::Unknown:     break;
        case TracedRay::Convex:      break;
        case TracedRay::Concave:     fprintf(fout, ", (not traced further)");    break;
        case TracedRay::Obscured:    fprintf(fout, ", 1st Strike=(%g,%g)", m_StrikePts.begin()->x(), m_StrikePts.begin()->y() ); break;
        case TracedRay::NStrike:     // fall thru
        case TracedRay::NStrikeOut:  // fall thru
        case TracedRay::Unobscured:  {
                                        int count = 0;
                                        fprintf(fout, ", ReflectDir=%g", m_reflect_dir );
                                        for (auto it= m_StrikePts.begin(); it != m_StrikePts.end(); ++it) {
                                            count++;
                                            if (count == 1) fprintf(fout, ", Strikes=");
                                            fprintf(fout, "(%g,%g) ", it->x(), it->y() );
                                        }
                                    }
                                    break;
        default:            break;
    }
    fprintf(fout,"\n");
}

unsigned TracedRay::CountObscuredRays() const
{
    return (m_ray_status <= Obscured) ? 1 : 0;
}


bool Intersection(const Point& pt1, double arg_dir1, const Point& pt2, double arg_dir2, Point &intersection_pt) // returns success
{
    if (pt1 == pt2) { intersection_pt = pt1; return true; } // avoids some special cases below
    double dir1 = NormalizeAngle( arg_dir1 );
    double dir2 = NormalizeAngle( arg_dir2 );
    if (dir1 == dir2) return false; // Could be true! If the lines are co-linear!
    if (dir1 == NormalizeAngle(dir2+180)) return false; // Could be true! If the lines are co-linear!

    bool bad_tan1 = ( (dir1 == 90) || (dir1 == 270) ) ? 1 : 0; // Tangent would explode
    bool bad_tan2 = ( (dir2 == 90) || (dir2 == 270) ) ? 1 : 0; // Tangent would explode

    if ( bad_tan1 ) {
        if (bad_tan2)
            fprintf(stderr,"%s((%g,%g),%g, (%g,%g),%g,...), unexpected case at %d of %s\n", __func__,
                       pt1.x(), pt1.y(), arg_dir1, pt2.x(), pt2.y(), arg_dir2, __LINE__, __FILE__);
        Set( intersection_pt, pt1.x(), pt2.y() + tan(to_radians(dir2)) * (pt1.x() - pt2.x()) );
        return true;
    } 
    if ( bad_tan2 ) {
        Set( intersection_pt, pt2.x(), pt1.y() + tan(to_radians(dir1)) * (pt2.x() - pt1.x()) );
        return true;
    }

    double tan1 = tan(to_radians( dir1 ));
    double tan2 = tan(to_radians( dir2 ));

    double X = (pt2.y() - pt1.y() + tan1 * pt1.x() - tan2 * pt2.x()) / (tan1-tan2);
    double Y = BadValue;
    if (X != pt1.x()) {
        Y = pt1.y() + tan1 * (X - pt1.x());
    } else if (X != pt2.x()){
        Y = pt2.y() + tan2 * (X - pt2.x());
    } else { // X == pt1.x() == pt2.x() - so must also have the same Ys - should have been caught for the pt1==pt2 above
        fprintf(stderr,"%s((%g,%g),%g, (%g,%g),%g,...), unexpected case at %d of %s\n", __func__,
                   pt1.x(), pt1.y(), arg_dir1, pt2.x(), pt2.y(), arg_dir2, __LINE__, __FILE__);
        return false;
    }

    Set(intersection_pt, X,Y);
    if (NearlyEqual( intersection_pt, pt1 )) return true;
    if (NearlyEqual( intersection_pt, pt2 )) return true;

    // Found and intersection point - now check if it is in the negative direction (i.e. before where the ray starts).
    double final_dir1 = Direction(pt1, intersection_pt);
    double final_dir2 = Direction(pt2, intersection_pt);

    if (     (!NearlyEqual( dir1, final_dir1 )) || (!NearlyEqual( dir2, final_dir2 )) ) {
        if(dvo_debug)
            printf("%s((%g,%g),dir1=%g, (%g,%g),dir2=%g), intersection=(%g,%g), final1=%g, final2=%g\n",
                   __func__, pt1.x(), pt1.y(), arg_dir1, pt2.x(), pt2.y(), arg_dir2, intersection_pt.x(), intersection_pt.y(), final_dir1, final_dir2);
        return false;
    }

    return true;
}


Point FindReflectPoint_Concave( const Point& MirrorCOC, double Radius, const Point& MirrorReflectPt, double Incident_dir)
    /* A reflected ray originates at MirrorReflectPt on the concave surface of a (semi-)circular mirror with center
     * of curvature at MirrorCOC. The ray will travel a distance across the interior and the cross the circle (again
     * striking the mirror if it extends that far). This routine determines the point where the ray would strike
     * the mirror.
     */
{
    double normal_dir = Direction(MirrorCOC,MirrorReflectPt);
    double tangent_dir = NormalizeAngle(normal_dir + 90);
    double inner_angle = NormalizeAngle(Incident_dir - tangent_dir); // angle from COC to MirrorReflectPt to Incident_dir
    double arc_length_deg = inner_angle *2;
    double second_normal_dir = NormalizeAngle(normal_dir + arc_length_deg);

    return Find2ndPoint(MirrorCOC, second_normal_dir, Radius );
}

bool NormalWithinArc(double arg_the_normal_dir, double arg_min_normal_dir, double arg_max_normal_dir)
    // Is the_normal_dir with [min_normal_dir,max_normal_dir]? (all are degrees)
    // The trick is our [0,360) scheme can break down a bit here. Consider:
    // the_normal_dir = 60, min_normal_dir=350, max_normal_dir=70. In this case
    // the_normal_dir<min_normal_dir, so a naive implementation might consider outside
    // the arc.
    // Here's my approach.
    // 1st normalize the_normal_dir and min_normal_dir i.e. adjust so that each is [0,360).
    // 2nd, if max_normal_dir < min_normal_dir then add 360 to max_normal_dir.
    // 3rd - do the comparison.
{
    // Step 1
    double the_normal_dir = NormalizeAngle(arg_the_normal_dir);
    double min_normal_dir = NormalizeAngle(arg_min_normal_dir); // So min_normal_dir is [0,360)
    double max_normal_dir = NormalizeAngle(arg_max_normal_dir);

    // Step 2
    if (the_normal_dir < min_normal_dir) the_normal_dir += 360; // So the_normal_dir is now [min_normal_dir,720)
    if (max_normal_dir < min_normal_dir) max_normal_dir += 360; // So max_normal_dir is now [min_normal_dir,720) 

    // Step 3
    bool result = ( (min_normal_dir <= the_normal_dir) && (the_normal_dir <= max_normal_dir) ) ? 1 : 0;

    return result;
}




double ConcaveRayCalculate (
        const Point& MirrorCOC, double Radius, double min_normal_dir, double max_normal_dir, // These args define the mirror's size and position
        double incident_dir, const Point& RayOriginPt, const Point& TargetPt,
        std::deque<Point>& StrikePts, TracedRay::RayStatus& ray_status  // These args are output arguments
        ) 
    /* Assume a concave mirror, with center-of-curvature and radius as per the first two arguments.
     * Assume the min/max normal_dirs point from the COC to either end of the mirror's surface (i.e. are surface normals at the
     *     ends of the arc representing the concave surface). So the surface is an arc.
     * Assume the incident ray is targeted for the TargetPt on the mirror. RayOriginPt is optional - doesn't make sense with the sun
     *      (which is at infinity). But is useful in reverse ray-tracing.
     *
     *
     * ConcaveRayCalculate(): Determine...
     * if Convex - then the ray is terminated at the first m_StrikePts - on the mirror's surface.
     * if Concave - then the ray is reflected and will be further traced.
     * if Obscured - The incident ray crossed the mirror surface first - between min and max normal_dirs. (If it did, then no
     *     no reflected ray is generated, and the incident ray is terminated at the point where it first struck the mirror's surface).
     * if NStrike - the reflected ray strikes the mirror surface again - m_StrikePts are set.
     * if Unobscured - then incident ray is reflected and extends beyond the mirror
     *  (As way of example - assume the concave mirror is in the shape of the letter C. If the incident_dir is from over-head and the TargetPt is at the
     *   top of the C, then that is the Convex situation.  If the incident dir is from the right and the TargetPt is on the left, then the ray would
     *   the C and hit the Concave surface. If the incident ray is from over-head and the TargetPt is at the bottom of the C, then that is Obscured.)
     */
{
    assert(Defined(MirrorCOC));
    if ( !Defined( TargetPt ) ) return BadValue;
    
    // Case 1 - does the ray (incident_dir) reach the TargetPt from the inside (concave) or outside?
    double normal_at_target = Direction( MirrorCOC, TargetPt );
    ray_status = RayStrikeConcave( incident_dir, normal_at_target ) ? TracedRay::Concave : TracedRay::Convex ; // RayStrikeConcave() returns 0 or 1
    if ( ray_status == TracedRay::Convex ) {
        StrikePts.push_back( TargetPt );
        return BadValue;
    }

    // Case 2 - does the ray (incident_dir) cross the mirror surface between the normal_dirs before it reaches the TargetPt?
    // Applicable only if the ray originates outside the radius of the mirror's surface
    if ( !Defined(RayOriginPt) || (Distance(RayOriginPt,MirrorCOC) > Radius) ) {
        Point potential_1stStrikePt = FindReflectPoint_Concave( MirrorCOC, Radius, TargetPt, NormalizeAngle( incident_dir + 180 ) );
        // potential_1stStrikePt is on the circle around MirrorCOC - but is that point between the normals delineating the arc?
        double potential_1stStrike_normal_dir = Direction( MirrorCOC, potential_1stStrikePt );
        if ( NormalWithinArc(potential_1stStrike_normal_dir, min_normal_dir, max_normal_dir) ) {
            StrikePts.push_back( potential_1stStrikePt );
            ray_status = TracedRay::Obscured;
            return BadValue;
        }
    }

    // Case 3 - a reflection is generated. Does the reflection again strike the arc?
    StrikePts.push_back( TargetPt );
    ray_status = TracedRay::Unobscured; // May get changed below
    double next_incident_dir = incident_dir;
    double reflect_dir = BadValue;

    const int loop_limit = 20;
    int loop_count = 0;
    while (1) { // follow reflections along the mirror surface
        loop_count++;
        double this_strike_normal = Direction( MirrorCOC, StrikePts.back() );
        reflect_dir = NormalizeAngle( next_incident_dir + 2*(this_strike_normal + -next_incident_dir) +180);
        Point next_potential_strike_pt = FindReflectPoint_Concave( MirrorCOC, Radius,  StrikePts.back(), reflect_dir );
        double next_potential_strike_normal = Direction( MirrorCOC, next_potential_strike_pt );
        if ( ! NormalWithinArc(next_potential_strike_normal, min_normal_dir, max_normal_dir) ) { break; }
        ray_status = TracedRay::NStrike;

        // Get here only if there is another reflection on the mirror
        StrikePts.push_back( next_potential_strike_pt );
        next_incident_dir = reflect_dir;

        if (loop_count >= loop_limit) {
            ray_status = TracedRay::NStrikeOut;
            break;
        }
    } // while 1

    // Case 4
//    ray_status = TracedRay::Unobscured;
    return reflect_dir;
}


class TheData { // Please come up with a better name
    public:
        // input data
        double m_radius;
        double m_sun_dir; // degrees
        double m_sun_width_ang; // degrees;

        
// Concave - the following fields are applicable to CONCAVE mirrors only
               // Limits a portion of the circle (i.e. arc-length)
        bool m_IsConvex; // true=Convex, false=Concave
        Point m_MirrorCOCPt; // mirror's center-of-curvature
        double m_min_normal_dir; // min/max normal directions define the limits of the ARC
        double m_max_normal_dir;

        Point m_min_normal_pt;
        Point m_max_normal_pt;
        Point m_MidArcPt;

        Segment m_screen;
        std::deque< Segment > m_stencils;
        std::list<Point> m_target_pts;

// Convex - the following fields are applicable to CONVEX mirrors only - DVO 12/16/2018: why is that a restriction?
        double m_distance; // from observer to mirror's COC
		Point m_ObserverPt;

        // Results (output data)
        //
        std::deque<TracedRay> m_TopRays; // indexed in steps from m_min_normal_dir to m_max_normal_dir.
        std::deque<TracedRay> m_BotRays;

        unsigned m_CountOfObscuredRays; // # of m_TopRays+m_BotRays whose reflected rays are invalid (see TracedRay::m_ray_status)

        std::deque<Point> m_TopIntersectionPts; // (N-1)squared - intersection points of the reflected Top rays
        std::deque<Point> m_BotIntersectionPts; // (N-1)squared - intersection points of the reflected Bot rays

        BBox m_TopIntersectionBBox; // bounding-box of all m_TopIntersectionPts
        BBox m_BotIntersectionBBox; // bounding-box of all m_BotIntersectionPts

        double m_reflected_rays_width_ang;
        double m_reflected_focal_distance;
        double m_reflected_blur; // a distance across the BBox

		// Tangent (light ray that reaches observer that skims the mirror)
		double m_ObserverTangentAng;  // from observer to tangent point. 0 is horizontal (>0 is up)
		double m_NormalTangentAng; // From center of mirror
		Point m_TangentPt; // On mirror


		double m_ObserverReflectedSunBot; // Sun's bottom
		double m_SunBotAng;
		Point m_SunBotMirrorPt;

		double m_ObserverReflectedSunMid; // Sun's middle
		double m_SunMidAng;
		Point m_SunMidMirrorPt;

		double m_ObserverReflectedSunTop;
		double m_SunTopAng;
		Point m_SunTopMirrorPt;

		double m_Pupil_Entrance; // Experimental: Entrance pupil - the physical separation between the sun's top/bottom rays at the mirror
		double m_Pupil_Exit; // Experimental: Entrance pupil - the physical separation between the sun's top/bottom rays at the mirror
		double m_Brightness;  // Experimental: relative to direct sun's intensity  - uses ratio of pupils (entrance/exit).
		double m_Brightness2;  // Yet another approach

////////////////////////////////////

        TheData() :
            m_radius(BadValue),
            m_sun_dir(BadValue),
            m_sun_width_ang(0.5),

            m_IsConvex(false),
            m_MirrorCOCPt(),
            m_min_normal_dir(0),
            m_max_normal_dir(360),
            m_min_normal_pt(),
            m_max_normal_pt(),
            m_MidArcPt(),

            m_screen(),
            m_stencils(),
            m_target_pts(),

            m_distance(BadValue),
            m_ObserverPt(),

            m_TopRays(),
            m_BotRays(),
            m_CountOfObscuredRays(0),

            m_TopIntersectionPts(),
            m_BotIntersectionPts(),
            m_TopIntersectionBBox(),
            m_BotIntersectionBBox(),
            m_reflected_rays_width_ang(BadValue),
            m_reflected_focal_distance(BadValue),
            m_reflected_blur(BadValue),

            m_ObserverTangentAng(BadValue),
            m_NormalTangentAng(BadValue),
            m_TangentPt(),
            m_ObserverReflectedSunBot(BadValue),
            m_SunBotAng(BadValue),
            m_SunBotMirrorPt(),
            m_ObserverReflectedSunMid(BadValue),
            m_SunMidAng(BadValue),
            m_SunMidMirrorPt(),
            m_ObserverReflectedSunTop(BadValue),
            m_SunTopAng(BadValue),
            m_SunTopMirrorPt(),
            m_Pupil_Entrance(BadValue),
            m_Pupil_Exit(BadValue),
            m_Brightness(BadValue),
            m_Brightness2(BadValue)
                   {};

        void InputDump(FILE *fout=stdout) const;

        bool CheckInputs() const; // return success
        void Dump(FILE *fout=stdout) const;

        void Calculate(int num_rays, int do_pupil);

        bool GenSVG_Concave(FILE *fout, double offset_X, double offset_Y, const std::string& title, bool first_call=1, bool last_call=1, int animate=0, int animate_interval_ms=250, bool do_boxes=1, bool focal_pts=1) const;
		bool GenSVG_Convex (FILE *fout, double offset_X, double offset_Y, bool first_call=1, bool last_call=1, int animate=0) const;

        void RayReport(FILE *fout=stdout, unsigned level=0) const;

        double GetValue(const std::string& name) const;

        TheData& operator=(const TheData&other);

        void DuplicateSettings(const TheData&other); // copies other's set-up data, but no the results

    private:
        void Calculate_Concave(int num_rays, int do_pupil); // forward-trace if num_rays>0, reverse-trace if num_rays==0
        void Calculate_Convex(int num_rays, int do_pupil);

};

TheData& TheData::operator=(const TheData& other)
{
    if (this == &other) return *this;

    m_radius = other.m_radius;
    m_sun_dir = other.m_sun_dir;
    m_sun_width_ang = other.m_sun_width_ang;

    m_IsConvex = other.m_IsConvex;
    m_MirrorCOCPt = other.m_MirrorCOCPt;
    m_min_normal_dir = other.m_min_normal_dir;
    m_max_normal_dir = other.m_max_normal_dir;
    m_min_normal_pt = other.m_min_normal_pt;
    m_max_normal_pt = other.m_max_normal_pt;
    m_MidArcPt = other.m_MidArcPt;

    m_screen = other.m_screen;
    m_stencils = other.m_stencils;
    m_target_pts = other.m_target_pts;

    m_distance = other.m_distance;
    m_ObserverPt = other.m_ObserverPt;

    m_TopRays = other.m_TopRays;
    m_BotRays = other.m_BotRays;
    m_CountOfObscuredRays = other.m_CountOfObscuredRays;

    m_TopIntersectionPts = other.m_TopIntersectionPts;
    m_BotIntersectionPts = other.m_BotIntersectionPts;
    m_TopIntersectionBBox = other.m_TopIntersectionBBox;
    m_BotIntersectionBBox = other.m_BotIntersectionBBox;
    m_reflected_rays_width_ang = other.m_reflected_rays_width_ang;
    m_reflected_focal_distance = other.m_reflected_focal_distance;
    m_reflected_blur = other.m_reflected_blur;

    m_ObserverTangentAng = other.m_ObserverTangentAng;
    m_NormalTangentAng = other.m_NormalTangentAng;
    m_TangentPt = other.m_TangentPt;
    m_ObserverReflectedSunBot = other.m_ObserverReflectedSunBot;
    m_SunBotAng = other.m_SunBotAng;
    m_SunBotMirrorPt = other.m_SunBotMirrorPt;
    m_ObserverReflectedSunMid = other.m_ObserverReflectedSunMid;
    m_SunMidAng = other.m_SunMidAng;
    m_SunMidMirrorPt = other.m_SunMidMirrorPt;
    m_ObserverReflectedSunTop = other.m_ObserverReflectedSunTop;
    m_SunTopAng = other.m_SunTopAng;
    m_SunTopMirrorPt = other.m_SunTopMirrorPt;
    m_Pupil_Entrance = other.m_Pupil_Entrance;
    m_Pupil_Exit = other.m_Pupil_Exit;
    m_Brightness = other.m_Brightness;
    m_Brightness2 = other.m_Brightness2;
}

void TheData::InputDump(FILE *fout) const
{
    fprintf(fout,"Dump this=%p: Radius=%g, Sun: dir=%g (altitude=%g), Width=%g degrees, Normals=%g,%g degrees, Screen=(%g,%g)..(%g,%g)\n",
                this, m_radius, m_sun_dir, NormalizeAngle(m_sun_dir+180), m_sun_width_ang, m_min_normal_dir, m_max_normal_dir,
                m_screen.first.x(), m_screen.first.y(), m_screen.second.x(), m_screen.second.y() );
    if (! m_stencils.empty() ) {
        fprintf(fout, "\tStencils (each line of 4 points):");
        int max_index = m_stencils.size()-1;
        for (int ii=0; ii <= max_index; ii++) {
            fprintf(fout, "  (%g,%g)..(%g,%g)", m_stencils[ii].first.x(), m_stencils[ii].first.y(), m_stencils[ii].second.x(), m_stencils[ii].second.y() );
        }
        fprintf(fout, "\n");
    }
    if ( !m_target_pts.empty()) {
        fprintf(fout, "\tTarget Points (for reverse tracing): ");
        for (std::list<Point>::const_iterator it = m_target_pts.begin(); it != m_target_pts.end(); ++it)
            fprintf(fout, " (%g,%g)", it->x(), it->y() );
        fprintf(fout, "\n");
    }
}

void TheData::DuplicateSettings(const TheData& other)
{
    m_radius = other.m_radius;
    m_sun_dir = other.m_sun_dir;
    m_sun_width_ang = other.m_sun_width_ang;

    m_IsConvex = other.m_IsConvex;
    m_MirrorCOCPt = other.m_MirrorCOCPt;
    m_min_normal_dir = other.m_min_normal_dir;
    m_max_normal_dir = other.m_max_normal_dir;
    m_min_normal_pt = other.m_min_normal_pt;
    m_max_normal_pt = other.m_max_normal_pt;
    m_MidArcPt = other.m_MidArcPt;

    m_screen = other.m_screen;
    m_stencils = other.m_stencils;
    m_target_pts = other.m_target_pts;
    m_distance = other.m_distance;
    m_ObserverPt = other.m_ObserverPt;
}


void TheData::Dump(FILE *fout) const
{
    InputDump(fout);
    if (m_IsConvex) { // if Convex
        fprintf(fout, "Observer: (%g,%g), ConvexMirror: (%g,%g)\n", m_ObserverPt.x(), m_ObserverPt.y(), m_MirrorCOCPt.x(), m_MirrorCOCPt.y() );
        fprintf(fout, "Tangent: (%g,%g): Observer Ang=%g, Normal (COC)=%g\n", m_TangentPt.x(), m_TangentPt.y(), m_ObserverTangentAng, m_NormalTangentAng );
        fprintf(fout, "Sun's Bot: Ang=%g, Observer (to reflection)=%g, Reflection Point=(%g,%g)\n", m_SunBotAng, m_ObserverReflectedSunBot, m_SunBotMirrorPt.x(), m_SunBotMirrorPt.y() );
        fprintf(fout, "Sun's Mid: Ang=%g, Observer (to reflection)=%g, Reflection Point=(%g,%g)\n", m_SunMidAng, m_ObserverReflectedSunMid, m_SunMidMirrorPt.x(), m_SunMidMirrorPt.y() );
        fprintf(fout, "Sun's Top: Ang=%g, Observer (to reflection)=%g, Reflection Point=(%g,%g)\n", m_SunTopAng, m_ObserverReflectedSunTop, m_SunTopMirrorPt.x(), m_SunTopMirrorPt.y() );
        fprintf(fout, "Pupils=%g/%g, Brightness=%g,%g Obsever Angle=%g\n", m_Pupil_Entrance, m_Pupil_Exit, m_Brightness, m_Brightness2, m_ObserverReflectedSunTop-m_ObserverReflectedSunBot);
    } else { // Concave
        fprintf(fout, "ConcaveMirror: (%g,%g)\n", m_MirrorCOCPt.x(), m_MirrorCOCPt.y() );
        for (auto it=m_TopRays.begin(); it != m_TopRays.end(); ++it) {
            fprintf(fout,"Top: Sun dir=%g, Reflect dir=%g:", it->m_sun_dir, it->m_reflect_dir);
            for (auto rr=it->m_StrikePts.begin(); rr != it->m_StrikePts.end(); ++rr)
                fprintf(fout, " (%g,%g)", rr->x(), rr->y() );
        }
        for (auto it=m_BotRays.begin(); it != m_BotRays.end(); ++it) {
            fprintf(fout,"Bot: Sun dir=%g, Reflect dir=%g:", it->m_sun_dir, it->m_reflect_dir);
            for (auto rr=it->m_StrikePts.begin(); rr != it->m_StrikePts.end(); ++rr)
                fprintf(fout, " (%g,%g)", rr->x(), rr->y() );
        }
        for (auto it = m_TopIntersectionPts.begin(); it != m_TopIntersectionPts.end(); ++it)
            fprintf(fout,"Top: Intersection Point=(%g,%g)\n", it->x(), it->y());
        for (auto it = m_BotIntersectionPts.begin(); it != m_BotIntersectionPts.end(); ++it)
            fprintf(fout,"Bot: Intersection Point=(%g,%g)\n", it->x(), it->y());
        fprintf(fout,"Top bounding Box: (%g,%g)..(%g,%g), Bot bounding Box: (%g,%g)..(%g,%g)\n",
            m_TopIntersectionBBox.MinX(), m_TopIntersectionBBox.MinY(),
            m_TopIntersectionBBox.MaxX(), m_TopIntersectionBBox.MaxY(),
            m_BotIntersectionBBox.MinX(), m_BotIntersectionBBox.MinY(),
            m_BotIntersectionBBox.MaxX(), m_BotIntersectionBBox.MaxY()
            );
        fprintf(fout,"Reflected Rays width angle=%g (deg), focal distance=%g, blur=%g, #obscured rays=%d\n",
            m_reflected_rays_width_ang, m_reflected_focal_distance, m_reflected_blur, m_CountOfObscuredRays );
    }
}

double TheData::GetValue(const std::string& name) const
    // DVO HELP - needs to be updated for m_TopRays and m_BotRays.
{
    if (name == "radius")           return m_radius;
    if (name == "distance")         return m_distance;
    if (name == "sun_width")        return m_sun_width_ang;
    if (name == "sun_a")            return m_sun_dir;
    if (name == "sun_A")            return NormalizeAngle(m_sun_dir+180);
    if (name == "ref_width")        return m_CountOfObscuredRays ? BadValue : NormalizeAngle(m_reflected_rays_width_ang);
    if (name == "ref_width_p")      return m_CountOfObscuredRays ? BadValue : 100 * NormalizeAngle(m_reflected_rays_width_ang) / m_sun_width_ang;
    if (name == "ref_focal_d")      return m_reflected_focal_distance;
    if (name == "ref_focal_p")      return 100 * (m_reflected_focal_distance / (m_radius/2)) ;
    if (name == "ref_blur")         return m_reflected_blur;
    if (name == "min_normal")       return m_min_normal_dir;
    if (name == "max_normal")       return m_max_normal_dir;
    if (name == "mirror_width")     return m_max_normal_dir - m_min_normal_dir;

	if (name == "pupil")			return m_Pupil_Entrance;
	if (name == "pupil1")			return m_Pupil_Entrance;
	if (name == "pupil2")			return m_Pupil_Exit;
	if (name == "brightness")		return m_Brightness;
	if (name == "brightness2")		return m_Brightness2;

fprintf(stderr,"ERROR: %s(%s): Unrecognized parameter name.\n", __func__, name.c_str());
    return 0;
}

bool TheData::CheckInputs() const
{
    if (m_radius==0) {
        printf("Error: Radius is zero.\n");
        return false;
    }
    if (m_radius<0) {
        printf("Error: Radius is negative.\n");
        return false;
    }
    // It is ok for the sun altitude and/or width to be zero

    return true;
}



int Recursive_ConcaveRaySearch(const Point& MirrorCOCPt, double radius, double min_arc_normal_dir, double max_arc_normal_dir,
                                const Point& RayTraceStartPt,
                                double target_sun_dir,
                                double min_normal_dir, double max_normal_dir,
                                std::deque<TracedRay> & found_rays,
                                int nest_level=0,
                                const int num_steps = 51
                                )
/* Performs a search to identify the location on the mirror (MirrorCOCPt, radius min/max_arc_normal_dir) such that a ray starting at RayTraceStartPt
 * is reflected to the sun (target_sun_dir). min/max_normal_dir are within min/max_arc_normal_dir, and are tightened/refined as the search proceeds.
 * Starts by breaking the arc (min_normal_dir..max_normal_dir) into num_steps - and evaluating the reflection at each one. Then we'll pick the
 * two that are on either side and recursive evaluate that sub-region.
 *
 */
{
    assert(min_normal_dir != BadValue);
    assert(max_normal_dir != BadValue);
    double normals[num_steps];
    for (int jj=0; jj<num_steps; jj++) {
        normals[jj] = min_normal_dir + jj * ((max_normal_dir - min_normal_dir) / (num_steps-1));
    }


    int success_count = 0;
    double found_suns[num_steps];
    for (int jj=0; jj<num_steps; jj++) {
        found_suns[jj] = BadValue;
        TracedRay tr;
        Point target_pt = Find2ndPoint( MirrorCOCPt, normals[jj], radius ); // target_pt is on the mirror
        double incident_angle = Direction( RayTraceStartPt, target_pt );
        double reflect_angle = ConcaveRayCalculate (MirrorCOCPt, radius, min_arc_normal_dir, max_arc_normal_dir,
                                                    incident_angle, RayTraceStartPt, target_pt, tr.m_StrikePts, tr.m_ray_status);
        if (tr.m_ray_status >= TracedRay::NStrike) {
            found_suns[jj] = reflect_angle;

            if ( NearlyEqual( target_sun_dir, found_suns[jj] ) ) {
                tr.m_sun_dir = NormalizeAngle( found_suns[jj] + 180 );
                tr.m_MirrorPt = target_pt;
                tr.m_reflect_dir = NormalizeAngle(incident_angle+180); // We're doing this in reverse - so what we start with as incident is actually the reflected.
                if ( (jj==0) || (found_suns[jj-1] != BadValue) ) { // Prevents back to back submissions (likely the same ray - within floating-point roundoff)
                    found_rays.push_back(tr);
                    success_count++;
                }
                found_suns[jj] = BadValue; // So won't be considered again below
            }
        }
    }


    for (int jj=1; jj<num_steps; jj++) { // Starts at 1
        if (
             ( (found_suns[jj-1] != BadValue) && (found_suns[jj-0] != BadValue)) &&
            ( ( ( found_suns[jj-1] < target_sun_dir) && (found_suns[jj-0] > target_sun_dir) ) ||
              ( ( found_suns[jj-1] > target_sun_dir) && (found_suns[jj-0] < target_sun_dir) ) ) &&
            ( ! NearlyEqual( found_suns[jj-1], found_suns[jj-0] ) ) &&
            ( ! NearlyEqual( normals[jj-1], normals[jj-0] ) )
             ) {
                int result = Recursive_ConcaveRaySearch(MirrorCOCPt,radius,min_arc_normal_dir,max_arc_normal_dir,RayTraceStartPt,target_sun_dir,normals[jj-1],normals[jj-0], found_rays, nest_level+1, num_steps);
                if (result) {
                    success_count += result;
                }
        } // if
    } // for

    return success_count;
}



void TheData::Calculate(int num_rays, int do_pupil)
{
    if (m_IsConvex) Calculate_Convex (num_rays, do_pupil);
    else            Calculate_Concave(num_rays, do_pupil);
}

void TheData::Calculate_Concave(int num_rays, int do_pupil)
    /* The object a few 'input' parameters, and numerous 'derived' values - that are determined from the
     * 'input' parameters. This routine determines those derived values.
     */
{
    const int steps = num_rays-1;
    if (CheckInputs()) {
        // Parameters of the mirror
        Set( m_MirrorCOCPt, 0, 0 );
        double m_mid_normal_dir = (m_max_normal_dir + m_min_normal_dir)/2;
        m_MidArcPt= Find2ndPoint(Point(0,0), m_mid_normal_dir, m_radius );

        m_min_normal_pt = Find2ndPoint( Point(0,0), m_min_normal_dir, m_radius );
        m_max_normal_pt = Find2ndPoint( Point(0,0), m_max_normal_dir, m_radius );

        if (num_rays == 0) {    /* Reverse ray-tracing
                                 * The sun's angle in the sky is an input. Project a ray from stencil back to mirror
                                 * and then back to sun (and finally extend stencil to mirror segment to reach the
                                 * screen). Requires successive-approximation (search).
                                 * 1-Project a ray from a point on the stencil back to a point on the mirror.
                                 *      (Determine the angle from the stencil to the point. This is the initial
                                 *      angle - and will be adjusted in this search).
                                 * 2-The above determines an incident angle. From that, and the mirror's properties,
                                 *      determine the reflected angle.
                                 * 3-Adjust if/as necessary depending on whether the reflected-angle is greater than or
                                 *      less than the known sun-angle.
                                 */
            std::list<Point> target_points;
            for (auto it = m_stencils.begin(); it != m_stencils.end(); ++it) {
                    target_points.push_back(it->first);
                    target_points.push_back(it->second);
            }
            for (std::list<Point>::const_iterator it = m_target_pts.begin(); it != m_target_pts.end(); ++it) 
                    target_points.push_back( *it );
            
            for (int bot_top = 0; bot_top <= 1; bot_top++) { // bot_top=0 for bottom, =1 for top
                double sun_dir = m_sun_dir + ((bot_top == 0) ? -m_sun_width_ang : m_sun_width_ang)/2;
                double sun_dir_reversed = NormalizeAngle(sun_dir + 180);

                for (std::list<Point>::const_iterator it = target_points.begin(); it != target_points.end(); ++it) {
                    const Point &the_point = *it;

                    double sun_m_90 = sun_dir-90;
                    double sun_p_90 = sun_dir+90;

                    double min_normal_dir = Max( m_min_normal_dir, sun_m_90);
                    double max_normal_dir = Min( m_max_normal_dir, sun_p_90);

                    std::deque<TracedRay>& tr_deque = bot_top ? m_TopRays : m_BotRays;
                    int result = Recursive_ConcaveRaySearch(m_MirrorCOCPt, m_radius, m_min_normal_dir, m_max_normal_dir,
                            the_point,
                            sun_dir_reversed,
                            min_normal_dir, max_normal_dir,
                            tr_deque );
                } // for points
            } // for bot_top
        } else { // forward ray-trace - from Sun to mirror. First identify a target point on the mirror, then calculate the reflection.

            double step_size = (m_max_normal_dir - m_min_normal_dir) / steps;

            for (int step=0; step <= steps; step++) { // steps along points on the mirror
                // Terminology...
                // top/bot - refer to whether the incident ray originates at the top (12oc) or bottom (6oc) of the sun
                //
                TracedRay tr_top, tr_bot;
                tr_top.m_sun_dir = m_sun_dir + m_sun_width_ang/2;
                tr_bot.m_sun_dir = m_sun_dir - m_sun_width_ang/2;

                double normal_dir = m_min_normal_dir + step * step_size;
                tr_top.m_MirrorPt = tr_bot.m_MirrorPt = Find2ndPoint(Point(0,0), normal_dir, m_radius );

                tr_top.m_reflect_dir = NormalizeAngle(ConcaveRayCalculate(Point(0,0), m_radius, m_min_normal_dir, m_max_normal_dir,
                                tr_top.m_sun_dir, Point(BadValue,BadValue), tr_top.m_MirrorPt, tr_top.m_StrikePts, tr_top.m_ray_status ));
                tr_bot.m_reflect_dir = NormalizeAngle(ConcaveRayCalculate(Point(0,0), m_radius, m_min_normal_dir, m_max_normal_dir,
                                tr_bot.m_sun_dir, Point(BadValue,BadValue), tr_bot.m_MirrorPt, tr_bot.m_StrikePts, tr_bot.m_ray_status ));

                if (tr_top.m_ray_status >= TracedRay::NStrike) m_TopRays.push_back( tr_top );
                if (tr_bot.m_ray_status >= TracedRay::NStrike) m_BotRays.push_back( tr_bot );

//                m_CountOfObscuredRays += tr_top.CountObscuredRays();
//                m_CountOfObscuredRays += tr_bot.CountObscuredRays();
            } // for step
        } // if else forward ray trace


        // An N-squared algorithm (originally, but not much better now) - looking for all intersections of Top
        // rays (and then again, all intersections of Bot rays)
        {
            int outer = 0;
            std::deque<TracedRay>* traced_rays[] = { &m_TopRays, &m_BotRays };
            for (int tri = 0; tri < sizeof(traced_rays)/sizeof(traced_rays[0]); tri++) {
                for (auto it_outer = traced_rays[tri]->begin(); it_outer != traced_rays[tri]->end(); ++it_outer) {
                    outer++;
                    if (it_outer->m_ray_status <= TracedRay::Obscured) continue;
                    int inner = 0;
                    for (auto it_inner = traced_rays[tri]->begin(); it_inner != traced_rays[tri]->end(); ++it_inner) {
                        inner++;
                        if (it_outer == it_inner) break;
                        if (it_inner->m_ray_status <= TracedRay::Obscured) continue;
                        Point intersection_pt;
                        int ok = Intersection( it_outer->m_StrikePts.back(), it_outer->m_reflect_dir,
                                               it_inner->m_StrikePts.back(), it_inner->m_reflect_dir,
                                               intersection_pt);

                        // There can be three situations:
                        // 1) Unobscured - if so, then don't worry about where intersection point is
                        // 2) Initially NStrike - but intersection is beyond the mirror surface - so the intersection point is NOT valid
                        // 3) Initially NStrike - but intersection is before the mirror surface - so the intersection point is valid


                        if (ok) {
                            assert(Defined(it_outer->m_StrikePts.back()));
                            assert(Defined(it_inner->m_StrikePts.back()));
                            double outer_reflctPtToIntrsct_dis  = Distance( it_outer->m_StrikePts.back(), intersection_pt );
                            double inner_reflctPtToIntrsct_dis  = Distance( it_inner->m_StrikePts.back(), intersection_pt );

                            if (tri == 0) { // This is an ugly hack
                                m_TopIntersectionBBox.Update( intersection_pt );
                                m_TopIntersectionPts.push_back( intersection_pt );
                            } else {
                                m_BotIntersectionBBox.Update( intersection_pt );
                                m_BotIntersectionPts.push_back( intersection_pt );
                            }
                        }
                    } // for it_inner
                } // for it_outer
            }
        }

        if ( m_TopIntersectionBBox.Defined() && m_BotIntersectionBBox.Defined() ) { // reflected rays 'blur' width angle
            // using the middle points of the bounding boxes as the intersection points - this is first implementation - there may be a better way
            Point top_intersection( m_TopIntersectionBBox.MidX(), m_TopIntersectionBBox.MidY() );
            Point bot_intersection( m_BotIntersectionBBox.MidX(), m_BotIntersectionBBox.MidY() );


            double dir1 = to_degrees( atan2( top_intersection.y()-m_MidArcPt.y(), top_intersection.x()-m_MidArcPt.x() ));
            double dir2 = to_degrees( atan2( bot_intersection.y()-m_MidArcPt.y(), bot_intersection.x()-m_MidArcPt.x() ));
            m_reflected_rays_width_ang = fabs(dir1-dir2);

            assert( Defined(m_MidArcPt) );
            assert( Defined(top_intersection) );
            assert( Defined(bot_intersection) );
            double distance1 = Distance( m_MidArcPt, top_intersection );
            double distance2 = Distance( m_MidArcPt, bot_intersection );
            m_reflected_focal_distance = (distance1 + distance2)/2;

            double blur1 = m_TopIntersectionBBox.Diagonal();
            double blur2 = m_BotIntersectionBBox.Diagonal();
            m_reflected_blur = (blur1 + blur2)/2;
        }

        if (0) { // experimental - bounding ellipse
            // https://stackoverflow.com/questions/1768197/bounding-ellipse
            for (int  tri=0; tri<=1; tri++) {
                std::deque<Point> &IntersectionPts = /* ugly hack continued */ (tri == 0) ? m_TopIntersectionPts : m_BotIntersectionPts;
            }
        }

    } // if CheckPoint()
}


class CoordConverter
{
    public:

        void DefineFrom(double x_bot_left, double y_bot_left, double x_top_right, double y_top_right);
        void DefineTo  (double x_bot_left, double y_bot_left, double x_top_right, double y_top_right);
        double X(double from_X) const;
        double Y(double from_X) const;
        double Scale() const;

        CoordConverter() :  m_X_bot_left_from(0), m_Y_bot_left_from(0), m_X_top_right_from(0), m_Y_top_right_from(0),
                    m_X_bot_left_to(0), m_Y_bot_left_to(0), m_X_top_right_to(0), m_Y_top_right_to(0),
                        m_cache_is_stale(1), m_X_scale(1), m_Y_scale(1) {};

        void Dump(FILE* fout=stdout) const;

        static int Test();
    private:
        void CacheCalc() const;
        double m_X_bot_left_from, m_Y_bot_left_from, m_X_top_right_from, m_Y_top_right_from;
        double m_X_bot_left_to, m_Y_bot_left_to, m_X_top_right_to, m_Y_top_right_to;

        mutable bool m_cache_is_stale;
        mutable double m_X_scale, m_Y_scale;
};

void CoordConverter::Dump(FILE *fout) const
{
    fprintf(fout, "Dump() of CoordConverter this=%p\n", this);
    fprintf(fout, "\tFrom: bottomLeft=(%g,%g), topRight=(%g,%g)\n", m_X_bot_left_from, m_Y_bot_left_from, m_X_top_right_from, m_Y_top_right_from );
    fprintf(fout, "\t  To: bottomLeft=(%g,%g), topRight=(%g,%g)\n", m_X_bot_left_to, m_Y_bot_left_to, m_X_top_right_to, m_Y_top_right_to );
    fprintf(fout, "\tScale=(%g,%g): m_cache_is_stale=%d\n", m_X_scale, m_Y_scale, m_cache_is_stale );

}

int CoordConverter::Test()
{
    int test_count = 0;
    int fail_count = 0;
    { // Defines an identity mapping
        CoordConverter cc;
        cc.DefineFrom(0,0, 100,100);
        cc.DefineTo  (0,0, 100,100);
        const static double test_points[] = { // A 1-dimensional array - but treating as pairs of X,Y coordinates.
            0,0,    1,1,    0,1,    1,0,    2,2,    2,0,    0,2,    0,100,    100,100,    100,0,    51,49 };
        for (int ii=0; ii<sizeof(test_points)/sizeof(test_points[0]); ii+=2) {
            double x = test_points[ii];
            double y = test_points[ii+1];
            if ( (!NearlyEqual(x, cc.X(x) )) || !NearlyEqual(y,cc.Y(y))) {
                printf("Test failure: (%g,%g) != (%g,%g) at %d of %s\n", x,y, cc.X(x), cc.Y(y), __LINE__, __FILE__);
                fail_count++;
            }
            test_count++;
        }
    }

    { // Scaling only
        CoordConverter cc;
        cc.DefineFrom(-100,-100, 100,100);
        cc.DefineTo  (-1,-1, 1,1);
        const static double test_points[] = { // A 1-dimensional array - but treating as pairs of X,Y coordinates.
            0,0,    1,1,    0,1,    1,0,    2,2,    2,0,    0,2,    0,100,    100,100,    100,0,    51,49 };
        for (int ii=0; ii<sizeof(test_points)/sizeof(test_points[0]); ii+=2) {
            double x = test_points[ii];
            double y = test_points[ii+1];
            if ( (! NearlyEqual(x/100, cc.X(x) )) || !NearlyEqual(y/100,cc.Y(y)) ) {
                printf("Test failure: (%g,%g) != (%g,%g) at %d of %s\n", x,y, cc.X(x), cc.Y(y), __LINE__, __FILE__);
                fail_count++;
            }
            test_count++;
        }
    }

    { // Offset only
        CoordConverter cc;
        cc.DefineFrom(-100,-100, 100,100);
        cc.DefineTo  (0,0, 200,200);
        const static double test_points[] = { // A 1-dimensional array - but treating as pairs of X,Y coordinates.
            0,0,    1,1,    0,1,    1,0,    2,2,    2,0,    0,2,    0,100,    100,100,    100,0,    51,49 };
        for (int ii=0; ii<sizeof(test_points)/sizeof(test_points[0]); ii+=2) {
            double x = test_points[ii];
            double y = test_points[ii+1];
            if ( (! NearlyEqual(x+100, cc.X(x) )) || !NearlyEqual(y+100,cc.Y(y)) ) {
                printf("Test failure: (%g,%g) != (%g,%g) at %d of %s\n", x,y, cc.X(x), cc.Y(y), __LINE__, __FILE__);
                fail_count++;
            }
            test_count++;
        }
    }

    { // Inversion top-to-bottom only
        CoordConverter cc;
        cc.DefineFrom(-100,-100, 100, 100);
        cc.DefineTo  (-100, 100, 100,-100);
        const static double test_points[] = { // A 1-dimensional array - but treating as pairs of X,Y coordinates.
            0,0,    1,1,    0,1,    1,0,    2,2,    2,0,    0,2,    0,100,    100,100,    100,0,    51,49 };
        for (int ii=0; ii<sizeof(test_points)/sizeof(test_points[0]); ii+=2) {
            double x = test_points[ii];
            double y = test_points[ii+1];
            if ( (! NearlyEqual(x, cc.X(x) )) || !NearlyEqual(-1*y,cc.Y(y)) ) {
                printf("Test failure: (%g,%g) != (%g,%g) at %d of %s\n", x,y, cc.X(x), cc.Y(y), __LINE__, __FILE__);
                fail_count++;
            }
            test_count++;
        }
    }

    { // Explicit mapping
        CoordConverter cc;
        cc.DefineFrom(-3, 0, 3, 6);
        cc.DefineTo  (0, 800, 800, 0);
        const static double test_points[] = { // A 1-dimensional array - but treating as pairs of X,Y coordinates.
            -3,6,    3,6,     3,0,        -3,0,    0,3,        0,0        };     
        const static double expect_points[] = { // Matches (after coordinate-conversion) the test_points[] array
            0,0,    800,0,    800,800,    0,800,    400,400,    400,800        };
        for (int ii=0; ii<sizeof(test_points)/sizeof(test_points[0]); ii+=2) {
            double x_f = test_points[ii];
            double y_f = test_points[ii+1];
            double x_t = expect_points[ii];
            double y_t = expect_points[ii+1];

            if ( (! NearlyEqual(x_t, cc.X(x_f) )) || !NearlyEqual(y_t,cc.Y(y_f)) ) {
                printf("Test failure: (%g,%g) != (%g,%g) - was (%g,%g) ii=%d at %d of %s\n", x_f,y_f, x_t,y_t, cc.X(x_f), cc.Y(y_f), ii, __LINE__, __FILE__);
                fail_count++;
            }
            test_count++;
        }
    }

    { // Angles
        const static double test_data[] = { // 5 values per test-point: X,Y of pt1, X,Y of pt2 and expected Angle
             0, 0,     0, 0,      0.0,

             0, 0,     1, 0,      0.0,
             0, 0,    -1, 0,    180.0,
             0, 0,     0, 1,     90.0,
             0, 0,     0,-1,    -90.0,
             0, 0,     1, 1,     45.0,
             0, 0,    -1, 1,    135.0,
             0, 0,     1,-1,    -45.0,
             0, 0,    -1,-1, -135.0,

             1, 0,     0, 0,    180.0,
            -1, 0,     0, 0,      0.0,
             0, 1,     0, 0,    -90.0,
             0,-1,     0, 0,     90.0,
             1, 1,     0, 0, -135.0,
             1,-1,     0, 0,    135.0,
            -1, 1,     0, 0,  -45.0,
            -1,-1,     0, 0,     45.0
        };
        for (int ii=0; ii<sizeof(test_data)/sizeof(test_data[0]); ii+=5) {
            double result = Direction( Point(test_data[ii+0], test_data[ii+1]), Point(test_data[ii+2], test_data[ii+3]) );
            double result_normalize = NormalizeAngle( result );
            double expected_normalize = NormalizeAngle( test_data[ii+4] );
            if ( ! NearlyEqual(result_normalize, expected_normalize) ) { 
                printf("Test failure: Direction( %g,%g, %g,%g )=%g - expecting %g (ii=%d at %d of %s)\n",
                        test_data[ii+0], test_data[ii+1], test_data[ii+2], test_data[ii+3],
                        result_normalize, expected_normalize,
                        ii, __LINE__, __FILE__ );
                fail_count++;
            }
            test_count++;
        }
    }

    { // RayStrikeConcave
        double test_points[] = { // 3 values per test-point - the incident ray, the normal and the expected result (1 or 0)
              0,  0,     1,    // The normal is always from inside
              1,  1,     1,    // The normal is always from inside
             45, 45,     1,    // The normal is always from inside
             89, 89,    1,
             90, 90,    1,
             91, 91,    1,
             89, 90,    1,
             89, 91,    1,
             91, 89,    1,
             91, 90,    1,
            179,179,    1,
            180,180,    1,
            181,181,    1,
            269,269,    1,
            270,270,    1,
            271,271,    1,
            359,359,    1,
            360,360,    1,    // 360 should get normalized to 0
            361,361,    1,    // 360 should get normalized to 0

              0,  0+89,    1,
              0,  0-89,    1,
             90, 90-89,    1,
             90, 90-89,    1,
            180,180+89,    1,
            180,180-89,    1,
            270,270+89,    1,
            270,270-89,    1,
            359,359+89,    1,
            359,359-89,    1,

              0,  0+91,    0,
              0,  0-91,    0,
             90, 90-91,    0,
             90, 90-91,    0,
            180,180+91,    0,
            180,180-91,    0,
            270,270+91,    0,
            270,270-91,    0,
            359,359+91,    0,
            359,359-91,    0,

              0,  0+180,    0,
              0,  0-180,    0,
             90, 90-180,    0,
             90, 90-180,    0,
            180,180+180,    0,
            180,180-180,    0,
            270,270+180,    0,
            270,270-180,    0,
            359,359+180,    0,
            359,359-180,    0,

              0,  0+269,    0,
              0,  0-269,    0,
             90, 90-269,    0,
             90, 90-269,    0,
            180,180+269,    0,
            180,180-269,    0,
            270,270+269,    0,
            270,270-269,    0,
            359,359+269,    0,
            359,359-269,    0,

        };
        for (int ii=0; ii<sizeof(test_points)/sizeof(test_points[0]); ii+=3) {
            double result = RayStrikeConcave( test_points[ii+0], test_points[ii+1] );
            if (result != test_points[ii+2]) {
                printf("Test failure: RayStrikeConcave(%g,%g)=%g, expected %g. ii=%d at %d of %s\n",
                        test_points[ii+0], test_points[ii+1], result, test_points[ii+2],
                        ii, __LINE__, __FILE__ );
                fail_count++;
                }
            test_count++;
        }
    }

    { // Intersection - point-dir
        static const double test_points[] = { // 9 values per test-point - pt1, dir1, pt2, dir2, pass/fail, intersection pt (each pt has 2 values)
            // pt1     dir1        pt2     dir2          p/f    intersect pt  
            1,   2,     0,       1,    2,    270,        1,        1,   2,
            1,   2,    13,       1,    2,     57,        1,        1,   2,

            0,   0,     0,       1,    1,    270,        1,        1,   0,
            0,   0,     0,       1,    1,      0,        0,        0,   0,
            0,   0,     0,       1,    1,    180,        0,        0,   0,
            0,   0,    90,       1,    1,    180,        1,        0,   1,

            1,   2,    45,       0,    2,      0,        1,        1,   2,
            1,   2,    45,       0,    4,      0,        1,        3,   4,
            1,   2,    45,       1,    4,    -45,        1,        2,   3,

        };

        for (int ii=0; ii<sizeof(test_points)/sizeof(test_points[0]); ii+=9) {
            Point pt1(test_points[ii+0],test_points[ii+1]);
            double dir1 = test_points[ii+2];
            Point pt2(test_points[ii+3],test_points[ii+4]);
            double dir2 = test_points[ii+5];
            Point intersection_pt;
            bool result = Intersection( pt1, dir1, pt2, dir2, intersection_pt);

            if (result != test_points[ii+6]) {
                printf("Test failure: Intersection( (%g,%g), %g, (%g,%g), %g, ...)=%d, expected %g. ii=%d at %d of %s\n",
                        pt1.x(), pt1.y(), dir1,
                        pt2.x(), pt2.y(), dir2,
                        result, test_points[ii+6],
                        ii, __LINE__, __FILE__ );
                fail_count++;
            }
            test_count++;

            if (result && (test_points[ii+6] != 0)) {
                Point expected_pt(test_points[ii+7], test_points[ii+8]);
                if ( ! NearlyEqual( expected_pt, intersection_pt ) ) {
                    printf("Test failure: Intersection( (%g,%g), %g, (%g,%g), %g, ...)=%d, intersection: (%g,%g) != (%g,%g) ii=%d at %d of %s\n",
                            pt1.x(), pt1.y(), dir1,
                            pt2.x(), pt2.y(), dir2,
                            result,
                            intersection_pt.x(), intersection_pt.y(), expected_pt.x(), expected_pt.y(),
                            ii, __LINE__, __FILE__ );
                    fail_count++;
                }
                test_count++;
            }
        }
    }

    { // Indent()
        static const int test_points[] = { // In sets of 3: 1st and 2nd are arguments to Indent(), 3rd is the expected results of strlen(Indent())
            0,1,0*1,        1,1,1*1,        2,1,2*1,        40,1,40*1,
            0,2,0*2,        1,2,1*2,        2,2,2*2,        40,2,40*2,
            0,4,0*4,        1,4,1*4,        2,4,2*4,        40,4,40*4,
        };

        for (int ii=0; ii<sizeof(test_points)/sizeof(test_points[0]); ii+=3) {
            const char* result = Indent( test_points[ii+0], test_points[ii+1] );
            unsigned length = strlen(result);
            if (length != test_points[ii+2]) {
                printf("Test failure: Indent(%d,%d)=>%s<= length=%d, expected length=%d, ii=%d at %d of %s\n",
                        test_points[ii+0], test_points[ii+1], result, length, test_points[ii+2],
                        ii, __LINE__, __FILE__ );
                fail_count++;
            }
            test_count++;
        }
    }

    if (1) {
            //double ApparentWidth( const Point&p1, const Point&p2, double from_this_angle)
            const double sqrt2 = sqrt(2.0);
            const double sin30 = sin( to_radians(30) );
            const double cos30 = cos( to_radians(30) );
            static const double test_points[] = { // In sets of 6: x,y for each of 2 points, the observer's angle, and the expected result.
                0,0,     10,0,    270,   10,           // I.e. X1,Y1, X2,Y2, angle, expected result. Observing horizontal line from top.
                0,0,     10,0,    225,   10/sqrt2,    //  Same line from 45 degrees.
                0,0,     10,0,    180,   10*0,        //  Same line edge on.

                0,10,    10,0,    270,   10*sqrt2/sqrt2,  // Length of lines is 10*1.414 - but for-shortened when viewed from above.
                0,10,    10,0,    225,   10*sqrt2,        // 
                0,10,    10,0,    180,   10*sqrt2/sqrt2,  //

                -10,-10, 25,-10,  0,     30*0, 
                -10,-10, 25,-10,  30,    35*sin30, 
                -10,-10, 25,-10,  60,    35*cos30, 
                -10,-10, 25,-10,  90,    35*1, 
                -10,-10, 25,-10,  120,   35*cos30, 
                -10,-10, 25,-10,  150,   35*sin30, 
                -10,-10, 25,-10,  180,   35*0, 
                -10,-10, 25,-10,  210,   35*sin30, 
                -10,-10, 25,-10,  240,   35*cos30, 
                -10,-10, 25,-10,  270,   35*1, 
                -10,-10, 25,-10,  300,   35*cos30, 
                -10,-10, 25,-10,  330,   35*sin30, 
                -10,-10, 25,-10,  360,   35*0, 
            };

        for (int ii=0; ii<sizeof(test_points)/sizeof(test_points[0]); ii+=6) {
            const Point pt1(test_points[ii+0], test_points[ii+1] );
            const Point pt2(test_points[ii+2], test_points[ii+3] );
            const double angle = test_points[ii+4];
            const double expected_value = test_points[ii+5];
            const double result = ApparentWidth( pt1, pt2, angle );
            if ( ! NearlyEqual( result, expected_value ) ) {
                    printf("Test failure: ApparentWidth( (%g,%g), (%g,%g), %g)=%g expecting=%g ii=%d at %d of %s\n",
                            pt1.x(), pt1.y(),
                            pt2.x(), pt2.y(),
                            angle,
                            result, expected_value,
                            ii, __LINE__, __FILE__ );
                    fail_count++;
                }
                test_count++;
            }
        }

    if (1) {
            //double ApparentWidth_ang( const Point&p1, const Point&p2, const Point& observer)
            // angular width (in degrees) of a line from p1 to p2 as seen from observer
            static const double test_points[] = { // In sets of 8: line#, x,y for each of 3 points, and the expected result.
__LINE__,                0,0,     10,0,    5,5,  90,
__LINE__,                0,0,     10,0,    100,0,  0,

__LINE__,                -100,0,  0,-100,   0,0,  90,
__LINE__,                -100,0,  0,+100,   0,0,  90,
__LINE__,                +100,0,  0,+100,   0,0,  90,
__LINE__,                +100,0,  0,-100,   0,0,  90,

__LINE__,                -2,0,   0,-100,     0,0, 90,

__LINE__,                10,10,  10,0,       0,0, 45,
__LINE__,                10,0,   10,10,      0,0, 45,

__LINE__,                110,110, 110,100,   100,100, 45,
__LINE__,                110,100, 110,110,   100,100, 45,

__LINE__,                10,0,   10, 0,      0,0, to_degrees(atan2( 0.0,10)),
__LINE__,                10,0,   10, 9,      0,0, to_degrees(atan2( 9.0,10)),
__LINE__,                10,0,   10,10,      0,0, to_degrees(atan2(10.0,10)),
__LINE__,                10,0,   10,11,      0,0, to_degrees(atan2(11.0,10)),
__LINE__,                10,0,   10,20,      0,0, to_degrees(atan2(20.0,10)),
__LINE__,                10,0,   10,30,      0,0, to_degrees(atan2(30.0,10)),
__LINE__,                10,0,   10,40,      0,0, to_degrees(atan2(40.0,10)),
__LINE__,                10,0,   10,50,      0,0, to_degrees(atan2(50.0,10)),
__LINE__,                10,0,   10,60,      0,0, to_degrees(atan2(60.0,10)),
__LINE__,                10,0,   10,70,      0,0, to_degrees(atan2(70.0,10)),
__LINE__,                10,0,   10,80,      0,0, to_degrees(atan2(80.0,10)),
__LINE__,                10,0,   10,90,      0,0, to_degrees(atan2(90.0,10)),
__LINE__,                10,0,   10,99,      0,0, to_degrees(atan2(99.0,10)),

            };

        for (int ii=0; ii<sizeof(test_points)/sizeof(test_points[0]); ii+=8) {
            const double line = test_points[ii+0];
            const Point         pt1(test_points[ii+1], test_points[ii+2] );
            const Point         pt2(test_points[ii+3], test_points[ii+4] );
            const Point observer_pt(test_points[ii+5], test_points[ii+6] );
            const double expected_value = test_points[ii+7];
            const double result = ApparentWidth_ang( pt1, pt2, observer_pt );
            if ( ! NearlyEqual( result, expected_value ) ) {
                    printf("Test failure: ApparentWidth_ang( (%g,%g), (%g,%g), (%g,%g) )=%g expecting=%g ii=%d at %g of %s\n",
                            pt1.x(), pt1.y(),
                            pt2.x(), pt2.y(),
                            observer_pt.x(), observer_pt.y(),
                            result, expected_value,
                            ii, line, __FILE__ );
                    fail_count++;
                }
                test_count++;
            }
        }




    if (fail_count)
        printf("%s(): FAILED %d of %d test-steps.\n", __func__, fail_count, test_count );
    else
        printf("%s(): PASSED all %d test-steps.\n", __func__, test_count );

    return fail_count;
}


void CoordConverter::CacheCalc() const
{
    if (m_cache_is_stale) {
        m_X_scale = (m_X_top_right_to - m_X_bot_left_to) / (m_X_top_right_from - m_X_bot_left_from);
        m_Y_scale = (m_Y_top_right_to - m_Y_bot_left_to) / (m_Y_top_right_from - m_Y_bot_left_from);

        m_cache_is_stale = 0;
    }
}

void CoordConverter::DefineFrom(double x_bot_left, double y_bot_left, double x_top_right, double y_top_right)
{
    m_X_bot_left_from = x_bot_left;
           m_Y_bot_left_from = y_bot_left;
    m_X_top_right_from = x_top_right;
    m_Y_top_right_from = y_top_right;
    m_cache_is_stale = 1;
}

void CoordConverter::DefineTo(double x_bot_left, double y_bot_left, double x_top_right, double y_top_right)
{
    m_X_bot_left_to = x_bot_left;
           m_Y_bot_left_to = y_bot_left;
    m_X_top_right_to = x_top_right;
    m_Y_top_right_to = y_top_right;
    m_cache_is_stale = 1;
}


double CoordConverter::X(double from_X) const
{
    CacheCalc();
    return (from_X - m_X_bot_left_from) * m_X_scale + m_X_bot_left_to;
}
double CoordConverter::Y(double from_Y) const
{
    CacheCalc();
    return (from_Y - m_Y_bot_left_from) * m_Y_scale + m_Y_bot_left_to;
}
double CoordConverter::Scale() const
{
    CacheCalc();
    double f1 = fabs(m_X_scale);
    double f2 = fabs(m_Y_scale);
    return (f1>f2) ? f1 : f2;
}


void Calc_far_point( const Point& FromThisPt, double InThisDirection, Point &far_pt, double border_left, double border_top, double border_right)
{
    double normalized_ray = NormalizeAngle(InThisDirection);
    // Where should the ray end? at one of the borders - or at the  screen
    if ((normalized_ray > 90) && (normalized_ray < 270)) { // Check for intersection with left border
        far_pt.x( border_left );
        far_pt.y( FromThisPt.y() - tan( to_radians( normalized_ray ) ) * (FromThisPt.x() - border_left) );
        if (far_pt.y() > border_top) {
            far_pt.y( border_top );
            far_pt.x( FromThisPt.x() + (far_pt.y() - FromThisPt.y())/tan( to_radians(normalized_ray) ) );
        }
    } else if ((normalized_ray < 90) || (normalized_ray > 270)) { // Check for intersection with right border - DVO HELP - can this be combined with above if <90 ?
        far_pt.x( border_right );
        far_pt.y( FromThisPt.y() + tan( to_radians( normalized_ray ) ) * (border_right - FromThisPt.x()) );
        if (far_pt.x() > border_right) {
            far_pt.y( border_top );
            far_pt.x( FromThisPt.x() - (border_top-FromThisPt.y())/tan( to_radians(normalized_ray) ) );
        }
    } else { // else must be vertical exactly 90 or 270 degrees
        far_pt.x( FromThisPt.x() );
        far_pt.y( border_top );
    }
}


void TheData::RayReport(FILE *fout, unsigned level) const
{
    if (! m_TopRays.empty() ) {
        printf("%sTop Rays:\n", Indent(level));
        for (auto it = m_TopRays.begin(); it != m_TopRays.end(); ++it) {
            it->RayReport(fout, level+1);
        }
    }
    if (! m_BotRays.empty() ) {
        printf("%sBot Rays:\n", Indent(level));
        for (auto it = m_BotRays.begin(); it != m_BotRays.end(); ++it) {
            it->RayReport(fout, level+1);
        }
    }
}

bool TheData::GenSVG_Concave(FILE *fout, double offset_X, double offset_Y, const std::string& title, bool first_call, bool last_call, int animate, int animate_interval_ms, bool do_boxes, bool focal_pts) const
{
    // Intend for a 10% margin/borders.
    // As always with SVG, increasing X is to the right, and increase Y is DOWN the screen.
    // So we typically invert (*-1) the Y coordinates from our optics modelling to match the SVG model.
    // Relocate the origin to be the center of the mirror - and place this at the center of the viewport
    const double border_proportion = 0.10;
    const double canvas_size = 800;
    const double line_width = 1; // Thinest
    const double mirror_line_width = 0.5;

    const double something = 32;
    const double border_size = 1*something * border_proportion;

    double from_top_border   =  1*something;
    double from_bottom_border= -1*something;
    double from_left_border  = -1*something;
    double from_right_border =  1*something;
    CoordConverter cc;
    cc.DefineFrom( offset_X+from_left_border  - border_size, offset_Y+from_bottom_border - border_size,
                   offset_X+from_right_border + border_size, offset_Y+from_top_border    + border_size);
    cc.DefineTo  (0, 800, 800, 0);

    const std::deque<TracedRay>* traced_rays[] = { &m_TopRays, &m_BotRays };

    if (first_call) {
        fprintf(fout, "<?xml version=\"1.0\" standalone=\"yes\"?>\n");
        fprintf(fout, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
        fprintf(fout, "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" ");
        fprintf(fout, "width=\"%g\" height=\"%g\" id=\"svg\" viewBox=\"%g %g %g %g\">\n",
            canvas_size, canvas_size,
            0.0,0.0, canvas_size, canvas_size
            );

        if (1) { // Use inline CSS?
            fprintf(fout, "<defs>\n");
            fprintf(fout, "<style type=\"text/css\"><![CDATA[\n");

            fprintf(fout, "path{ shape-rendering : crispEdges; }\n");

            // For the sun's ray-tracing: T=top, B=bottom. Reflected=both start and end points are on mirror.
            fprintf(fout, ".ray_incidentT  { stroke-linecap: round; stroke: red; }\n" );
            fprintf(fout, ".ray_reflectedT { stroke-linecap: round; stroke: pink; }\n" );
            fprintf(fout, ".ray_finalT     { stroke-linecap: round; stroke: blue; }\n" );
            fprintf(fout, ".ray_incidentB  { stroke-linecap: round; stroke: orange; }\n" );
            fprintf(fout, ".ray_reflectedB { stroke-linecap: round; stroke: purple; }\n" );
            fprintf(fout, ".ray_finalB     { stroke-linecap: round; stroke: green; }\n" );

            fprintf(fout, ".bboxT          { stroke-width: 0.5; stroke: red; fill: none; }\n" );
            fprintf(fout, ".bboxB          { stroke-width: 0.5; stroke: orange; fill: none; }\n" );
            fprintf(fout, ".intersectT     { stroke-width: 0.5; stroke: red; fill: none; }\n" );
            fprintf(fout, ".intersectB     { stroke-width: 0.5; stroke: orange; fill: none; }\n" );

            fprintf(fout, ".debug          { stroke-width: 0.25; stroke: black; }\n" );
            fprintf(fout, ".debug_1        { stroke-width: 0.5; stroke: red; }\n" );
            fprintf(fout, ".debug_2        { stroke-width: 0.5; stroke: red; }\n" );


            int ray_index = 0;
            for (int tri=0; tri<sizeof(traced_rays)/sizeof(traced_rays[0]); tri++) {
                for (auto it = traced_rays[tri]->begin(); it != traced_rays[tri]->end(); ++it) {
                    ray_index++;
                    fprintf(fout, ".ray_%d { stroke-width: 0.5; }\n", ray_index );
                }
            }

            fprintf(fout, "]]></style>\n");

            fprintf(fout, "</defs>\n");

        } // inline CSS
    } // first_call

    if ( ! debug_segments.empty() ) {
        int ii=0;
        for (auto it = debug_segments.begin(); it != debug_segments.end(); ++it) {
            ii++;
            fprintf(fout, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" class=\"debug debug_%d\"/>\n",
                cc.X(it->first.x()), cc.Y(it->first.y()), cc.X(it->second.x()), cc.Y(it->second.y()), ii);
        } // for
    }

    const char* RayType = "TBabcdefghijklmnopqrstuvwxyz";
    if(!animate || first_call) {
        // Mirror's Center cross-marks
        fprintf(fout, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" style=\"stroke-width: 0.5; stroke: #888888;\"/>\n",
            cc.X(0-m_radius/10), cc.Y(0), cc.X(0+m_radius/10), cc.Y(0) );
        fprintf(fout, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" style=\"stroke-width: 0.5; stroke: #888888;\"/>\n",
            cc.X(0), cc.Y(0-m_radius/10), cc.X(0), cc.Y(0+m_radius/10) );

        // Mirror - show full circle as a dashed line
        fprintf(fout, "<circle cx=\"%g\" cy=\"%g\" r=\"%g\" stroke-dasharray=\"2, 5\" style=\"stroke-width: %g; stroke: grey; fill: none;\"/>\n",
            cc.X(0), cc.Y(0), m_radius*cc.Scale(), mirror_line_width);

        // arc - show as two overlapping arcs - a thicker one (darker) and a lighter/thinner one to indicae the reflective surface.
        fprintf(fout, "<path d=\"M%g,%g A %g %g  0 %d 0 %g %g\" style=\"stroke-width: 2; stroke: grey; fill: none;\"/>\n",
            cc.X(m_min_normal_pt.x()), cc.Y(m_min_normal_pt.y()), m_radius*cc.Scale(), m_radius*cc.Scale(), 
            (m_max_normal_dir - m_min_normal_dir) > 180.0 ? 1 : 0,
            cc.X(m_max_normal_pt.x()), cc.Y(m_max_normal_pt.y()));
        fprintf(fout, "<path d=\"M%g,%g A %g %g  0 %d 0 %g %g\" style=\"stroke-width: %g; stroke: silver; fill: none;\"/>\n",
            cc.X(m_min_normal_pt.x()), cc.Y(m_min_normal_pt.y()), m_radius*cc.Scale(), m_radius*cc.Scale(), 
            (m_max_normal_dir - m_min_normal_dir) > 180.0 ? 1 : 0,
            cc.X(m_max_normal_pt.x()), cc.Y(m_max_normal_pt.y()), mirror_line_width);
        // Little circles to mark the ends (and center) of the arc
        fprintf(fout, "<circle cx=\"%g\" cy=\"%g\" r=\"%g\" style=\"stroke-width: %g; stroke: teal; fill: none;\"/>\n",
                cc.X(m_min_normal_pt.x()), cc.Y(m_min_normal_pt.y()), 1.0, mirror_line_width);
        fprintf(fout, "<circle cx=\"%g\" cy=\"%g\" r=\"%g\" style=\"stroke-width: %g; stroke: teal; fill: none;\"/>\n",
                cc.X(m_max_normal_pt.x()), cc.Y(m_max_normal_pt.y()), 1.0, mirror_line_width);
        fprintf(fout, "<circle cx=\"%g\" cy=\"%g\" r=\"%g\" style=\"stroke-width: %g; stroke: teal; fill: none;\"/>\n",
                cc.X(m_MidArcPt.x()), cc.Y(m_MidArcPt.y()), 1.0, mirror_line_width);

        // Screen
        if ( (m_screen.first.x() != BadValue) && (m_screen.first.y() != BadValue) && (m_screen.second.x() != BadValue) && (m_screen.second.y() != BadValue) ) {
            fprintf(fout, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" style=\"stroke-width: 2.0; stroke: purple;\"/>\n",
                cc.X( m_screen.first.x() ), cc.Y( m_screen.first.y() ), cc.X( m_screen.second.x() ), cc.Y( m_screen.second.y() ) );
        }

        // Stencil
        int max_stencil_index = m_stencils.size()-1;
        for (int ii=0; ii <= max_stencil_index; ii++) {
            fprintf(fout, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" style=\"stroke-width: 1.5; stroke: pink;\"/>\n",
                cc.X( m_stencils[ii].first.x() ), cc.Y( m_stencils[ii].first.y() ), cc.X( m_stencils[ii].second.x() ), cc.Y( m_stencils[ii].second.y() ) );
        }


        fprintf(fout, "<text x=\"%g\" y=\"%g\" id=\"title_text\" font-size=\"12\">%s</text>\n", cc.X(10), cc.Y(30), title.c_str() );
        fprintf(fout, "<text x=\"%g\" y=\"%g\" id=\"more_text\" font-size=\"12\">%s</text>\n", cc.X(10), cc.Y(33), "" );

        // Walk thru each traced ray
        /* Note traced ray may result in numerous segments:
         * incident=from sun (or rather, from the edge of the viewBox) to the mirror,
         * reflected=starts and stops on the mirror - there can be multiple of these per traced ray
         * final=starts on the mirror and proceeds away (can hit screen, stencil, or proceed out of viewBox).
         *
         * We use a couple of CSS classes and a tag/id per segment:
         * class: one of ray_incidentT, ray_incidentB, ray_reflectedT, ray_reflectB, ray_finalT, rayfinalB - used for CSS styles (width and color, etc.)
         * class: something like ray_7 - all segments associated with a single ray-trace have the same class-name. Used for Javascript to highlight
         *      all segments on a ray on a mouseover event.
         * id(tag): something like ray_sun_7, ray_7_2, ray_final_7 - used to allow javascript to locate the segment in the DOM to support animation. The
         *      2nd form (ray_7_2), means the 2nd reflected ray of traced-ray #7.
         */
        static int ray_index = 0; // For class creation
        for (int tri=0; tri<sizeof(traced_rays)/sizeof(traced_rays[0]); tri++) { 
            for (auto it = traced_rays[tri]->begin(); it != traced_rays[tri]->end(); ++it) {
                if ( ! it->m_StrikePts.empty() ) {
                ray_index++;
                Point sun_far_pt, reflected_far_pt;
                Calc_far_point( it->m_StrikePts.front(), 180+it->m_sun_dir,     sun_far_pt,      from_left_border, from_top_border, from_right_border);
                Calc_far_point( it->m_StrikePts.back(),     it->m_reflect_dir, reflected_far_pt, from_left_border, from_top_border, from_right_border);

                TerminateRay( Segment(it->m_StrikePts.back(), reflected_far_pt), m_screen, reflected_far_pt );

                int max_index = m_stencils.size()-1;
                for (int ii=0; ii <= max_index; ii++) {
                    TerminateRay( Segment( it->m_StrikePts.back(), reflected_far_pt), m_stencils[ii], reflected_far_pt );
                }

                int segment_index=0;
                // Incident ray from the sun
                fprintf(fout, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" class=\"ray_incident%c ray_%d\" id=\"ray_sun_%d\"/>\n",
                        cc.X(sun_far_pt.x()), cc.Y(sun_far_pt.y()), cc.X(it->m_StrikePts.front().x()), cc.Y(it->m_StrikePts.front().y()),
                        RayType[tri], ray_index, ray_index );

                if (it->m_ray_status >= TracedRay::NStrike) { // reflected ray
                    Point previous_pt = it->m_StrikePts.front();
                    for (auto rr = it->m_StrikePts.begin(); rr != it->m_StrikePts.end(); ++rr) {
                        const Point& this_pt = *rr;
                        if (this_pt == previous_pt) continue;
                        fprintf(fout, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" class=\"ray_reflected%c ray_%d\" id=\"ray_%d_%d\"/>\n",
                            cc.X(previous_pt.x()), cc.Y(previous_pt.y()),
                            cc.X(this_pt.x()), cc.Y(this_pt.y()), RayType[tri], ray_index, ray_index, ++segment_index);
                        previous_pt = this_pt;
                    }
                    fprintf(fout, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" class=\"ray_final%c ray_%d\" id=\"ray_final_%d\"/>\n",
                        cc.X(it->m_StrikePts.back().x()), cc.Y(it->m_StrikePts.back().y()),
                        cc.X(reflected_far_pt.x()), cc.Y(reflected_far_pt.y()),
                        RayType[tri], ray_index, ray_index);

                    // Indicate reflection point
                    if (1) {
                        double normal_dir = Direction(Point(0,0), it->m_StrikePts.back());
                        Point pt1 = Find2ndPoint(it->m_StrikePts.back(), normal_dir,  m_radius/20 );
                        Point pt2 = Find2ndPoint(it->m_StrikePts.back(), normal_dir, -m_radius/20 );
                        fprintf(fout, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" style=\"stroke-width: 0.5; stroke: silver;\"/>\n",
                            cc.X(pt1.x()), cc.Y(pt1.y()), cc.X(pt2.x()), cc.Y(pt2.y()) );
                    }
                } // reflected ray
                }
            } // for it
        } // for tri

        if (focal_pts) {
            int intersect_count = 0;
            for (auto it = m_TopIntersectionPts.begin(); it != m_TopIntersectionPts.end(); ++it)
                fprintf(fout, "<circle cx=\"%g\" cy=\"%g\" r=\"%g\" class=\"intersectT\" id=\"intersectT_%d\"/>\n",
                    cc.X(it->x()), cc.Y(it->y()), 1.0, intersect_count++);
            intersect_count = 0;
            for (auto it = m_BotIntersectionPts.begin(); it != m_BotIntersectionPts.end(); ++it)
                fprintf(fout, "<circle cx=\"%g\" cy=\"%g\" r=\"%g\" class=\"intersectB\" id=\"intersectB_%d\"/>\n",
                    cc.X(it->x()), cc.Y(it->y()), 1.0, intersect_count++ );
        }

        if (do_boxes) {
            { // Bounding-box rectangle
                double x1 = cc.X( m_TopIntersectionBBox.MinX() );
                double x2 = cc.X( m_TopIntersectionBBox.MaxX() );
                double y1 = cc.Y( m_TopIntersectionBBox.MinY() );
                double y2 = cc.Y( m_TopIntersectionBBox.MaxY() );
                double X,Y,width,height;
                if (x1 > x2) { X = x2; width  = x1-x2; } else { X = x1; width  = x2-x1; }
                if (y1 > y2) { Y = y2; height = y1-y2; } else { Y = y1; height = y2-y1; }
                fprintf(fout, "<rect x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\" class=\"bboxT\" id=\"top_rec\"/>\n",
                        X,Y, width, height);
            }

            { // Bounding-box rectangle
                double x1 = cc.X( m_BotIntersectionBBox.MinX() );
                double x2 = cc.X( m_BotIntersectionBBox.MaxX() );
                double y1 = cc.Y( m_BotIntersectionBBox.MinY() );
                double y2 = cc.Y( m_BotIntersectionBBox.MaxY() );
                double X,Y,width,height;
                if (x1 > x2) { X = x2; width  = x1-x2; } else { X = x1; width  = x2-x1; }
                if (y1 > y2) { Y = y2; height = y1-y2; } else { Y = y1; height = y2-y1; }
                fprintf(fout, "<rect x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\" class=\"bboxB\" id=\"bot_rec\"/>\n",
                        X,Y, width, height);
            }
        }
    }


    if (animate) {
        if (first_call) {
            fprintf(fout, "<script type=\"text/ecmascript\"><![CDATA[\n");
            fprintf(fout, "var line_data = [\n");
        }


        // Walk thru each traced ray
        int ray_index = 0; // For CSS class creation
        for (int tri=0; tri<sizeof(traced_rays)/sizeof(traced_rays[0]); tri++) { 
            for (auto it = traced_rays[tri]->begin(); it != traced_rays[tri]->end(); ++it) {
                ray_index++;
                fprintf(fout, "\t[ 'more_text', 'text', 'Sun Angle=%g' ],\n", it->m_sun_dir );
                Point sun_far_pt, reflected_far_pt;
                Calc_far_point( it->m_StrikePts.front(), 180+it->m_sun_dir,     sun_far_pt,      from_left_border, from_top_border, from_right_border);
                Calc_far_point( it->m_StrikePts.back(),     it->m_reflect_dir, reflected_far_pt, from_left_border, from_top_border, from_right_border);

                TerminateRay( Segment(it->m_StrikePts.back(), reflected_far_pt), m_screen, reflected_far_pt );

                int max_index = m_stencils.size()-1;
                for (int ii=0; ii <= max_index; ii++) {
                    TerminateRay( Segment(it->m_StrikePts.back(), reflected_far_pt), m_stencils[ii], reflected_far_pt );
                }


                fprintf(fout, "\t[ 'ray_sun_%d', 'line', 'ray_incident%c', %g,%g,  %g,%g ],\n", 
                    ray_index, RayType[tri], cc.X(sun_far_pt.x()), cc.Y(sun_far_pt.y()), cc.X(it->m_StrikePts.front().x()), cc.Y(it->m_StrikePts.front().y()) );

                int segment_index = 0;
                if (it->m_ray_status >= TracedRay::NStrike) { // reflected ray
                    Point previous_pt = it->m_StrikePts.front();
                    for (auto rr = it->m_StrikePts.begin(); rr != it->m_StrikePts.end(); ++rr) {
                        const Point& this_pt = *rr;
                        if (this_pt == previous_pt) continue;
                        fprintf(fout, "\t[ 'ray_%d_%d', 'line', 'ray_reflected%c', %g,%g,  %g,%g ],\n", 
                            ray_index, ++segment_index, RayType[tri],
                            cc.X(previous_pt.x()), cc.Y(previous_pt.y()), cc.X(this_pt.x()), cc.Y(this_pt.y()) );
                        previous_pt = this_pt;
                    }
                    fprintf(fout, "\t[ 'ray_final_%d', 'line', 'ray_final%c', %g,%g,  %g,%g ],\n", 
                        ray_index, RayType[tri],
                        cc.X(it->m_StrikePts.back().x()), cc.Y(it->m_StrikePts.back().y()), cc.X(reflected_far_pt.x()), cc.Y(reflected_far_pt.y()) );

                } // reflected ray
            } // for it
        } // for tri

        if (focal_pts) {
            int intersect_count = 0;
            for (auto it = m_TopIntersectionPts.begin(); it != m_TopIntersectionPts.end(); ++it)
                fprintf(fout, "\t[ 'intersectT_%d', 'circle', %g, %g, %g ],\n",
                    intersect_count++, cc.X(it->x()), cc.Y(it->y()), 1.0 );
            intersect_count = 0;
            for (auto it = m_BotIntersectionPts.begin(); it != m_BotIntersectionPts.end(); ++it)
                fprintf(fout, "\t[ 'intersectB_%d', 'circle', %g, %g, %g ],\n",
                    intersect_count++, cc.X(it->x()), cc.Y(it->y()), 1.0 );
        }


        if (do_boxes) {
            { // Top Bounding-box rectangle
                double x1 = cc.X( m_TopIntersectionBBox.MinX() );
                double x2 = cc.X( m_TopIntersectionBBox.MaxX() );
                double y1 = cc.Y( m_TopIntersectionBBox.MinY() );
                double y2 = cc.Y( m_TopIntersectionBBox.MaxY() );
                double X,Y,width,height;
                if (x1 > x2) { X = x2; width  = x1-x2; } else { X = x1; width  = x2-x1; }
                if (y1 > y2) { Y = y2; height = y1-y2; } else { Y = y1; height = y2-y1; }
                fprintf(fout, "\t[ 'top_rec', 'rect', %g, %g, %g, %g ],\n", X,Y, width, height);
            }

            { // Bot Bounding-box rectangle
                double x1 = cc.X( m_BotIntersectionBBox.MinX() );
                double x2 = cc.X( m_BotIntersectionBBox.MaxX() );
                double y1 = cc.Y( m_BotIntersectionBBox.MinY() );
                double y2 = cc.Y( m_BotIntersectionBBox.MaxY() );
                double X,Y,width,height;
                if (x1 > x2) { X = x2; width  = x1-x2; } else { X = x1; width  = x2-x1; }
                if (y1 > y2) { Y = y2; height = y1-y2; } else { Y = y1; height = y2-y1; }
                fprintf(fout, "\t[ 'bot_rec', 'rect', %g, %g, %g, %g ],\n", X,Y, width, height);
            }
        }

        fprintf(fout, "\t[ 'next', 0 ],\n");



        if (last_call) {
            fprintf(fout, "];\n");

            fprintf(fout, "var id = setInterval(intervalCallback, %d);\n", animate_interval_ms);
            fprintf(fout, "var row_index = 0;\n");
            fprintf(fout, "var prev_index = row_index;\n");
            fprintf(fout, "function intervalCallback() {\n");
            fprintf(fout, "\twhile (1) {\n");
            fprintf(fout, "\t\tif(prev_index >= line_data.length) { prev_index = 0; }\n");
            fprintf(fout, "\t\tif (line_data[prev_index][0] == 'next') { break; }\n");
            fprintf(fout, "\t\tvar ref_element = document.getElementById( line_data[prev_index][0] );\n");
            fprintf(fout, "\t\tif (ref_element != null) {\n");
            fprintf(fout, "\t\t\tref_element.setAttribute('display','none');\n");
            fprintf(fout, "\t\t}\n");
            fprintf(fout, "\t\tprev_index++;\n");
            fprintf(fout, "\t}\n");
            fprintf(fout, "\tprev_index = row_index;\n");
            fprintf(fout, "\twhile (1) {\n");
            fprintf(fout, "\t\tif(row_index >= line_data.length) { row_index = 0; }\n");
            fprintf(fout, "\t\tif (line_data[row_index][0] == 'next') { row_index++; break; }\n");
            fprintf(fout, "\t\tvar ref_element = document.getElementById( line_data[row_index][0] );\n");
            fprintf(fout, "\t\tif (ref_element == null) {\n");
            fprintf(fout, "\t\t\tvar xmlns = \"http://www.w3.org/2000/svg\";\n");
            fprintf(fout, "\t\t\tref_element = document.createElementNS(xmlns, line_data[row_index][1]);\n");
            fprintf(fout, "\t\t\tref_element.setAttribute('id',  line_data[row_index][0] );\n");
            fprintf(fout, "\t\tvar svg = document.getElementById( 'svg' );\n");
            fprintf(fout, "\t\t\tsvg.appendChild(ref_element);\n");
            fprintf(fout, "\t\t\tref_element.setAttribute('style',  'stroke-width: 1; stroke: pink;');\n");
            fprintf(fout, "\t\t}\n");
            fprintf(fout, "\t\tref_element.setAttribute('display','1');\n");
            fprintf(fout, "\t\tswitch( ref_element.tagName.toLowerCase() ) {\n");
            fprintf(fout, "\t\t\tcase 'line':\n");
            fprintf(fout, "\t\t\t\tref_element.classList.add(line_data[row_index][2]);\n");
            fprintf(fout, "\t\t\t\tref_element.setAttribute('x1',line_data[row_index][3]);\n");
            fprintf(fout, "\t\t\t\tref_element.setAttribute('y1',line_data[row_index][4]);\n");
            fprintf(fout, "\t\t\t\tref_element.setAttribute('x2',line_data[row_index][5]);\n");
            fprintf(fout, "\t\t\t\tref_element.setAttribute('y2',line_data[row_index][6]);\n");
            fprintf(fout, "\t\t\t\tbreak;\n");
            fprintf(fout, "\t\t\tcase 'circle':\n");
            fprintf(fout, "\t\t\t\tref_element.setAttribute('cx',line_data[row_index][2]);\n");
            fprintf(fout, "\t\t\t\tref_element.setAttribute('cy',line_data[row_index][3]);\n");
            fprintf(fout, "\t\t\t\tref_element.setAttribute('r', line_data[row_index][4]);\n");
            fprintf(fout, "\t\t\t\tbreak;\n");
            fprintf(fout, "\t\t\tcase 'rect':\n");
            fprintf(fout, "\t\t\t\tref_element.setAttribute('x', line_data[row_index][2]);\n");
            fprintf(fout, "\t\t\t\tref_element.setAttribute('y', line_data[row_index][3]);\n");
            fprintf(fout, "\t\t\t\tref_element.setAttribute('width', line_data[row_index][4]);\n");
            fprintf(fout, "\t\t\t\tref_element.setAttribute('height', line_data[row_index][5]);\n");
            fprintf(fout, "\t\t\t\tbreak;\n");
            fprintf(fout, "\t\t\tcase 'text':\n");
            fprintf(fout, "\t\t\t\tref_element.textContent=line_data[row_index][2];\n");
            fprintf(fout, "\t\t\t\tbreak;\n");
            fprintf(fout, "\t\t}\n");
            fprintf(fout, "\t++row_index;\n");
            fprintf(fout, "\t}\n");

            fprintf(fout, "}\n");

            fprintf(fout, "// ]]>\n</script>\n");
        }
    } // if animate


    if (last_call) {
        if (1) { // Highlight a ray (all segments/lines) on mouseover
            const int js_console_debug = 0;
            fprintf(fout, "<script type=\"text/javascript\">\n// <![CDATA[\n");
            fprintf(fout, "var all_rays = document.querySelectorAll('[class^=ray_]');\n");
            if (js_console_debug) fprintf(fout, "console.log('all_rays.length=', all_rays.length);\n");
            fprintf(fout, "var ii;\n");
            fprintf(fout, "for (ii=0; ii<all_rays.length;ii++) {\n");
            fprintf(fout, "\tall_rays[ii].addEventListener(\"mouseover\", rays_mouseover, false);\n");
            fprintf(fout, "\tall_rays[ii].addEventListener(\"mouseout\",  rays_mouseout,  false);\n");
            if (js_console_debug) fprintf(fout, "console.log('ii=', ii, 'elem=', all_rays[ii], ', class=', all_rays[ii].className.baseVal);\n");
            fprintf(fout, "}\n");
            fprintf(fout, "function find_ray_class(e) {\n");
            fprintf(fout, "\tvar class_names= e.target.className.baseVal.split(' ');\n");
            fprintf(fout, "\tvar ray_class_name;\n");
            fprintf(fout, "\tfor (var ii=0; ii<class_names.length; ii++) {\n");
            fprintf(fout, "\t\tvar regular_expression = /^ray_\\d+$/;\n");
            fprintf(fout, "\t\tif (regular_expression.test(class_names[ii])) {\n");
            fprintf(fout, "\t\t\treturn class_names[ii];\n");
            fprintf(fout, "\t\t}\n");
            fprintf(fout, "\t}\n");
            fprintf(fout, "\treturn '';\n");
            fprintf(fout, "}\n");
            fprintf(fout, "function rays_mouseover(e) {\n");
            fprintf(fout, "\tvar ray_class_name = find_ray_class(e);\n");
            fprintf(fout, "\tvar rays = document.getElementsByClassName(  ray_class_name );\n");
            fprintf(fout, "\tfor (var ii=0; ii<rays.length; ii++) {\n");
            fprintf(fout, "\t\trays[ii].style['stroke-width'] = 3;\n");
            fprintf(fout, "\t}\n");
            fprintf(fout, "\tvar mouseover_text_element = document.getElementById( 'more_text' );\n");
//            fprintf(fout, "\tif (mouseover_text_element)\n");
            fprintf(fout, "\t\t\tmouseover_text_element.textContent = ray_class_name;\n");
            fprintf(fout, "}\n");
            fprintf(fout, "function rays_mouseout(e) {\n");
            fprintf(fout, "\tvar ray_class_name = find_ray_class(e);\n");
            fprintf(fout, "\tvar rays = document.getElementsByClassName(  ray_class_name );\n");
            fprintf(fout, "\tfor (var ii=0; ii<rays.length; ii++) {\n");
            fprintf(fout, "\t\trays[ii].style['stroke-width'] = '';\n");
            fprintf(fout, "\t}\n");
            fprintf(fout, "}\n");
            fprintf(fout, "// ]]>\n</script>\n");
        }
        fprintf(fout, "</svg>\n");
    }


}


struct arg_iterator
{
    double from, to, increment;
    std::string parameter_name;
    std::deque<double> value_list;

    double GetValue(int index, int &bad_index) const;
};
double arg_iterator::GetValue(int index, int &bad_index) const
{
    bad_index = 0;
    if (value_list.empty()) { // use from, to, increment
        if (index == 0) return from;
        if (increment == 0) {
            if ((from == to) || (index > 1)) bad_index=1;
            return to;
        }    
        double value = from + index * increment;
             if ((from < to) && (value > to)) bad_index=1;
        else if ((from > to) && (value < to)) bad_index=1;
        return value;
    }
    // Else
    if (index < value_list.size()) return value_list[index];
    bad_index = 1;
    return 0;
}

void GrabIteratorArgs( int &arg_index, int argc, const char* argv[], const char* param_name, arg_iterator &arg_it)
{
    arg_it.parameter_name = param_name;

    if (isdigit(argv[arg_index+1][0]) || ((argv[arg_index+1][0] == '-') && isdigit(argv[arg_index+1][1]))) {
        arg_index++;
        arg_it.increment = 0;
        arg_it.from  = arg_it.to = atof(argv[arg_index]);
    }
    if (isdigit(argv[arg_index+1][0]) || ((argv[arg_index+1][0] == '-') && isdigit(argv[arg_index+1][1]))) {
        arg_index++;
        arg_it.to = atof(argv[arg_index]);
    }
    if (isdigit(argv[arg_index+1][0]) || ((argv[arg_index+1][0] == '-') && isdigit(argv[arg_index+1][1]))) {
        arg_index++;
        double value = atof(argv[arg_index]);
        if (value < arg_it.to) {
            arg_it.increment = value;
            return;
        }
        // else take as a list of values
        arg_it.value_list.push_back(arg_it.from);
        arg_it.value_list.push_back(arg_it.to);
        arg_it.value_list.push_back(value);
        arg_it.from = arg_it.to = 0.0;
        while (    argv[arg_index+1] &&
              (argv[arg_index+1][0] != '\0') &&
              (isdigit(argv[arg_index+1][0]) || ((argv[arg_index+1][0] == '-') && isdigit(argv[arg_index+1][1]))) ) {
            arg_index++;
            arg_it.value_list.push_back( atof(argv[arg_index]) );
        }
    }
}


void GenerateReport(const std::deque<TheData> &td,const std::string& row_param, const std::string& col_param, const std::string& value_param)
{
    std::set<double> row_values_set;
    std::set<double> col_values_set;
    for (auto it = td.begin(); it != td.end(); ++it) {
        double row_value = it->GetValue(row_param);
        row_values_set.insert( row_value );
        double col_value = it->GetValue(col_param);
        col_values_set.insert( col_value );
    }

    int row_index = 0;
    std::map<double,int> row_indices;
    std::deque<double> row_values( row_values_set.size() );
    for (auto it = row_values_set.begin(); it != row_values_set.end(); ++it) {
        row_indices.insert( std::pair<double,int>( *it, row_index ) );
        row_values[row_index] = *it;
        row_index++;
    }

    int col_index = 0;
    std::map<double,int> col_indices;
    std::deque<double> col_values( col_values_set.size() );
    for (auto it = col_values_set.begin(); it != col_values_set.end(); ++it) {
        col_indices.insert( std::pair<double,int>( *it, col_index ) );
        col_values[col_index] = *it;
        col_index++;
    }


    std::deque< std::deque<double> > two_d(row_index);
    for (auto it = two_d.begin(); it != two_d.end(); ++it)
        it->resize(col_index);

    for (auto it = td.begin(); it != td.end(); ++it) {
        double row_value = it->GetValue(row_param);
        auto row_it = row_indices.find( row_value );
        int row_index = row_it->second;

        double col_value = it->GetValue(col_param);
        auto col_it = col_indices.find( col_value );
        int col_index = col_it->second;

        double value = it->GetValue(value_param);
        two_d[row_index][col_index] = value;
    }



    printf("%s", row_param.c_str());
    for (int ii=0; ii<col_index; ii++) {
        printf(",%s_%g",value_param.c_str(), col_values[ii]);
    }
    printf("\n");
    for (int rr=0; rr<row_index; rr++) {
        printf("%g", row_values[rr] );
        for (int cc=0; cc<col_index; cc++) {
            if (two_d[rr][cc] == BadValue)    printf(",");
            else                printf(",%g",  two_d[rr][cc]);
        }
        printf("\n");
    }
}


void usage(const char* program_name)
{
    printf("Usage: %s [-next | -iterate] [-r ...] [-d ...] [-s[aA] ...] [-sw <value>] [-svg [<filename>]] [-csv] [-pupil] [-animate]\n", program_name);
    printf("\tRuns a reverse ray-tracing for a system involving the sun, a (convex) mirror and (sometimes) an observer.\n");
    printf("\t-next: may be used to specify a number of test-conditions. Separates the -r, -d, -sa  (and -sA) arguments. May be used multiple\n");
    printf("\t\ttimes to specify multiple test-conditions. Such as %s -r 1 -d 1.1 -sA 10 -next -r 1 -d 1.1 -sA 20 -next -r 1 -d 1.1 -sA 30\n", program_name);
    printf("\t\tNot compatible with the -iterate argument.\n");
    printf("\t-iterate: Use before -r, -d and/or -sa (and -sA) arguments. In this case each of these arguments may be followed by a series of values:\n");
    printf("\t\t<value1>: Only the single value is used in the iteration.\n");
    printf("\t\t<value1> <value2>: Iterates twice, first with value1 and then with value2\n");
    printf("\t\t<value1> <value2> <value3> (if value2>value3): Iterate from value1 to <=value2, incrementing by value3.\n");
    printf("\t\t<value1> <value2> <value3> ... (if value2<value3): taken as a series of values.\n");
    printf("\t\tNote that -iterate and -next are not compatible with each other.\n");
    printf("\t-r <value> [...]: Defines the radius if the mirror (defaults to 1). See above comments regarding -next and -iterate.\n");
    printf("\t-d <value> [...]: Defines the distance from the observer to the center-of-curvature of the mirror. Must be >radius. See\n");
    printf("\t\tabove comments.\n");
    printf("\t-sa <value> [...]: Defines the the angle of the sun's rays (i.e. 0 is horizontal to the left, 270 vertically down).\n");
    printf("\t-sA <value> [...]: An alterative to -sa - defines the sun's altitude (is 180 more/less than -sa): 90 is vertically down.\n");
    printf("\t-sw <value>: Defines the angular width of the sun in degrees. Defaults to 0.5. The above comments are not applicable.\n");
    printf("\t-csv: generates results in a comma-separated-values format on standard-output.\n");
    printf("\t-svg <filename>: generates SVG graphics in the indicated filename. Typically observer in a browser.\n");
    printf("\t-animate: Adds animation to the SVG (per the test-cases identified with -next or -iterate).\n");
    printf("\t-pupil: (experimental) - perform and report on the the entrance pupil calculations.\n");
    printf("\n");

}

int main(int argc, const char* argv[])
{
    std::deque<TheData> td;
    td.resize(1);
    int tdi = 0; // Number of elements used in td

    int do_iterate = 0;
    arg_iterator arg_it[3];
    int aii = 0; // Number of elements used in arg_it

    int do_svg = 0;
    std::string svg_filename = "output.svg";
    int animate = 0;
    int do_csv = 0;
    int calc_pupil = 0;
    int num_rays = 3;

    double offset_X=0, offset_Y=0;

    double default_radius = BadValue;
    double default_distance = BadValue;
    double default_sun_altitude_dir = BadValue;
    double default_min_normal_dir = BadValue;
    double default_max_normal_dir = BadValue;
    double default_mirror_width = BadValue;
    int animate_interval_ms = 200;

    int do_convex = 0;
    int do_concave = 0;
    int do_boxes = 0;
    int focal_pts = 0;



    bool do_csv2 = 0;
    int ray_report = 0;
    int do_reverse_trace = 0;
    std::string csv2_row, csv2_col, csv2_val;
    std::string title;
    for (int ii=1; ii<argc; ii++) {
             if (strcmp(argv[ii], "-svg"     ) == 0) { do_svg++; if (((ii+1)<argc) && (argv[ii+1][0] != '-')) { ii++; svg_filename = argv[ii]; }}
        else if (strcmp(argv[ii], "-help"    ) == 0) { usage(argv[0]); exit(0); }
        else if (strcmp(argv[ii], "-title"   ) == 0) { title = argv[++ii]; }
        else if (strcmp(argv[ii], "-convex"  ) == 0) { do_convex=1; do_concave=0; td[tdi].m_IsConvex = 1; }
        else if (strcmp(argv[ii], "-concave" ) == 0) { do_convex=0; do_concave=1; td[tdi].m_IsConvex = 0; }
        else if (strcmp(argv[ii], "-debug"   ) == 0) { dvo_debug++; if (((ii+1)<argc) && (argv[ii+1][0] != '-')) { ii++; dvo_debug = atoi(argv[ii]); }}
        else if (strcmp(argv[ii], "-test"    ) == 0) { int result = CoordConverter::Test(); exit(result); }
        else if (strcmp(argv[ii], "-brighttable")==0){ int brighttable(); int result = brighttable(); exit(result); }
        else if (strcmp(argv[ii], "-report"  ) == 0) { ray_report++; }
        else if (strcmp(argv[ii], "-box"     ) == 0) { do_boxes++; }
        else if (strcmp(argv[ii], "-focal_pts")== 0) { focal_pts++; }
        else if (strcmp(argv[ii], "-animate" ) == 0) { animate++; }
        else if (strcmp(argv[ii], "-interval") == 0) { animate_interval_ms = atol(argv[++ii]); }
        else if (strcmp(argv[ii], "-reverse" ) == 0) { do_reverse_trace++; }
        else if (strcmp(argv[ii], "-pupil"   ) == 0) { calc_pupil++; }
        else if (strcmp(argv[ii], "-iterate" ) == 0) { do_iterate++; }
        else if (strcmp(argv[ii], "-sw"      ) == 0) { ii++; td[tdi].m_sun_width_ang    = atof(argv[ii]); }
        else if (strcmp(argv[ii], "-nr"      ) == 0) { ii++; num_rays    = atoi(argv[ii]); }
        else if (strcmp(argv[ii], "-csv"     ) == 0) { do_csv++; }
        else if (strcmp(argv[ii], "-csv2"    ) == 0) {
            // Expect 3 more arguments - name of row-index (independent variable #1), name of col-index (independent variable #2) and value
            // Names are as supported in TheData::GetValue()
            if (argc < (ii+3)) fprintf(stderr,"ERROR: Expecting 3 fields for the -csv2 argument (row index, col index and value\n");
            do_csv2 = 1;
            csv2_row = argv[++ii];
            csv2_col = argv[++ii];
            csv2_val = argv[++ii];
             } 
        else if (strcmp(argv[ii], "-screen" ) == 0) {
            if (argc < (ii+4)) fprintf(stderr,"ERROR: Expecting 4 fields for the %s argument: End points (each with an X,Y value) for the screen.\n", argv[ii]);
            double X1 = atof( argv[++ii] );
            double Y1 = atof( argv[++ii] );
            double X2 = atof( argv[++ii] );
            double Y2 = atof( argv[++ii] );
            td[tdi].m_screen = Segment( Point(X1,Y1), Point(X2,Y2) );
        }
        else if (strcmp(argv[ii], "-stencil" ) == 0) {
            if (argc < (ii+4)) fprintf(stderr,"ERROR: Expecting 4 fields for the %s argument: End points (each with an X,Y value) for one line of the stencil.\n", argv[ii]);
            double X1 = atof( argv[++ii] );
            double Y1 = atof( argv[++ii] );
            double X2 = atof( argv[++ii] );
            double Y2 = atof( argv[++ii] );
            td[tdi].m_stencils.push_back(  Segment( Point(X1,Y1), Point(X2,Y2) ) );
        }
        else if (strcmp(argv[ii], "-offset" ) == 0) {
            if (argc < (ii+2)) fprintf(stderr,"ERROR: Expecting 2 fields for the %s argument", argv[ii]);
            offset_X = atof( argv[++ii] );
            offset_Y = atof( argv[++ii] );
        }
        else if (strcmp(argv[ii], "-target" ) == 0) {
            if (argc < (ii+2)) fprintf(stderr,"ERROR: Expecting 2 fields for the %s argument", argv[ii]);
            double X = atof( argv[++ii] );
            double Y = atof( argv[++ii] );
            td[tdi].m_target_pts.push_back( Point(X,Y) );
        }
        else if (do_iterate) { // We interpret some arguments differently depending on whether the -iterate argument has been specified (must be earlier)
            if (aii >= sizeof(arg_it)/sizeof(arg_it[0])) fprintf(stderr, "Error: too many iterate arguments (%d >= %d)\n", aii, sizeof(arg_it)/sizeof(arg_it[0]));
                 if (strcmp(argv[ii], "-r"   ) == 0) { GrabIteratorArgs( ii, argc, argv, "radius",       arg_it[aii] ); aii++; }
            else if (strcmp(argv[ii], "-d"   ) == 0) { GrabIteratorArgs( ii, argc, argv, "distance",     arg_it[aii] ); aii++; }
            else if (strcmp(argv[ii], "-sa"  ) == 0) { GrabIteratorArgs( ii, argc, argv, "sunangle",     arg_it[aii] ); aii++; }
            else if (strcmp(argv[ii], "-sA"  ) == 0) { GrabIteratorArgs( ii, argc, argv, "sunaltitude",  arg_it[aii] ); aii++; }
            else if (strcmp(argv[ii], "-mna" ) == 0) { GrabIteratorArgs( ii, argc, argv, "minnormal",    arg_it[aii] ); aii++; }
            else if (strcmp(argv[ii], "-mxa" ) == 0) { GrabIteratorArgs( ii, argc, argv, "mixnormal",    arg_it[aii] ); aii++; }
            else if (strcmp(argv[ii], "-mw"  ) == 0) { GrabIteratorArgs( ii, argc, argv, "mirror_width", arg_it[aii] ); aii++; }
            else    { fprintf(stderr, "ERROR - unrecognized command line argument (#%d): %s - with -iterate option.\n", ii, argv[ii] ); }
        } else { // not iterate
                 if (strcmp(argv[ii], "-r"   ) == 0) { ii++; default_radius           = td[tdi].m_radius           = atof(argv[ii]); }
            else if (strcmp(argv[ii], "-d"   ) == 0) { ii++; default_distance         = td[tdi].m_distance         = atof(argv[ii]); }
            else if (strcmp(argv[ii], "-sa"  ) == 0) { ii++; default_sun_altitude_dir = td[tdi].m_sun_dir = atof(argv[ii]); }
            else if (strcmp(argv[ii], "-sA"  ) == 0) { ii++; default_sun_altitude_dir = td[tdi].m_sun_dir = atof(argv[ii]) + 180; }
            else if (strcmp(argv[ii], "-mna" ) == 0) { ii++; default_min_normal_dir   = td[tdi].m_min_normal_dir   = atof(argv[ii]); }
            else if (strcmp(argv[ii], "-mxa" ) == 0) { ii++; default_max_normal_dir   = td[tdi].m_max_normal_dir   = atof(argv[ii]); }
            else if (strcmp(argv[ii], "-mw"  ) == 0) { ii++; default_mirror_width=atof(argv[ii]); td[tdi].m_min_normal_dir=270-default_mirror_width; td[tdi].m_max_normal_dir=270+default_mirror_width; }
            else if (strcmp(argv[ii], "-next") == 0) { tdi++; td.resize(tdi+1); td[tdi].DuplicateSettings( td[tdi-1] ); }
            else    { fprintf(stderr, "ERROR - unrecognized command line argument (#%d): %s\n", ii, argv[ii] ); }
        }
    } // for ii<argc

    if (aii && dvo_debug)
        for (int ii=0; ii<aii; ii++) {
            printf("Iterator #%d: %-10s from=%g, to=%g, increment=%g\n",
                ii, arg_it[ii].parameter_name.c_str(), arg_it[ii].from, arg_it[ii].to, arg_it[ii].increment);
            printf("\tvalue_list.size()=%d\n", arg_it[ii].value_list.size() );
            printf("\t");
            for (int jj=0; jj<arg_it[ii].value_list.size(); jj++)
                printf("%g ", arg_it[ii].value_list[jj]);
            printf("\n");
        }

    if (aii) {
        int it_indices[ sizeof(arg_it)/sizeof(arg_it[0]) ];
        double it_values[ sizeof(arg_it)/sizeof(arg_it[0]) ];
        int bad_index[ sizeof(arg_it)/sizeof(arg_it[0]) ];
        for (int ii=0; ii<sizeof(arg_it)/sizeof(arg_it[0]); ii++) {
            it_indices[ii] = 0;
            it_values[ii] = 0;
            bad_index[ii] = 0;
        }
        // Here's the approach - the first iterator in arg_it spins the fastest. Iterate through it until reaches its
        // end, then reset it to the beginning. Then increment the next iterator (if possible) and restart on the first iterator.
        int done = 0;
        while (! done) {
            td[tdi].m_radius = default_radius;
			td[tdi].m_distance = default_distance;
            td[tdi].m_sun_dir = default_sun_altitude_dir;
            if (default_min_normal_dir != BadValue) td[tdi].m_min_normal_dir = default_min_normal_dir;
            if (default_max_normal_dir != BadValue) td[tdi].m_max_normal_dir = default_max_normal_dir;
            if (default_mirror_width   != BadValue) { td[tdi].m_min_normal_dir=270-default_mirror_width; td[tdi].m_max_normal_dir=270+default_mirror_width; }

            // Get cache the values
            for (int ii=0; ii<aii; ii++) {
                it_values[ii] = arg_it[ii].GetValue( it_indices[ii], bad_index[ii] );
                if (bad_index[ii]) {
                    it_indices[ii] = 0;
                    it_values[ii] = arg_it[ii].GetValue( it_indices[ii], bad_index[ii] );
                    if (ii<(aii-1)) it_indices[ii+1]++;
                    else done = true;
                }
                if (ii == 0) it_indices[ii]++; // increment for next loop
            }

            // Apply the values
            for (int ii=0; ii<aii; ii++) {
                     if (arg_it[ii].parameter_name == "radius"      ) { td[tdi].m_radius           = it_values[ii]; }
				else if (arg_it[ii].parameter_name == "distance"    ) { td[tdi].m_distance         = it_values[ii]; }
                else if (arg_it[ii].parameter_name == "sunangle"    ) { td[tdi].m_sun_dir = it_values[ii]; }
                else if (arg_it[ii].parameter_name == "sunaltitude" ) { td[tdi].m_sun_dir = it_values[ii] + 180; }
                else if (arg_it[ii].parameter_name == "minnormal"   ) { td[tdi].m_min_normal_dir   = it_values[ii]; }
                else if (arg_it[ii].parameter_name == "minnormal"   ) { td[tdi].m_max_normal_dir   = it_values[ii]; }
                else if (arg_it[ii].parameter_name == "mirror_width") {
                           td[tdi].m_max_normal_dir = 270 + it_values[ii]/2;
                           td[tdi].m_min_normal_dir = 270 - it_values[ii]/2;
                       } else {
                    fprintf(stderr,"Unrecognized iterator parameter (%s) for index=%d at %d of %s\n",
                                   arg_it[ii].parameter_name.c_str(), ii, __LINE__, __FILE__ );
                }
            }
            if (! done) {
                tdi++; td.resize(tdi+1);
                td[tdi] = td[tdi-1];
            }

        } // while ! done
    } // if aii


    FILE *fout = NULL;
    if (do_svg) {
        fout = fopen(svg_filename.c_str(), "w");
        if (fout == NULL) {
            fprintf(stderr, "Error: Can't open %s for writing.\n", svg_filename.c_str() );
            return false;
        }
    }


    for (int ii=0; ii<=tdi; ii++) {
        if (dvo_debug>1) {
            printf("Iteration loop %d of %d\n", ii, tdi);
            td[ii].InputDump(stdout);
        }

        td[ii].Calculate(do_reverse_trace ? 0 : num_rays, calc_pupil);

        if (dvo_debug) {
            printf("Calculated Data in Iteration loop %d of %d:\n", ii, tdi);
            td[ii].Dump(stdout);
        }


        if (ray_report) {
            td[ii].RayReport(stdout,1);
        }
    }

    for (int ii=0; ii<=tdi; ii++) {
        if (do_svg) {
#if 0
            if (td[ii].m_distance != BadValue)
    			td[ii].GenSVG_Convex(fout, offset_X, offset_Y, ii==0, ii==tdi, animate);
            else
                td[ii].GenSVG_Concave(fout, offset_X, offset_Y, title, ii==0, ii==tdi, animate, animate_interval_ms);
#else
            if (td[ii].m_IsConvex) td[ii].GenSVG_Convex(fout, offset_X, offset_Y, ii==0, ii==tdi, animate);
            else                   td[ii].GenSVG_Concave(fout, offset_X, offset_Y, title, ii==0, ii==tdi, animate, animate_interval_ms, do_boxes, focal_pts);
#endif
        }
    }

    if (do_csv2) GenerateReport(td, csv2_row, csv2_col, csv2_val );

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Start of the original Convex code


/* A 3 component optic system - a sun (at infinite distance and a finite
 * angular width), a Convex mirror, and an observer. The observer is at the
 * same Y elevation as the center of curvature of the mirror.
 *
 * My conventions...
 * Center of the curvature of the Mirror is at (0,0)
 * Observer is to the left at (-distance,0)
 * Regarding angles (3 sets: from Mirror's center, from observer, from Sun). All 3 Angles
 * 	are 0 when the sun is directly behind the observer (on horizon).
 * From Center of curvature:
 * 	0 degrees is horizontally to the left.
 * 	90 degrees is vertical.
 * From Observer:
 * 	0 degree is horizontally to the right.
 * 	>0 (and <90) degrees is upwards to the right.
 * From Sun:
 *	Sun's 0 degree is a horizontal ray from the left.
 *	Sun's 0..90 degrees are rays from above left - 90degrees is vertical (down),
 *	and >90 degrees are from the above right.
 *	We use 3 points in the Sun - the bottom (at 6 o'clock) the center/middle and the top (12 o'clock)
 *
 *
 * Be aware: Standard Trig Quadrants...
 *          |
 *        2 | 1
 *      ----+----
 *        3 | 4
 *          |
 * But this is NOT my convention here.
 *
 *
 * Inputs (all angles in degrees):
 * r=radius of convex mirror whose Center of curvature is at (0,0)
 * d=distance of observer from the Center of curvature. Precondition: d>r
 * 	r and d have the same units (of your choice).
 * sa=altitude of the center-point of the sun. (0=horizontal and
 * 	behind the observer)
 * sw=angular width of the sun. Default=0.5
 *
 * Calculates:
 * Observer Tangent Angle - the angle at which the observer sees the upper-most portion
 * 	of the mirror (i.e. a ray from the observer that grazes along a tangent
 * 	of the mirror). This is determined exactly by 'r' and 'd'.
 * Normal Tange Angle - for the above tangent - the corresponding normal angle (from C to mirror)
 * Observed angle of the reflected sun off the mirror. There are 3 variations
 * 	of this - for the top, middle and bottom points of the sun.
 * 	Note that this each constrained between 0 and the 'Tangent Angle'.
 * Observed Angular Size of the reflected sun off the mirror. Simply
 * 	the difference between 'top' and 'bottom' observed angles of the
 * 	reflected sun off the mirror.
 *
 */






static Point my_local_normal_point; // junk storage usable for an optional default argument.

bool CalcFromNormal_Convex(const TheData&td,
			double normal_ang,
		       	double& ang_from_observer,
		       	double& ang_from_sky,
		       	Point & normalPt=my_local_normal_point)
// Convex Mirror
	// Take the normal_angle (in degrees from Mirror's center-of-curvature) - 
	// Step 1: calculate the (X,Y) coordinates on the mirror,
	// Step 2: calculate angle from observer to the (X,Y) point,
	// Step 3: calculate the corresponding angle from the sky that would get
	// 	reflected at point (X,Y) to the observer.
	// return success.
{
	// Step 0 - error check
	if (normal_ang > td.m_NormalTangentAng) {
		ang_from_observer = ang_from_sky = 0;
		normalPt.x(0);
		normalPt.y(0);
		return false;
	}

	// Step 1
	double normal_ang_radians = to_radians(normal_ang);
	normalPt.x( td.m_MirrorCOCPt.x() + td.m_radius * cos( normal_ang_radians ) );
	normalPt.y( td.m_MirrorCOCPt.y() + td.m_radius * sin( normal_ang_radians ) );

	// Step 2
double observer_ang_radians = atan2( normalPt.y() - td.m_ObserverPt.y(), normalPt.x() - td.m_ObserverPt.x() ); 
double ang_from_observer_old = to_degrees( observer_ang_radians );
	ang_from_observer = Direction( td.m_ObserverPt, normalPt );

	// Step 3
double ang_from_sky_old = 2* normal_ang + ang_from_observer;
	ang_from_sky = normal_ang + normal_ang + (180-ang_from_observer) + 180;

//printf("angle-from_observer(old)=%g, new=%g, angle-from-sky (old)=%g, new=%g\n", ang_from_observer_old, ang_from_observer, ang_from_sky_old, ang_from_sky );
	return true;
}



bool SearchForSkyAng_Convex(const TheData& td,
			double target_sky_ang,
			double &found_normal_ang,
			double &found_sky_ang,
			double &found_observer_ang,
			Point  &found_MirrorPoint,
			double acceptable_difference = 0.001)
{
	/* Successive approximation.
	 */
	double prev_ang_from_sky = 0; // to determine when we are close enough

	double max_normal =  td.m_NormalTangentAng; // The search will close the window between max_normal and min_normal
	double min_normal = -td.m_NormalTangentAng;

	for (int iterate_counter = 0; iterate_counter < 100; iterate_counter++) {
		double guess_normal = (max_normal + min_normal) / 2;

		double ang_from_observer, ang_from_sky;
		Point mirror_point;
		bool success = CalcFromNormal_Convex(td, guess_normal, ang_from_observer, ang_from_sky, mirror_point);

		if (success) {
			if (dvo_debug >= 4)
				printf("%3d: Target=%-7.4g Calc(guess_normal=%-7.4g, observer=%-7.4g, sky=%-7.4g)=%d MirrorPtr=(%-7.4g,%-7.4g), min,max=%-7.4g,%-7.4g\n",
					iterate_counter, target_sky_ang, guess_normal, ang_from_observer, ang_from_sky,
				       	success, mirror_point.x(), mirror_point.y(), min_normal, max_normal);

			// Conditions to stop looping...
			if ( NearlyEqual(ang_from_sky, target_sky_ang) || ((iterate_counter >= 1) && NearlyEqual(prev_ang_from_sky, ang_from_sky))) {
				bool passed = NearlyEqual(ang_from_sky,target_sky_ang); // Fails if didn't converge

				if (! passed ) { // Didn't converge
					if (dvo_debug >= 3)
						printf("End of search - didn't converge: target_sky_ang=%g, normal boundaries=%g,%g, ang_from_sky (previous)=%g (%g)\n",
							target_sky_ang, min_normal, max_normal, ang_from_sky, prev_ang_from_sky);
						return false;
				} // Else - passed
				found_normal_ang = guess_normal;
				found_sky_ang = ang_from_sky;
				found_observer_ang = ang_from_observer;
				found_MirrorPoint = mirror_point;
				if (dvo_debug >= 3)
					printf("End of search: FoundNormalAng=%g, FoundSkyAng=%g, FoundObserverAng=%g, FoundMirrorPt=(%g,%g)\n",
						      found_normal_ang, found_sky_ang, found_observer_ang,  found_MirrorPoint.x(), found_MirrorPoint.y() );
				return true; // Normal exit point in a successful search
			}
		}

		// Refine the window for the next iteration
		if (ang_from_sky > target_sky_ang) { max_normal = guess_normal;  }
		if (ang_from_sky < target_sky_ang) { min_normal = guess_normal; }

		prev_ang_from_sky = ang_from_sky;
	}
	return false;
}


void TheData::Calculate_Convex(int num_rays, int do_pupil)
	/* The object a few 'input' parameters, and numerous 'derived' values - that are determined from the
	 * 'input' parameters. This routine determines those derived values.
	 */
{
	if (CheckInputs()) {
		Set(m_MirrorCOCPt, 0, 0 );
		// The user, via command-line arguments, can have the ObserverPoint set - or the distance - but not both.
		if (m_distance != BadValue) {
			Set(m_ObserverPt, m_distance, 0 );
		} else if (m_ObserverPt.x() || m_ObserverPt.y()) {
			m_distance = Distance(m_ObserverPt, m_MirrorCOCPt );
		} else { // neither was set, so pick a default.
			m_distance = 2.0;
		}

		// Calculate the tangent - imagine a right-triangle - mirror-tangent is 90 degrees, other 2 vertices are at observer and Mirror COC
		double angle_tangentPt_obs_mirrorCoc = to_degrees( asin( m_radius / Distance(m_MirrorCOCPt, m_ObserverPt) ) ); // angle at observer
		double third_angle = 180.0 -90.0 - angle_tangentPt_obs_mirrorCoc; // angle at Mirror's COC
		double third_angle_to_X_axis = 180.0 - third_angle; // assumes observer and Mirror's COC are both on X axis (Y=0)
		m_NormalTangentAng = 90.0 + (90.0 - third_angle_to_X_axis); // From Mirror's COC
		if (1) { // Finds the tangent point
			double junk1, junk2;
			CalcFromNormal_Convex(*this, m_NormalTangentAng, junk1, junk2, m_TangentPt);
		}
		m_ObserverTangentAng = Direction( m_ObserverPt, m_TangentPt );


		// Calculate (searches) for the 3 rays from observer to mirror to sun (reverse ray-tracing)
		double normal_mid, normal_bot, normal_top;
		bool success1 = SearchForSkyAng_Convex(*this, m_sun_dir, normal_mid, m_SunMidAng, m_ObserverReflectedSunMid, m_SunMidMirrorPt);
		double target_sun_bot_ang = m_SunMidAng - m_sun_width_ang/2;
		double target_sun_top_ang = m_SunMidAng + m_sun_width_ang/2;
		bool success2 = SearchForSkyAng_Convex(*this, target_sun_bot_ang, normal_bot, m_SunBotAng, m_ObserverReflectedSunBot, m_SunBotMirrorPt);
		bool success3 = SearchForSkyAng_Convex(*this, target_sun_top_ang, normal_top, m_SunTopAng, m_ObserverReflectedSunTop, m_SunTopMirrorPt);

#if 0 // try1
		if (success1 && success2 && success3 && do_pupil) { // experimental
			// Here's my approach - imagine a line that passes through the m_SunMidMirrorPt - perpendicular
			// to the ray from the sun. Determine where that line crosses the rays (from the sun) for the
			// Top and Bot rays. In many cases, this line crosses into the mirror, but we ignore that that one
			// of the rays might not actually reach this line.
			//
			// My first attempt at dot-products didn't work. So now trying a different approach that is based on having
			// two points on each of two lines (http://www.ambrsoft.com/MathCalc/Line/TwoLinesIntersection/TwoLinesIntersection.htm)
			// What I have is the m_SunXxxMirrorPt and the m_SunXxxAng for each of the 3 rays - so a point and and angle (I should
			// be able to treat each as a vector - and use the dot-products (etc.) to determine the intersection points). So,
			// I'm now arbitrarily locating a 2nd point (a distance of +1 from the first) on each ray and using that in the calculations.
			Point BotRay_2nd_pt( m_SunBotMirrorPt.x() + 1*cos(to_radians(m_SunBotAng)), m_SunBotMirrorPt.y() - 1*sin(to_radians(m_SunBotAng)) ); 
			Point TopRay_2nd_pt( m_SunTopMirrorPt.x() + 1*cos(to_radians(m_SunTopAng)), m_SunTopMirrorPt.y() - 1*sin(to_radians(m_SunTopAng)) ); 

			double pupil_line_ang = m_SunMidAng + 90.0;
			Point Pupil_2nd_pt ( m_SunMidMirrorPt.x() + 1*cos(to_radians(pupil_line_ang)), m_SunMidMirrorPt.y() - 1*sin(to_radians(pupil_line_ang)) ); 

			Intersection_2Segments( Segment(m_SunMidMirrorPt, Pupil_2nd_pt), Segment(m_SunTopMirrorPt, TopRay_2nd_pt), m_PupilTopPt );
			Intersection_2Segments( Segment(m_SunMidMirrorPt, Pupil_2nd_pt), Segment(m_SunBotMirrorPt, BotRay_2nd_pt), m_PupilBotPt );

            if (Defined(m_PupilBotPt) && Defined(m_PupilTopPt) ) {
    			m_Pupil = Distance( m_PupilBotPt, m_PupilTopPt ); 

    			m_Brightness = to_degrees( asin( m_Pupil / Distance( m_ObserverPt, m_SunMidMirrorPt )) ) / 0.5;
            }
		}
#endif

#if 1 // try2
		if (success1 && success2 && success3 && do_pupil) { // experimental
			// Here's my approach - Consider a line-segement between the m_PupilTopPt and m_PupilBotPt - what
			// is the length of the line as 'seen' from the Sun? (for Entrance pupil) or From the Observer (for
			// the Exit pupil).

            m_Pupil_Entrance = ApparentWidth( m_SunTopMirrorPt, m_SunBotMirrorPt, m_sun_dir );
            m_Pupil_Exit     = ApparentWidth( m_SunTopMirrorPt, m_SunBotMirrorPt, m_ObserverReflectedSunMid );
// printf("Mirror Points=(%g,%g),  (%g,%g), angles=%g, %g\n", m_SunTopMirrorPt.x(), m_SunTopMirrorPt.y(), m_SunBotMirrorPt.x(), m_SunBotMirrorPt.y(), m_sun_dir, m_ObserverReflectedSunMid );

            m_Brightness = m_Pupil_Entrance / m_Pupil_Exit;

            // Another brightness approach - compare the apparent angular width to 0.5 degrees
            m_Brightness2 = ApparentWidth_ang( m_SunTopMirrorPt, m_SunBotMirrorPt, m_ObserverPt ) / m_sun_width_ang;

//printf("Pupils: %g/%g = %g, Brightness2=%g\n", m_Pupil_Entrance, m_Pupil_Exit, m_Brightness, m_Brightness2 );
		}
#endif


		if (dvo_debug>=2)
			printf("Results: %d%d%d: observer_angs: %g, %g, %g (diff=%g) sun_angs: (tar=%g) %g, %g, %g (diff=%g) (Normals=%g,%g,%g), Pupil=%g/%g, Bright=%g,%g\n",
				success1, success2, success3,
				m_ObserverReflectedSunBot, m_ObserverReflectedSunMid, m_ObserverReflectedSunTop,
			       	m_ObserverReflectedSunTop-m_ObserverReflectedSunBot,
				m_sun_dir, m_SunBotAng, m_SunMidAng, m_SunTopAng, m_SunTopAng-m_SunBotAng,
				normal_bot, normal_mid, normal_top,
				m_Pupil_Entrance/m_Pupil_Exit, m_Brightness, m_Brightness2);
	}
}


static void OneRayFromSunToObserver_CalcLines(bool is_bad, double sun_ang,
	       Point& found_sun_pt, Point& sun_pt2 /* start with mirror-point */,
	       const Point& observer_pt,
	       double border_left, double border_top, double border_right, double border_bottom)
{
	const Point mirror_pt = sun_pt2;
	if ( is_bad ) { // Assuming this means the sun is past the tangent (behind the mirror)
		// Sun is obscurred - we'll draw a partial ray of a different color - this happens only if sun_ang > 90
		// We'll create the rays as if they would reach the observer (if not for the mirror)
		found_sun_pt.x( border_left );
		found_sun_pt.y( ( observer_pt.x() - found_sun_pt.x() ) * -1 * tan( to_radians(sun_ang) ) );
		// How to determine a reasonable length for this ray?
		sun_pt2.x( border_left/2 ); // Not actually on the mirror - just the other end of a shortened sun-ray
		sun_pt2.y( (observer_pt.x() - sun_pt2.x()) * -1 * tan( to_radians(sun_ang) ) );
	} else {
		// Incident ray from the sun
		// Where should the beginning of the incident ray line start? - at one of the border.
		double sun_ang_normalized = NormalizeAngle( sun_ang );
		if (sun_ang_normalized > 270) { // Check for intersection with right border
			found_sun_pt.x( border_left );
			found_sun_pt.y( mirror_pt.y() - tan( to_radians( sun_ang_normalized ) ) * (mirror_pt.x() - border_left) );
			if (found_sun_pt.y() > border_top) {
				found_sun_pt.y( border_top );
				found_sun_pt.x( mirror_pt.x() + (found_sun_pt.y() - mirror_pt.y())/tan( to_radians(sun_ang_normalized) ) );
			}
		} else if ((sun_ang_normalized < 270) && (sun_ang_normalized >= 90)) { // Check for intersection with right border - DVO HELP- can this be combined with above?
			found_sun_pt.y( border_top );
			found_sun_pt.x( mirror_pt.x() + (border_top-mirror_pt.y())/tan( to_radians(sun_ang_normalized) ) );
			if (found_sun_pt.x() > border_right) {
				found_sun_pt.x( border_right );
				found_sun_pt.y( mirror_pt.y() + tan( to_radians( sun_ang_normalized ) ) * (border_right - mirror_pt.x()) );
			}
		} else if (sun_ang_normalized < 90) { // Check for intersection with bottom border - DVO HELP- can this be combined with above?
			found_sun_pt.y( border_bottom );
			found_sun_pt.x( mirror_pt.x() - (mirror_pt.y()-border_bottom)/tan( to_radians(sun_ang_normalized) ) );
			if (found_sun_pt.x() > border_right) {
				found_sun_pt.x( border_right );
				found_sun_pt.y( mirror_pt.y() + tan( to_radians( sun_ang_normalized ) ) * (border_right - mirror_pt.x()) );
			}
		} else { // else must be exactly 270 degrees
			found_sun_pt.x( mirror_pt.x() );
			found_sun_pt.y( border_top );
		}
	}
}


static void SVG_OneRayFromSunToObserver_Convex(FILE *fout, const CoordConverter& cc,
		double sun_ang, const char* id_name_suffix,
		const Point& observer_pt, const Point& mirror_pt,
		double border_left, double border_top, double border_right, double border_bottom, double line_width)
{
	Point sunpt1, sunpt2(mirror_pt);

	bool obscured_sun = NearlyEqual(mirror_pt.x(),0) && NearlyEqual(mirror_pt.y(),0);

	OneRayFromSunToObserver_CalcLines(obscured_sun, sun_ang, sunpt1, sunpt2, observer_pt, border_left, border_top, border_right, border_bottom);

	// Incident ray from the sun
	fprintf(fout, "<line id=\"incident_ray_%s\" x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" style=\"stroke-width: %g; stroke: %s;\"/>\n",
			id_name_suffix, cc.X(sunpt1.x()), cc.Y(sunpt1.y()),  cc.X(sunpt2.x()), cc.Y(sunpt2.y()), line_width, obscured_sun ? "purple" : "red" );

	if ( ! obscured_sun ) {
		// Reflected ray - from mirror to observer
		fprintf(fout, "<line id=\"reflected_ray_%s\" x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" style=\"stroke-width: %g; stroke: orange;\"/>\n",
			id_name_suffix, cc.X(mirror_pt.x()), cc.Y(mirror_pt.y()),  cc.X(observer_pt.x()), cc.Y(observer_pt.y()), line_width);
	}
}

bool TheData::GenSVG_Convex(FILE *fout, double offset_X, double offset_Y, bool first_call, bool last_call, int animate) const
{
	// Intend for a 10% margin/borders.
	// As always with SVG, increasing X is to the right, and increase Y is DOWN the screen.
	// So we typically invert (*-1) the Y coordinates from our optics modelling to match the SVG model.
	// Relocate the origin to be the center of the mirror - and place this at the left/right center and on bottom border.
	// Put the observer at the left-bottom border corner.
	const double border_proportion = 0.10;
	const double canvas_size = 800;
	const double line_width = 1; // Thinest
	const double mirror_line_width = 0.5;

	const double border_size = 2.5*m_distance * border_proportion;

	double from_top_border    =  2   *m_distance;
	double from_bottom_border = -0.5 *m_distance;
	double from_left_border   = -1.25*m_distance;
	double from_right_border  =  1.25*m_distance;
	CoordConverter cc;
	cc.DefineFrom( offset_X+from_left_border  - border_size, offset_Y+from_bottom_border - border_size,
                   offset_X+from_right_border + border_size, offset_Y+from_top_border    + border_size);
	cc.DefineTo  (0, 800, 800, 0);

	if (first_call) {
		fprintf(fout, "<?xml version=\"1.0\" standalone=\"yes\"?>\n");
		fprintf(fout, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
		fprintf(fout, "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" ");
		fprintf(fout, "width=\"%g\" height=\"%g\" viewBox=\"%g %g %g %g\">\n",
			canvas_size, canvas_size,
			0.0,0.0, canvas_size, canvas_size
			);

		if (1) { // Use inline CSS?
			fprintf(fout, "<defs>\n");
			fprintf(fout, "<style type=\"text/css\"><![CDATA[\n");

			fprintf(fout, "path{ shape-rendering : crispEdges; }\n");

			fprintf(fout, "]]></style>\n");

			fprintf(fout, "</defs>\n");

			if (animate == 1) {
				fprintf(fout, "<style>\n");
				fprintf(fout, "@keyframes try1 {\n");
				fprintf(fout, "\t0%%   {  --dvo: yellow;}\n");
		fprintf(fout, "<animate xlink:href=\"#incident_ray\"  attributeName=\"x1\" from=\"0\" to=\"800\" begin=\"0s\" dur=\"10s\" fill=\"freeze\">\n");
				fprintf(fout, "\t10%%  {  --dvo: blue;}\n");
				fprintf(fout, "\t20%%  {  --dvo: green;}\n");
				fprintf(fout, "\t40%%  {  --dvo: red;}\n");
				fprintf(fout, "\t100%% {  --dvo: orange;}\n");
				fprintf(fout, "}\n");
				fprintf(fout, "</animate>\n");

				fprintf(fout, "</style>\n");
			}
		}
	}
	if(!animate || first_call) {

		// Mirror
		fprintf(fout, "<circle cx=\"%g\" cy=\"%g\" r=\"%g\" style=\"stroke-width: %g; stroke: black; fill: none;\"/>\n",
			cc.X(0), cc.Y(0), m_radius*cc.Scale(), mirror_line_width);
		// Center cross-marks
		fprintf(fout, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" style=\"stroke-width: 0.5; stroke: #888888;\"/>\n",
			cc.X(0-m_radius/10), cc.Y(0), cc.X(0+m_radius/10), cc.Y(0) );
		fprintf(fout, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" style=\"stroke-width: 0.5; stroke: #888888;\"/>\n",
			cc.X(0), cc.Y(0-m_radius/10), cc.X(0), cc.Y(0+m_radius/10) );

		// Observer
		fprintf(fout, "<circle cx=\"%g\" cy=\"%g\" r=\"%g\" style=\"stroke-width: %g; stroke: black; fill: blue;\"/>\n",
			cc.X(m_ObserverPt.x()), cc.Y(m_ObserverPt.y()), 2.0, line_width);
		// Center line from observer to Mirror's center's cross-marks
		fprintf(fout, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" stroke-dasharray=\"15, 10, 5, 10\" style=\"stroke-width: 0.5; stroke: #888888;\"/>\n",
			cc.X(m_ObserverPt.x()), cc.Y(m_ObserverPt.y()), cc.X(m_MirrorCOCPt.x()), cc.Y(m_MirrorCOCPt.y()) );

		// Tangent Line - from observer, tangent to the mirror and beyond
		double tangent_line_end_X =  m_ObserverPt.x() + cos(to_radians(m_ObserverTangentAng)) * fabs(m_distance * 1.5);
		double tangent_line_end_Y =  m_ObserverPt.y() + sin(to_radians(m_ObserverTangentAng)) * fabs(m_distance * 1.5);
		fprintf(fout, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" stroke-dasharray=\"15, 10, 5, 10\" style=\"stroke-width: 0.5; stroke: #888888;\"/>\n",
			cc.X(m_ObserverPt.x()), cc.Y(m_ObserverPt.y()), cc.X(tangent_line_end_X), cc.Y(tangent_line_end_Y) );

		// Normal line
		if (0)
		fprintf(fout, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" stroke-dasharray=\"10, 3, 3, 3\" style=\"stroke-width: 0.5; stroke: #000088;\"/>\n",
			cc.X(0), cc.Y(0), cc.X(m_TangentPt.x() * 1.3), cc.Y(m_TangentPt.y() * 1.3) );

		SVG_OneRayFromSunToObserver_Convex(fout, cc, m_sun_dir-m_sun_width_ang/2, "bot", m_ObserverPt, m_SunBotMirrorPt, from_left_border, from_top_border, from_right_border, from_bottom_border, line_width/2);
		SVG_OneRayFromSunToObserver_Convex(fout, cc, m_sun_dir+m_sun_width_ang/2, "top", m_ObserverPt, m_SunTopMirrorPt, from_left_border, from_top_border, from_right_border, from_bottom_border, line_width/2);

#if 0
		if (m_Pupil_Entrance != 0) {
			fprintf(fout, "<circle cx=\"%g\" cy=\"%g\" r=\"%g\" style=\"stroke-width: 1; stroke: green; fill: green;\"/>\n",
				cc.X(m_PupilBotPt.x()), cc.Y(m_PupilBotPt.y()), 0.5 );
			fprintf(fout, "<circle cx=\"%g\" cy=\"%g\" r=\"%g\" style=\"stroke-width: 1; stroke: green; fill: green;\"/>\n",
				cc.X(m_PupilTopPt.x()), cc.Y(m_PupilTopPt.y()), 0.5 );
		}
#endif

	}

	if (0 && animate) {
		fprintf(fout, "<animate xlink:href=\"#incident_ray\"  attributeName=\"x1\" from=\"0\" to=\"800\" begin=\"0s\" dur=\"10s\" fill=\"freeze\">\n");
		fprintf(fout, "</animate>\n");
	}

	if (animate) {
		if (first_call) {
			fprintf(fout, "<text id=\"title_text\" x=\"%g\" y=\"%g\">Sun altitude=%-5.1g, Observer Angle=",
				       cc.X(m_MirrorCOCPt.x()), cc.Y(m_MirrorCOCPt.y())+30, m_sun_dir);
			if ((m_ObserverReflectedSunTop != BadValue) && (m_ObserverReflectedSunBot != BadValue))
				fprintf(fout, "%-5.1g", (m_ObserverReflectedSunTop+m_ObserverReflectedSunBot)/2 );
			else	fprintf(fout, "(obscured)");
			fprintf(fout, ", Width=");
			if ((m_ObserverReflectedSunTop != BadValue) && (m_ObserverReflectedSunBot != BadValue))
				fprintf(fout, "%5.3g", fabs(m_ObserverReflectedSunTop-m_ObserverReflectedSunBot));
			else	fprintf(fout, "(obscured)");
			fprintf(fout, "</text>\n");

			fprintf(fout, "<script type=\"text/ecmascript\"><![CDATA[\n");
			fprintf(fout, "var line_data = [\n");
			fprintf(fout, "//\tvisible, sunray start, sunray end, observer, sunangle, observer_angle\n");
		}


		Point sunpt1_bot, sunpt2_bot(m_SunBotMirrorPt);
		Point sunpt1_top, sunpt2_top(m_SunTopMirrorPt);
		bool obscured_sun_bot = NearlyEqual(m_SunBotMirrorPt.x(),0) && NearlyEqual(m_SunBotMirrorPt.y(),0);
		bool obscured_sun_top = NearlyEqual(m_SunTopMirrorPt.x(),0) && NearlyEqual(m_SunTopMirrorPt.y(),0);
		OneRayFromSunToObserver_CalcLines(obscured_sun_bot, m_sun_dir-m_sun_width_ang/2, sunpt1_bot, sunpt2_bot, m_ObserverPt, from_left_border, from_top_border, from_right_border, from_bottom_border);
		OneRayFromSunToObserver_CalcLines(obscured_sun_top, m_sun_dir+m_sun_width_ang/2, sunpt1_top, sunpt2_top, m_ObserverPt, from_left_border, from_top_border, from_right_border, from_bottom_border);

		fprintf(fout, "\t[ %d, %g, %g,%g, %g,%g, %g,%g, %g, %g ],\n",
				obscured_sun_bot?0:1,
				m_sun_dir,
			       	cc.X(sunpt1_bot.x()), cc.Y(sunpt1_bot.y()),
			       	cc.X(sunpt2_bot.x()), cc.Y(sunpt2_bot.y()),
			       	cc.X(m_ObserverPt.x()), cc.Y(m_ObserverPt.y()),
		      		m_SunBotAng==BadValue?0:m_SunBotAng,
			       	m_ObserverReflectedSunBot==BadValue?0:m_ObserverReflectedSunBot );
		fprintf(fout, "\t[ %d, %g, %g,%g, %g,%g, %g,%g, %g, %g ],\n",
				obscured_sun_top?0:1,
				m_sun_dir,
			       	cc.X(sunpt1_top.x()), cc.Y(sunpt1_top.y()),
			       	cc.X(sunpt2_top.x()), cc.Y(sunpt2_top.y()),
			       	cc.X(m_ObserverPt.x()), cc.Y(m_ObserverPt.y()),
		      		m_SunTopAng==BadValue?0:m_SunTopAng,
			       	m_ObserverReflectedSunTop==BadValue?0:m_ObserverReflectedSunTop );


		if (last_call) {
			fprintf(fout, "];\n");
			fprintf(fout, "var ref_ray_top = document.getElementById(\"reflected_ray_top\");\n");
			fprintf(fout, "var ref_ray_bot = document.getElementById(\"reflected_ray_bot\");\n");
			fprintf(fout, "var inc_ray_top = document.getElementById(\"incident_ray_top\");\n");
			fprintf(fout, "var inc_ray_bot = document.getElementById(\"incident_ray_bot\");\n");
			fprintf(fout, "var title_text  = document.getElementById(\"title_text\");\n");

			fprintf(fout, "var id = setInterval(intervalCallback, 250);\n");
			fprintf(fout, "var row_index = 0;\n");
			fprintf(fout, "function intervalCallback() {\n");
			fprintf(fout, "\tif(row_index >= line_data.length) row_index = 0;\n");
			fprintf(fout, "\tvar bot_i = row_index;\n");
			fprintf(fout, "\tvar top_i = row_index+1;\n");
			fprintf(fout, "\trow_index += 2;\n");
			fprintf(fout, "\tinc_ray_bot.setAttribute('x1',line_data[bot_i][2]);\n");
			fprintf(fout, "\tinc_ray_bot.setAttribute('y1',line_data[bot_i][3]);\n");
			fprintf(fout, "\tinc_ray_bot.setAttribute('x2',line_data[bot_i][4]);\n");
			fprintf(fout, "\tinc_ray_bot.setAttribute('y2',line_data[bot_i][5]);\n");
			fprintf(fout, "\tref_ray_bot.setAttribute('visibility',line_data[bot_i][0] ? 'visible' : 'hidden' );\n");
			fprintf(fout, "\tif (line_data[row_index][0]) {\n");
			fprintf(fout, "\t\tref_ray_bot.setAttribute('x1',line_data[bot_i][4]);\n");
			fprintf(fout, "\t\tref_ray_bot.setAttribute('y1',line_data[bot_i][5]);\n");
			fprintf(fout, "\t\tref_ray_bot.setAttribute('x2',line_data[bot_i][6]);\n");
			fprintf(fout, "\t\tref_ray_bot.setAttribute('y2',line_data[bot_i][7]);\n");
			fprintf(fout, "\t}\n");
			fprintf(fout, "\trow_index++;\n");
			fprintf(fout, "\tinc_ray_top.setAttribute('x1',line_data[top_i][2]);\n");
			fprintf(fout, "\tinc_ray_top.setAttribute('y1',line_data[top_i][3]);\n");
			fprintf(fout, "\tinc_ray_top.setAttribute('x2',line_data[top_i][4]);\n");
			fprintf(fout, "\tinc_ray_top.setAttribute('y2',line_data[top_i][5]);\n");
			fprintf(fout, "\tref_ray_top.setAttribute('visibility',line_data[top_i][0] ? 'visible' : 'hidden' );\n");
			fprintf(fout, "\tif (line_data[row_index][0]) {\n");
			fprintf(fout, "\t\tref_ray_top.setAttribute('x1',line_data[top_i][4]);\n");
			fprintf(fout, "\t\tref_ray_top.setAttribute('y1',line_data[top_i][5]);\n");
			fprintf(fout, "\t\tref_ray_top.setAttribute('x2',line_data[top_i][6]);\n");
			fprintf(fout, "\t\tref_ray_top.setAttribute('y2',line_data[top_i][7]);\n");
			fprintf(fout, "\t}\n");
			fprintf(fout, "\trow_index++;\n");
			fprintf(fout, "\tvar build_str='Radius='.concat((%g), ', Distance=', (%g), ' ');\n", m_radius, m_distance);
			fprintf(fout, "\tvar sun_angle=(line_data[top_i][1]+line_data[bot_i][1])/2;\n");
			fprintf(fout, "\tvar obs_angle=(line_data[top_i][8]+line_data[bot_i][9])/2;\n");
			fprintf(fout, "\tvar obs_width=Math.abs(line_data[top_i][9]-line_data[bot_i][9])/2;\n");
			fprintf(fout, "\ttitle_text.textContent=build_str.concat('Sun altitude=',(sun_angle).toFixed(2), ', Observer Angle=', line_data[top_i][0] ? (obs_angle).toFixed(2) : 'Obscured', ', Width=', (obs_width).toFixed(2));\n");
			fprintf(fout, "}\n");

			fprintf(fout, "// ]]>\n</script>\n");
		}
	}

	if(last_call) fprintf(fout, "</svg>\n");
}




int brighttable()
{
    TheData td1;
    
    td1.m_radius = 100;
    td1.m_IsConvex = true;
    td1.m_ObserverPt.y(0);

    static const double distances[] = { 0.1,0.2,0.5,1,2,5,10,20,50,100,200,500,1000,2000,5000,10000,20000,50000,100000,200000,500000,1000000,2000000,5000000};
    const int num_distances = sizeof(distances)/sizeof(distances[0]);
    static const int sun_angle_table[] = { 0, 1, 2, 5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180 };
    const int num_sun_angles = sizeof(sun_angle_table)/sizeof(sun_angle_table[0]);

    double results[num_distances][num_sun_angles]; // larger than needed (hopefully)
    for (int ii=0; ii<num_distances; ii++) for (int jj=0; jj<num_sun_angles; jj++) results[ii][jj] = BadValue;

    for (int distance_index=0; distance_index < num_distances; distance_index++) {
        double distance = distances[distance_index];
        td1.m_ObserverPt.x(100 + distance);

        for (int sun_angle_index=0; sun_angle_index < num_sun_angles; sun_angle_index++) {
            double sun_angle = sun_angle_table[sun_angle_index];
            td1.m_sun_dir = sun_angle;

            double normal_mid, normal_bot, normal_top;
            bool success1 = SearchForSkyAng_Convex(td1, td1.m_sun_dir, normal_mid, td1.m_SunMidAng, td1.m_ObserverReflectedSunMid, td1.m_SunMidMirrorPt);
            double target_sun_bot_ang = td1.m_SunMidAng - td1.m_sun_width_ang/2;
            double target_sun_top_ang = td1.m_SunMidAng + td1.m_sun_width_ang/2;
            bool success2 = SearchForSkyAng_Convex(td1, target_sun_bot_ang, normal_bot, td1.m_SunBotAng, td1.m_ObserverReflectedSunBot, td1.m_SunBotMirrorPt);
            bool success3 = SearchForSkyAng_Convex(td1, target_sun_top_ang, normal_top, td1.m_SunTopAng, td1.m_ObserverReflectedSunTop, td1.m_SunTopMirrorPt);

            td1.m_Brightness2 = ApparentWidth_ang( td1.m_SunTopMirrorPt, td1.m_SunBotMirrorPt, td1.m_ObserverPt ) / td1.m_sun_width_ang;

            if (td1.m_Brightness2 != BadValue) {
                results[distance_index][sun_angle_index] = td1.m_Brightness2;
            }
        }
    }


    printf("Relative intensity (in ppm)\n");
    printf("%9s", " ");
    for (int jj=0; jj<num_sun_angles; jj++) printf("%6d ", sun_angle_table[jj]);
    printf("\n");

    for (int ii=0; ii<num_distances; ii++) {
        if (distances[ii] < 1) printf("%7.1f: ", distances[ii]);
        else                   printf("%7.0f: ", distances[ii]);
        for (int jj=0; jj<num_sun_angles; jj++) {
            if (results[ii][jj] == BadValue) printf("%6s ", " - ");
            else {
                char buffer[20];
                sprintf(buffer,"%6d",  int(results[ii][jj] * 1e6) );
                printf("%6s ", buffer);
            }
        }
        printf("\n");
    }

    return 0;
}
